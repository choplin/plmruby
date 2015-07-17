#include <postgres.h>
#include <executor/spi.h>
#include <commands/trigger.h>
#include <catalog/pg_proc.h>
#include <catalog/pg_type.h>
#include <access/xact.h>

#include <mruby.h>
#include <mruby/proc.h>

#include "plmruby.h"
#include "plmruby_call.h"
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
		return call_set_returning_function(fcinfo, proc->xenv, cache->nargs, proc->argtypes);
	else
		return call_function(fcinfo, proc->xenv, cache->nargs, proc->argtypes, &proc->rettype);

	return (Datum) 0;
}

Datum
plmruby_inline_handler(PG_FUNCTION_ARGS)
{
	InlineCodeBlock *codeblock = (InlineCodeBlock *) DatumGetPointer(PG_GETARG_DATUM(0));

	Assert(IsA(codeblock, InlineCodeBlock));

	char *source_text = codeblock->source_text;

	plmruby_global_env *env = get_plmruby_global_env();
	mrb_state *mrb = env->mrb;
	int ai = mrb_gc_arena_save(mrb);

	StringInfoData src;
	initStringInfo(&src);
	appendStringInfo(&src, "Class.new do; def call; %s; end; end", source_text);
	mrb_value proc = mrb_load_string_cxt(mrb, src.data, env->cxt);
	if (mrb->exc != NULL)
	{
		pfree(src.data);
		mrb_gc_arena_restore(mrb, ai);
		ereport_exception(mrb);
	}
	pfree(src.data);

	PG_TRY();
	{
		plmruby_exec_env *xenv = create_plmruby_exec_env((struct RClass*) mrb_obj_ptr(proc));
		call_mruby_function(fcinfo, xenv, 0, NULL);
		if (mrb->exc != NULL)
			ereport_exception(mrb);
	}
	PG_CATCH();
	{
		mrb_gc_arena_restore(mrb, ai);
		PG_RE_THROW();
	}
	PG_END_TRY();

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
