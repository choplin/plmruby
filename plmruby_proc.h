#ifndef __PLMRUBY_PROC_H__
#define __PLMRUBY_PROC_H__

#include <postgres.h>
#include <fmgr.h>

#include "plmruby.h"
#include "plmruby_type.h"
#include "plmruby_env.h"

typedef struct {
	Oid fn_oid;

	void *function;
	char proname[NAMEDATALEN];
	char *prosrc;

	char *class_name;
	struct RClass *proc_class;

	TransactionId fn_xmin;
	ItemPointerData fn_tid;
	Oid user_id;

	int nargs;
	bool retset;
	Oid rettype;
	Oid argtypes[FUNC_MAX_ARGS];
} plmruby_proc_cache;

typedef struct {
	plmruby_proc_cache *cache;
	plmruby_exec_env *xenv;
	plmruby_type rettype;
	plmruby_type argtypes[FUNC_MAX_ARGS];
} plmruby_proc;

void
		init_proc_cache_hash(void);

plmruby_proc *
		new_plmruby_proc(Oid fn_oid, FunctionCallInfo fcinfo, bool validate, bool is_trigger);

#endif /* __PLMRUBY_PROC_H__ */
