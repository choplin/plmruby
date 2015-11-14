#ifndef __PLMRUBY_UTIL_H__
#define __PLMRUBY_UTIL_H__

#include <mruby.h>

/* TODO: cache classes */
#define ENUMERATOR_CLASS (mrb_class_get(mrb, "Enumerator"))
#define JSON_MODULE (mrb_module_get(mrb, "JSON"))
#define TIME_CLASS (mrb_class_get(mrb, "Time"))
#define XML_MODULE (mrb_module_get(mrb, "TineXML2"))
#define XML_DOCUMENT_CLASS (mrb_class_get_under(mrb, XML_MODULE, "XMLDocument"))
#define E_STOP_ITERATION (mrb_class_get(mrb, "StopIteration"))

#define DEBUG_P(mrb, v) elog(DEBUG1, #v ": %s", mrb_str_to_cstr((mrb), mrb_inspect((mrb), (v))))

void
		ereport_exception(mrb_state *mrb);


#endif /* __PLMRUBY_UTIL_H__ */
