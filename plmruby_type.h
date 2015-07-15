#ifndef __PLMRUBY_TYPE_H__
#define __PLMRUBY_TYPE_H__

#include <postgres.h>
#include <fmgr.h>

#include <mruby.h>

typedef struct {
	Oid typid;
	Oid ioparam;
	int16 len;
	bool byval;
	char align;
	char category;
	FmgrInfo fn_input;
	FmgrInfo fn_output;
} plmruby_type;

void
		plmruby_fill_type(plmruby_type *type, Oid typid, MemoryContext mcxt);

mrb_value
		datum_to_mrb_value(mrb_state *mrb, Datum datum, bool isnull, plmruby_type *type);

Datum
		mrb_value_to_datum(mrb_state *mrb, mrb_value value, bool *isnull, plmruby_type *type);



#endif /* __PLMRUBY_TYPE_H__ */
