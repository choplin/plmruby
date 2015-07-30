#include <postgres.h>
#include <mruby.h>
#include <mruby/string.h>
#include <mruby/class.h>

#include "plmruby_util.h"

#define DEBUG_P(v) elog(DEBUG1, #v ": %s", mrb_str_to_cstr(mrb, mrb_inspect(mrb, (v))))

void
ereport_exception(mrb_state *mrb)
{
	/* TODO: add backtrace */
	mrb_value s = mrb_funcall(mrb, mrb_obj_value(mrb->exc), "inspect", 0);
	if (mrb_string_p(s)) {
		char *err = mrb_str_to_cstr(mrb, s);
		mrb->exc = NULL;
		ereport(ERROR, (errmsg("%s", err)));
	}
	else
		ereport(ERROR, (errmsg("unknown error occured")));
}

void
mrb_debug_p(mrb_state *mrb, mrb_value v)
{
	DEBUG_P(v);
}
