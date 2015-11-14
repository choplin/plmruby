#include <postgres.h>
#include <mruby.h>
#include <mruby/string.h>
#include <mruby/class.h>

#include "plmruby_util.h"

void
ereport_exception(mrb_state *mrb)
{
	/* TODO: add backtrace */
	mrb_value s = mrb_funcall(mrb, mrb_obj_value(mrb->exc), "inspect", 0);
	mrb->exc = NULL;
	if (mrb_string_p(s)) {
		char *err = mrb_str_to_cstr(mrb, s);
		ereport(ERROR, (errmsg("%s", err)));
	}
	else
		ereport(ERROR, (errmsg("unknown error occured")));
}
