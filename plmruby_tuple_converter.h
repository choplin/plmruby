#ifndef __PLMRUBY_TUPLE_CONVERTER_H__
#define __PLMRUBY_TUPLE_CONVERTER_H__

#include <access/htup.h>
#include <access/tupdesc.h>
#include <utils/tuplestore.h>

#include <mruby.h>

typedef struct {
	mrb_state *mrb;
	TupleDesc tupdesc;
	mrb_value *colnames;
	plmruby_type *coltypes;
	MemoryContext memcontext;
} tuple_converter;

tuple_converter *
		new_tuple_converter(mrb_state *mrb, TupleDesc tupdesc);

void
		delete_tuple_converter(tuple_converter *converter);

mrb_value
		tuple_to_mrb_value(tuple_converter *converter, HeapTuple tuple);

Datum
		mrb_value_to_tuple_datum(tuple_converter *converter, mrb_value value,
								 Tuplestorestate *tupstore, bool is_scalar);

#endif /* __PLMRUBY_TUPLE_CONVERTER_H__ */
