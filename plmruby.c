#include <postgres.h>
#include <executor/spi.h>
#include <commands/trigger.h>
#include <catalog/pg_proc.h>
#include <catalog/pg_type.h>
#include <access/xact.h>

#include <mruby.h>
#include <mruby/proc.h>

#include "plmruby.h"
#include "plmruby_proc.h"
#include "plmruby_util.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(plmruby_call_handler);
PG_FUNCTION_INFO_V1(plmruby_inline_handler);

void _PG_init(void);

static void
plmruby_xact_cb(XactEvent event, void *arg)
{
	cleanup_plmruby_exec_env();
}

static Datum
		call_trigger(PG_FUNCTION_ARGS, plmruby_exec_env *xenv);

static Datum
		call_set_returning_function(PG_FUNCTION_ARGS, plmruby_exec_env *xenv,
									int nargs, plmruby_type argtypes[], plmruby_type *rettype);

static Datum
		call_function(PG_FUNCTION_ARGS, plmruby_exec_env *xenv,
					  int nargs, plmruby_type argtypes[], plmruby_type *rettype);

Datum
plmruby_call_handler(PG_FUNCTION_ARGS)
{
	Oid fn_oid = fcinfo->flinfo->fn_oid;
	bool is_trigger = CALLED_AS_TRIGGER(fcinfo);

	if (!fcinfo->flinfo->fn_extra)
	{
		plmruby_proc *proc = new_plmruby_proc(fn_oid, fcinfo, false, is_trigger);
		proc->xenv = create_plmruby_exec_env(proc->cache->proc_class);
		fcinfo->flinfo->fn_extra = proc;
	}

	plmruby_proc *proc = fcinfo->flinfo->fn_extra;
	plmruby_proc_cache *cache = proc->cache;

	if (is_trigger)
		return call_trigger(fcinfo, proc->xenv);
	else if (cache->retset)
		return call_set_returning_function(fcinfo, proc->xenv, cache->nargs, proc->argtypes, &proc->rettype);
	else
		return call_function(fcinfo, proc->xenv, cache->nargs, proc->argtypes, &proc->rettype);

	return (Datum) 0;
}

Datum
plmruby_inline_handler(PG_FUNCTION_ARGS)
{
	InlineCodeBlock *codeblock = (InlineCodeBlock *) DatumGetPointer(PG_GETARG_DATUM(0));

	Assert(IsA(codeblock, InlineCodeBlock));

	char * source_text = codeblock->source_text;

	plmruby_global_env *env = get_plmruby_global_env();
	mrb_state *mrb = env->mrb;
	int ai = mrb_gc_arena_save(mrb);

	mrbc_context *cxt = mrbc_context_new(mrb);
	cxt->capture_errors = TRUE;
	cxt->no_exec = TRUE;

	mrb_value proc = mrb_load_string_cxt(mrb, source_text, cxt);
	if (mrb->exc != NULL)
	{
		mrb_gc_arena_restore(mrb, ai);
		ereport_exception(mrb);
	}

	mrb_value receiver = mrb_obj_new(mrb, mrb->object_class, 0, NULL);
	mrb_run(mrb, mrb_proc_ptr(proc), receiver);
	if (mrb->exc != NULL)
	{
		mrb_gc_arena_restore(mrb, ai);
		ereport_exception(mrb);
	}

	mrb_gc_arena_restore(mrb, ai);
	PG_RETURN_VOID();
}

void
_PG_init(void)
{
	init_plmruby_env_cache();
	init_proc_cache_hash();

	RegisterXactCallback(plmruby_xact_cb, NULL);
}

static Datum
call_trigger(FunctionCallInfo fcinfo, plmruby_exec_env *xenv)
{
	return (Datum) 0;
}


static Datum
call_set_returning_function(FunctionCallInfo fcinfo, plmruby_exec_env *xenv,
							int nargs, plmruby_type argtypes[], plmruby_type *rettype)
{
	return (Datum) 0;
}


static Datum
call_function(FunctionCallInfo fcinfo, plmruby_exec_env *xenv,
			  int nargs, plmruby_type argtypes[], plmruby_type *rettype)
{
	elog(DEBUG1, "call function");
	mrb_value argv[FUNC_MAX_ARGS];
	for (int i = 0; i < nargs; ++i)
		argv[i] = datum_to_mrb_value(xenv->mrb, fcinfo->arg[i], fcinfo->argnull[i], &argtypes[i]);

	mrb_value result = mrb_funcall_with_block(xenv->mrb, xenv->proc, xenv->mid, nargs, argv, xenv->nil);
	/* TODO: error message */
	if (xenv->mrb->exc)
		ereport_exception(xenv->mrb);

	if(rettype != NULL)
		return mrb_value_to_datum(xenv->mrb, result, &fcinfo->isnull, rettype);
	else
		PG_RETURN_VOID();
}
