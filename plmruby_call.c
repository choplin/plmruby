#include <postgres.h>
#include <utils/tuplestore.h>
#include <nodes/execnodes.h>
#include <funcapi.h>
#include <catalog/pg_type.h>
#include <miscadmin.h>

#include <mruby.h>
#include <mruby/array.h>
#include <mruby/class.h>
#include <mruby/object.h>

#include "plmruby_call.h"
#include "plmruby_proc.h"
#include "plmruby_tuple_converter.h"
#include "plmruby_util.h"

Datum
call_trigger(FunctionCallInfo fcinfo, plmruby_exec_env *xenv)
{
	return (Datum) 0;
}

Datum
call_set_returning_function(FunctionCallInfo fcinfo, plmruby_exec_env *xenv,
							int nargs, plmruby_type argtypes[])
{
	mrb_state *mrb = xenv->mrb;
	plmruby_proc *proc = (plmruby_proc *) fcinfo->flinfo->fn_extra;
	ReturnSetInfo *rsinfo = (ReturnSetInfo *) fcinfo->resultinfo;
	TypeFuncClass functypclass = get_call_result_type(fcinfo, NULL, NULL);

	MemoryContext per_query_ctx;
	MemoryContext oldcontext;

	/* check to see if caller supports us returning a tuplestore */
	if (rsinfo == NULL || !IsA(rsinfo, ReturnSetInfo))
		ereport(ERROR,
				(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
						errmsg("set-valued function called in context that cannot accept a set")));

	if (proc->rettype.typid == RECORDOID && functypclass != TYPEFUNC_COMPOSITE)
		ereport(ERROR,
				(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
						errmsg("function returning record called in context "
									   "that cannot accept type record")));

	if (!(rsinfo->allowedModes & SFRM_Materialize))
		ereport(ERROR,
				(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
						errmsg("materialize mode required, but it is not "
									   "allowed in this context")));

	mrb_value result = call_mruby_function(fcinfo, xenv, nargs, argtypes);
	if (mrb->exc)
		ereport_exception(mrb);

	/* TODO: should be cached? */
	struct RClass *enumerator_class = mrb_class_get(mrb, "Enumerator");
	struct RClass *stop_iteration_class = mrb_class_get(mrb, "StopIteration");

	/*
	 * Accept Enumerator or any object which has each method.
	 * Since Enumerator has also each, it checks only for each method here.
	 */
	if (!mrb_respond_to(mrb, result, mrb_intern_cstr(mrb, "each")))
		ereport(ERROR,
				(errcode(ERRCODE_DATATYPE_MISMATCH),
						errmsg("set-returning plmruby function must return "
									   "Enumerator or an object which has each method")));

	per_query_ctx = rsinfo->econtext->ecxt_per_query_memory;
	oldcontext = MemoryContextSwitchTo(per_query_ctx);
	rsinfo->returnMode = SFRM_Materialize;
	rsinfo->setResult = tuplestore_begin_heap((bool) rsinfo->allowedModes & SFRM_Materialize_Random,
											  false, work_mem);
	rsinfo->setDesc = CreateTupleDescCopy(rsinfo->expectedDesc);
	MemoryContextSwitchTo(oldcontext);

	tuple_converter *converter = new_tuple_converter(mrb, rsinfo->setDesc);

	if (mrb_array_p(result))
	{
		mrb_int len = RARRAY_LEN(result);
		for (int i = 0; i < len; ++i)
			mrb_value_to_tuple_datum(converter, mrb_ary_ref(mrb, result, i),
									 rsinfo->setResult, functypclass == TYPEFUNC_SCALAR);
	}
	else
	{
		mrb_value enumerator;
		if (mrb_obj_is_instance_of(mrb, result, enumerator_class))
			enumerator = result;
		else /* Enumerable */
			enumerator = mrb_funcall(mrb, result, "to_enum", 0);

		while (true)
		{
			mrb_value next = mrb_funcall(mrb, enumerator, "next", 0);
			if (mrb->exc)
			{
				if (mrb->exc->c == stop_iteration_class)
					break;
				else
					ereport_exception(mrb);
			}
			mrb_value_to_tuple_datum(converter, next, rsinfo->setResult,
									 functypclass == TYPEFUNC_SCALAR);
		}
	}

	tuplestore_donestoring(tupstore);
	delete_tuple_converter(converter);

	return (Datum) 0;
}

Datum
call_function(FunctionCallInfo fcinfo, plmruby_exec_env *xenv,
			  int nargs, plmruby_type argtypes[], plmruby_type *rettype)
{
	mrb_value result = call_mruby_function(fcinfo, xenv, nargs, argtypes);
	if (xenv->mrb->exc)
		ereport_exception(xenv->mrb);

	if (rettype != NULL)
		return mrb_value_to_datum(xenv->mrb, result, &fcinfo->isnull, rettype);
	else
		PG_RETURN_VOID();
}

mrb_value
call_mruby_function(FunctionCallInfo fcinfo, plmruby_exec_env *xenv,
					int nargs, plmruby_type argtypes[])
{
	mrb_value argv[FUNC_MAX_ARGS];
	for (int i = 0; i < nargs; ++i)
		argv[i] = datum_to_mrb_value(xenv->mrb, fcinfo->arg[i], fcinfo->argnull[i], &argtypes[i]);

	return mrb_funcall_with_block(xenv->mrb, xenv->proc, xenv->mid, nargs, argv, xenv->nil);
}
