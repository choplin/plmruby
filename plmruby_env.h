#ifndef __PLMRUBY_ENV_H__
#define __PLMRUBY_ENV_H__

#include <mruby.h>
#include <mruby/compile.h>

/*
 * Holds mruby runtime for each user.
 * An instance of this struct is allocated when each user calls plmruby function for the first time,
 * and lives as long as backend.
 */
typedef struct {
	mrb_state *mrb;
	mrbc_context *cxt;
} plmruby_global_env;

/*
 * Holds context which is required for function execution and cleanup.
 */
typedef struct plmruby_exec_env {
	int ai;
	/* arena index to which mrb_state has to restore at the end of transaction to enable gc collect proc object */
	mrb_state *mrb;
	mrb_value proc;
	mrb_sym mid;
	/* always :call */
	mrb_value nil;
	/* passed as blk arg */
	struct plmruby_exec_env *next;
} plmruby_exec_env;

void
		init_plmruby_env_cache(void);

plmruby_global_env *
		get_plmruby_global_env();

plmruby_exec_env *
		create_plmruby_exec_env(struct RClass *proc_class);

void
		cleanup_plmruby_exec_env(void);


#endif /* __PLMRUBY_ENV_H__ */