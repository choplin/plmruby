#include <postgres.h>

#include <mruby.h>
#include <mruby/string.h>
#include <lib/stringinfo.h>

#define DEFINE_GLOBAL_CONST(v) mrb_define_global_const(mrb, #v, mrb_fixnum_value(v))

mrb_value
plmruby_elog(mrb_state *mrb, mrb_value self)
{
	mrb_int elevel;
	mrb_value v;
	mrb_value *rest;
	mrb_int rest_len;
	StringInfoData msg;

	initStringInfo(&msg);

	mrb_get_args(mrb, "io*", &elevel, &v, &rest, &rest_len);

	switch ((int) elevel)
	{
		case DEBUG5:
		case DEBUG4:
		case DEBUG3:
		case DEBUG2:
		case DEBUG1:
		case LOG:
		case INFO:
		case NOTICE:
		case WARNING:
		case ERROR:
			break;
		default:
			mrb_raisef(mrb, E_ARGUMENT_ERROR, "invalid error level %S", mrb_fixnum_value(elevel));
	}

	appendStringInfoString(&msg, mrb_str_to_cstr(mrb, mrb_str_to_str(mrb, v)));

	for (mrb_int i = 0; i < rest_len; i++)
	{
		mrb_value r = rest[i];
		appendStringInfo(&msg, " %s", mrb_str_to_cstr(mrb, mrb_str_to_str(mrb, r)));
	}

	elog((int) elevel, "%s", msg.data);
	pfree(msg.data);
	return mrb_nil_value();
}


void
mrb_plmruby_gem_init(mrb_state *mrb)
{
	DEFINE_GLOBAL_CONST(DEBUG5);
	DEFINE_GLOBAL_CONST(DEBUG4);
	DEFINE_GLOBAL_CONST(DEBUG3);
	DEFINE_GLOBAL_CONST(DEBUG2);
	DEFINE_GLOBAL_CONST(DEBUG1);
	DEFINE_GLOBAL_CONST(DEBUG5);
	DEFINE_GLOBAL_CONST(LOG);
	DEFINE_GLOBAL_CONST(INFO);
	DEFINE_GLOBAL_CONST(NOTICE);
	DEFINE_GLOBAL_CONST(WARNING);
	DEFINE_GLOBAL_CONST(ERROR);

	mrb_define_method(mrb, mrb->kernel_module, "elog", plmruby_elog, MRB_ARGS_REQ(2) | MRB_ARGS_REST());
}

void
mrb_plmruby_gem_final(mrb_state *mrb)
{
}
