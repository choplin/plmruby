#include <postgres.h>
#include <miscadmin.h>
#include <utils/memutils.h>

#include <mruby.h>
#include <mruby/string.h>

#include "plmruby_env.h"

#define INITIAL_LEN 16

typedef struct {
	plmruby_global_env *env;
	Oid user_id;
} env_entry;

static env_entry *envs = NULL;
static int envs_max_len = 0;
static int envs_len = 0;

static plmruby_exec_env *exec_env_head;

static plmruby_global_env *new_env(void);

static void extend_envs(int new_len);

static void append_env(plmruby_global_env *env, Oid user_id);

void
init_plmruby_env_cache(void)
{
	envs_max_len = INITIAL_LEN;
	envs = MemoryContextAlloc(TopMemoryContext, sizeof(env_entry) * envs_max_len);
}

plmruby_global_env*
get_plmruby_global_env(void)
{
	Oid user_id = GetUserId();
	for (int i = 0; i < envs_len; ++i)
	{
		if (envs[i].user_id == user_id)
			return envs[i].env;
	}

	plmruby_global_env *env = new_env();
	append_env(env, user_id);
	return env;
}

plmruby_exec_env *
create_plmruby_exec_env(struct RClass *proc_class)
{
	plmruby_global_env *env = get_plmruby_global_env();
	plmruby_exec_env *xenv = (plmruby_exec_env *) MemoryContextAllocZero(TopTransactionContext, sizeof(plmruby_exec_env));

	xenv->ai = mrb_gc_arena_save(env->mrb);
	xenv->mrb = env->mrb;
	xenv->mid = mrb_intern_cstr(env->mrb, "call");
	xenv->nil = mrb_nil_value();
	xenv->proc = mrb_obj_new(env->mrb, proc_class, 0, NULL);

	xenv->next = exec_env_head;
	exec_env_head = xenv;

	return xenv;
}

void
cleanup_plmruby_exec_env(void)
{
	plmruby_exec_env *xenv = exec_env_head;

	while (xenv != NULL)
	{
		mrb_gc_arena_restore(xenv->mrb, xenv->ai);
		xenv = xenv->next;
	}
	exec_env_head = NULL;
}

static plmruby_global_env *
new_env(void)
{
	plmruby_global_env *env = MemoryContextAlloc(TopMemoryContext, sizeof(plmruby_global_env));
	env->mrb = mrb_open();
	env->cxt = mrbc_context_new(env->mrb);
	env->cxt->capture_errors = TRUE;
	return env;
}

static void
extend_envs(int new_len)
{
	if (new_len == envs_max_len)
	{
		envs_max_len = envs_max_len * 2;
		MemoryContext old_context = MemoryContextSwitchTo(TopMemoryContext);
		repalloc(envs, sizeof(env_entry) * envs_max_len);
		MemoryContextSwitchTo(old_context);
	}
}

static void
append_env(plmruby_global_env *env, Oid user_id)
{
	int new_len = envs_len + 1;
	extend_envs(new_len);

	env_entry entry = envs[new_len - 1];
	entry.env = env;
	entry.user_id = GetUserId();

	envs_len = new_len;
}
