#ifndef __PLMRUBY_CALL_H__
#define __PLMRUBY_CALL_H__

#include <postgres.h>
#include <fmgr.h>

#include "plmruby_env.h"
#include "plmruby_type.h"

Datum
		call_trigger(PG_FUNCTION_ARGS, plmruby_exec_env *xenv);

Datum
		call_set_returning_function(PG_FUNCTION_ARGS, plmruby_exec_env *xenv,
									int nargs, plmruby_type argtypes[]);

Datum
		call_function(PG_FUNCTION_ARGS, plmruby_exec_env *xenv,
					  int nargs, plmruby_type argtypes[], plmruby_type *rettype);

mrb_value
		call_mruby_function(FunctionCallInfo fcinfo, plmruby_exec_env *xenv,
							int nargs, plmruby_type argtypes[]);

#endif /* __PLMRUBY_CALL_H__ */
