#include <postgres.h>
#include <access/htup_details.h>
#include <catalog/pg_proc.h>
#include <catalog/pg_type.h>
#include <commands/trigger.h>
#include <funcapi.h>
#include <miscadmin.h>
#include <utils/builtins.h>
#include <utils/lsyscache.h>
#include <utils/memutils.h>
#include <utils/syscache.h>
#include <lib/stringinfo.h>

#include <mruby.h>
#include <mruby/class.h>

#include "plmruby_proc.h"
#include "plmruby_util.h"

#define PROC_CACHE_HASH_NELEM 32

static HTAB *plmruby_proc_cache_hash = NULL;

static plmruby_proc_cache *
		get_proc_cache(Oid fn_oid, bool validate, bool is_trigger);

static bool
		cache_is_up_to_date(plmruby_proc_cache *cache, HeapTuple procCache);

static bool
		cache_is_compiled(plmruby_proc_cache *cache);

static void
		free_cache(plmruby_proc_cache *cache);

static bool
		supported_arg_type(Oid typid);

static struct RClass *
		compile_mruby(Oid fn_oid, const char *prosrc, int nargs, const char **argnames, bool is_trigger);

void
init_proc_cache_hash(void)
{
	HASHCTL hash_ctl = {0};

	hash_ctl.keysize = sizeof(Oid);
	hash_ctl.entrysize = sizeof(plmruby_proc_cache);
	hash_ctl.hash = oid_hash;
	plmruby_proc_cache_hash = hash_create("PLmruby Procedures", PROC_CACHE_HASH_NELEM,
										  &hash_ctl, HASH_ELEM | HASH_FUNCTION);
}

plmruby_proc *
new_plmruby_proc(Oid fn_oid, FunctionCallInfo fcinfo, bool validate, bool is_trigger)
{
	plmruby_proc_cache *cache = get_proc_cache(fn_oid, validate, is_trigger);

	MemoryContext mcxt = CurrentMemoryContext;
	if (!validate)
		mcxt = fcinfo->flinfo->fn_mcxt;

	plmruby_proc *proc = (plmruby_proc *) MemoryContextAllocZero(mcxt, offsetof(plmruby_proc, argtypes) +
										   sizeof(plmruby_type) * cache->nargs);

	proc->cache = cache;
	for (int i = 0; i < cache->nargs; i++)
	{
		Oid argtype = cache->argtypes[i];
		/* Resolve polymorphic types, if this is an actual call context. */
		if (!validate && IsPolymorphicType(argtype))
			argtype = get_fn_expr_argtype(fcinfo->flinfo, i);
		plmruby_fill_type(&proc->argtypes[i], argtype, mcxt);
	}

	Oid rettype = cache->rettype;
	/* Resolve polymorphic return type if this is an actual call context. */
	if (!validate && IsPolymorphicType(rettype))
		rettype = get_fn_expr_rettype(fcinfo->flinfo);
	plmruby_fill_type(&proc->rettype, rettype, mcxt);

	return proc;
}

static plmruby_proc_cache *
get_proc_cache(Oid fn_oid, bool validate, bool is_trigger)
{
	HeapTuple procTup;
	plmruby_proc_cache *cache;
	bool found;
	bool isnull;
	Datum prosrc;
	Oid *argtypes;
	char **argnames;
	char *argmodes;
	MemoryContext oldcontext;

	procTup = SearchSysCache(PROCOID, ObjectIdGetDatum(fn_oid), 0, 0, 0);
	if (!HeapTupleIsValid(procTup))
		elog(ERROR, "cache lookup failed for function %u", fn_oid);

	cache = (plmruby_proc_cache *) hash_search(plmruby_proc_cache_hash, &fn_oid, HASH_ENTER, &found);

	if (found)
	{
		/*
		 * We need to check user id and dispose it if it's different from
		 * the previous cache user id, as the mruby function is associated
		 * with the context where it was generated. In most cases,
		 * we can expect this doesn't affect runtime performance.
		 */
		if (cache_is_up_to_date(cache, procTup))
		{
			ReleaseSysCache(procTup);
			return cache;
		}
		else
			free_cache(cache);
	}

	Form_pg_proc procStruct;

	procStruct = (Form_pg_proc) GETSTRUCT(procTup);

	prosrc = SysCacheGetAttr(PROCOID, procTup, Anum_pg_proc_prosrc, &isnull);
	if (isnull)
		elog(ERROR, "null prosrc");

	oldcontext = MemoryContextSwitchTo(TopMemoryContext);
	cache->prosrc = TextDatumGetCString(prosrc);
	MemoryContextSwitchTo(oldcontext);

	cache->retset = procStruct->proretset;
	cache->rettype = procStruct->prorettype;
	strlcpy(cache->proname, NameStr(procStruct->proname), NAMEDATALEN);
	cache->fn_xmin = HeapTupleHeaderGetXmin(procTup->t_data);
	cache->fn_tid = procTup->t_self;
	cache->user_id = GetUserId();

	int nargs = get_func_arg_info(procTup, &argtypes, &argnames, &argmodes);

	if (validate)
	{
		/*
		 * Disallow non-polymorphic pseudotypes in arguments
		 * (either IN or OUT).  Internal type is used to declare
		 * mruby functions for find_function().
		 */
		for (int i = 0; i < nargs; i++)
		{
			if (supported_arg_type(argtypes[i]))
			{
				ereport(ERROR,
						(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
								errmsg("PL/mruby functions cannot accept type %s",
									   format_type_be(argtypes[i]))));
			}
		}
	}

	ReleaseSysCache(procTup);

	int inargs = 0;
	for (int i = 0; i < nargs; i++)
	{
		Oid argtype = argtypes[i];
		char argmode = argmodes != NULL ? argmodes[i] : (char) PROARGMODE_IN;

		switch (argmode)
		{
			case PROARGMODE_IN:
			case PROARGMODE_INOUT:
			case PROARGMODE_VARIADIC:
				break;
			default:
				continue;
		}

		if (argnames != NULL)
			argnames[inargs] = argnames[i];

		cache->argtypes[inargs] = argtype;
		inargs++;
	}
	cache->nargs = inargs;

	cache->proc_class = compile_mruby(cache->fn_oid, cache->prosrc, cache->nargs, (const char **) argnames,
									  is_trigger);

	return cache;
}

static bool
cache_is_up_to_date(plmruby_proc_cache *cache, HeapTuple procTup)
{
	return (cache_is_compiled(cache)) &&
		   cache->fn_xmin == HeapTupleHeaderGetXmin(procTup->t_data) &&
		   ItemPointerEquals(&cache->fn_tid, &procTup->t_self) &&
		   cache->user_id == GetUserId();
}

static bool
cache_is_compiled(plmruby_proc_cache *cache)
{
	return cache->function != NULL;
}

static void
free_cache(plmruby_proc_cache *cache)
{
	if (cache->prosrc)
	{
		pfree(cache->prosrc);
		cache->prosrc = NULL;
	}
}

static bool
supported_arg_type(Oid typid)
{
	return get_typtype(typid) != TYPTYPE_PSEUDO ||
		   typid != INTERNALOID ||
		   !IsPolymorphicType(typid);
}

static struct RClass *
compile_mruby(Oid fn_oid, const char *prosrc, int nargs, const char **argnames, bool is_trigger)
{
	StringInfoData class_name;
	StringInfoData src;

	initStringInfo(&class_name);
	appendStringInfo(&class_name, "PLMRUBY_%u", fn_oid);

	plmruby_global_env *env = get_plmruby_global_env();
	/*
	 *  class PLMRUBY_<fn_oid>
	 *      def call(<arg1 ,...>
	 *          <prosrc>
	 *      end
	 *  end
	 */
	initStringInfo(&src);
	appendStringInfo(&src, "class %s; def call(", class_name.data);
	if (is_trigger)
	{
		appendStringInfoString(&src,
							   "new, old, tg_name, tg_when, tg_level, tg_op, "
									   "tg_relid, tg_table_name, tg_table_schema, tg_argv");
	}
	else
	{
		for (int i = 0; i < nargs; ++i)
		{
			if (i > 0)
				appendStringInfoChar(&src, ',');

			if (argnames != NULL && argnames[i] != NULL)
				appendStringInfoString(&src, argnames[i]);
			else
				/*
				 * unnamed argument to _N. You cannot define these with $N as other pl languages
				 * because , in Ruby, a variable whose name begins with '$' always means global variable.
				 */
				appendStringInfo(&src, "_%d", i + 1);
		}
	}
	appendStringInfo(&src, "); %s; end; end;", prosrc);

	mrb_load_string_cxt(env->mrb, src.data, env->cxt);
	if (env->mrb->exc != NULL)
		ereport_exception(env->mrb);

	struct RClass *class = mrb_class_get(env->mrb, class_name.data);

	pfree(src.data);
	pfree(class_name.data);

	return class;
}
