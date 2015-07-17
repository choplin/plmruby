#ifndef __PLMRUBY_UTIL_H__
#define __PLMRUBY_UTIL_H__

#include <mruby.h>


void
		ereport_exception(mrb_state *mrb);

void
		mrb_debug_p(mrb_state *mrb, mrb_value v);

#endif /* __PLMRUBY_UTIL_H__ */
