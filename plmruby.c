#include <postgres.h>
#include <executor/spi.h>
#include <commands/trigger.h>
#include <catalog/pg_proc.h>
#include <catalog/pg_type.h>
#include <access/xact.h>

#include "plmruby.h"
#include "plmruby_proc.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(plmruby_call_handler);

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


	if (!fcinfo->flinfo->fn_extra) {
		elog(INFO, "new plmruby proc");
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
	mrb_value argv[FUNC_MAX_ARGS];
	for (int i = 0; i < nargs; ++i)
		argv[i] = datum_to_mrb_value(xenv->mrb, fcinfo->arg[i], fcinfo->argnull[i], &argtypes[i]);

	mrb_funcall_with_block(xenv->mrb, xenv->proc, xenv->mid, nargs, argv, xenv->nil);
	return (Datum) 0;
}
