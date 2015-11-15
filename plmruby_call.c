#include <postgres.h>
#include <commands/trigger.h>
#include <funcapi.h>
#include <catalog/pg_type.h>
#include <miscadmin.h>
#include <utils/rel.h>
#include <utils/lsyscache.h>

#include <mruby.h>
#include <mruby/array.h>
#include <mruby/class.h>

#include "plmruby_call.h"
#include "plmruby_proc.h"
#include "plmruby_tuple_converter.h"
#include "plmruby_util.h"

#define TRIGGER_ARGS_LEN 10

#define TRIGGER_UNMODIFIED(t) (TRIGGER_FIRED_BY_UPDATE((t)->tg_event) ? (t)->tg_newtuple : (t)->tg_trigtuple)

Datum
call_trigger(FunctionCallInfo fcinfo, plmruby_exec_env *xenv)
{
	// trigger arguments are:
	//	0: new
	//	1: old
	//	2: tg_name
	//	3: tg_when
	//	4: tg_level
	//	5: tg_op
	//	6: tg_relid
	//	7: tg_table_name
	//	8: tg_table_schema
	//	9: tg_argv
	TriggerData *trig = (TriggerData *) fcinfo->context;
	Relation rel = trig->tg_relation;
	TriggerEvent event = trig->tg_event;
	mrb_state *mrb = xenv->mrb;
	mrb_value args[TRIGGER_ARGS_LEN];

	if (TRIGGER_FIRED_FOR_ROW(event))
	{
		TupleDesc tupdesc = RelationGetDescr(rel);
		tuple_converter *converter = new_tuple_converter(mrb, tupdesc);

		if (TRIGGER_FIRED_BY_INSERT(event))
		{
			// new
			args[0] = tuple_to_mrb_value(converter, trig->tg_trigtuple);
			// old
			args[1] = xenv->nil;
		}
		else if (TRIGGER_FIRED_BY_DELETE(event))
		{
			// new
			args[0] = xenv->nil;
			// old
			args[1] = tuple_to_mrb_value(converter, trig->tg_trigtuple);
		}
		else if (TRIGGER_FIRED_BY_UPDATE(event))
		{
			// new
			args[0] = tuple_to_mrb_value(converter, trig->tg_newtuple);
			// old
			args[1] = tuple_to_mrb_value(converter, trig->tg_trigtuple);
		}
		delete_tuple_converter(converter);
	}
	else
	{
		args[0] = args[1] = xenv->nil;
	}

	// 2: tg_name
	char *tgname = trig->tg_trigger->tgname;
	args[2] = mrb_str_new_static(mrb, tgname, strlen(tgname));

	// 3: tg_when
	if (TRIGGER_FIRED_BEFORE(event))
		args[3] = mrb_symbol_value(mrb_intern_lit(mrb, "before"));
	else if (TRIGGER_FIRED_AFTER(event))
		args[3] = mrb_symbol_value(mrb_intern_lit(mrb, "after"));
	else
		args[3] = mrb_symbol_value(mrb_intern_lit(mrb, "instead_of"));

	// 4: tg_level
	if (TRIGGER_FIRED_FOR_ROW(event))
		args[4] = mrb_symbol_value(mrb_intern_lit(mrb, "row"));
	else
		args[4] = mrb_symbol_value(mrb_intern_lit(mrb, "statement"));

	// 5: tg_op
	if (TRIGGER_FIRED_BY_INSERT(event))
		args[5] = mrb_symbol_value(mrb_intern_lit(mrb, "insert"));
	else if (TRIGGER_FIRED_BY_DELETE(event))
		args[5] = mrb_symbol_value(mrb_intern_lit(mrb, "delete"));
	else if (TRIGGER_FIRED_BY_UPDATE(event))
		args[5] = mrb_symbol_value(mrb_intern_lit(mrb, "update"));
#ifdef TRIGGER_FIRED_BY_TRUNCATE
	else if (TRIGGER_FIRED_BY_TRUNCATE(event))
		args[5] = mrb_symbol_value(mrb_intern_lit(mrb, "truncate"));
#endif
	else
		args[5] = mrb_symbol_value(mrb_intern_lit(mrb, "UNKNOWN"));

	// 6: tg_relid
	args[6] = mrb_fixnum_value(RelationGetRelid(rel));

	// 7: tg_table_name
	char *relname = RelationGetRelationName(rel);
	args[7] = mrb_str_new_static(mrb, relname, strlen(relname));

	// 8: tg_table_schema
	char *namespace = get_namespace_name(RelationGetNamespace(rel));
	args[8] = mrb_str_new_static(mrb, namespace, strlen(namespace));

	// 9: tg_argv
	mrb_value argv = mrb_ary_new_capa(mrb, trig->tg_trigger->tgnargs);
	for (int i = 0; i < trig->tg_trigger->tgnargs; i++)
	{
		char *arg = trig->tg_trigger->tgargs[i];
		mrb_ary_push(mrb, argv, mrb_str_new_static(mrb, arg, strlen(arg)));
	}

	args[9] = argv;

	mrb_value ret = mrb_funcall_with_block(mrb, xenv->proc, xenv->mid, TRIGGER_ARGS_LEN, args, xenv->nil);

	if (mrb->exc)
		ereport_exception(mrb);

	if (TRIGGER_FIRED_FOR_STATEMENT(event) || TRIGGER_FIRED_AFTER(event))
		return PointerGetDatum(NULL);
	else if (mrb_nil_p(ret))
		return PointerGetDatum(TRIGGER_UNMODIFIED(trig));
	else if (mrb_symbol_p(ret))
	{
		mrb_sym ret_sym = mrb_symbol(ret);
		if (ret_sym == mrb_intern_lit(mrb, "ok"))
			return PointerGetDatum(TRIGGER_UNMODIFIED(trig));
		else if (ret_sym == mrb_intern_lit(mrb, "skip"))
			return PointerGetDatum(NULL);
		else
			ereport(ERROR,
					(errcode(ERRCODE_DATA_EXCEPTION),
							errmsg("returned an invalid symbol from trigger function")));
	}
	else
	{
		TupleDesc tupdesc = RelationGetDescr(rel);
		tuple_converter *converter = new_tuple_converter(mrb, tupdesc);
		// Trigger function must return a HeapTuple as it is, instead of calling HeapTupleGetDatum(heaptup)
		Datum datum = PointerGetDatum(mrb_value_to_heap_tuple(converter, ret, NULL, false));
		return datum;
	}
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
			mrb_value_to_heap_tuple(converter, mrb_ary_ref(mrb, result, i),
									rsinfo->setResult, functypclass == TYPEFUNC_SCALAR);
	}
	else
	{
		mrb_value enumerator;
		if (mrb_obj_is_instance_of(mrb, result, ENUMERATOR_CLASS))
			enumerator = result;
		else /* Enumerable */
			enumerator = mrb_funcall(mrb, result, "to_enum", 0);

		while (true)
		{
			mrb_value next = mrb_funcall(mrb, enumerator, "next", 0);
			if (mrb->exc)
			{
				if (mrb->exc->c == E_STOP_ITERATION)
				{
					mrb->exc = NULL;
					break;
				}
				else
					ereport_exception(mrb);
			}
			mrb_value_to_heap_tuple(converter, next, rsinfo->setResult,
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
