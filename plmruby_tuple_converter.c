#include <postgres.h>
#include <access/htup_details.h>
#include <utils/memutils.h>

#include <mruby.h>
#include <mruby/array.h>
#include <mruby/hash.h>
#include <mruby/string.h>
#include <funcapi.h>

#include "plmruby_type.h"
#include "plmruby_tuple_converter.h"

tuple_converter *
new_tuple_converter(mrb_state *mrb, TupleDesc tupdesc)
{
	tuple_converter *converter = palloc(sizeof(tuple_converter));
	converter->tupdesc = tupdesc;

	converter->memcontext = AllocSetContextCreate(
			CurrentMemoryContext,
			"ConverterContext",
			ALLOCSET_SMALL_MINSIZE,
			ALLOCSET_SMALL_INITSIZE,
			ALLOCSET_SMALL_MAXSIZE);

	MemoryContext old_context = MemoryContextSwitchTo(converter->memcontext);
	converter->colnames = palloc(sizeof(mrb_value));
	converter->coltypes = palloc(sizeof(plmruby_type));
	MemoryContextSwitchTo(old_context);

	for (int c = 0; c < tupdesc->natts; ++c)
	{
		if (tupdesc->attrs[c]->attisdropped)
			continue;

		converter->colnames[c] = mrb_str_new_cstr(mrb, NameStr(tupdesc->attrs[c]->attname));

		plmruby_fill_type(&converter->coltypes[c],
						  tupdesc->attrs[c]->atttypid,
						  converter->memcontext);
	}
	return converter;
}

void
delete_tuple_converter(tuple_converter *converter)
{
	if (converter->memcontext != NULL)
	{
		MemoryContextDelete(converter->memcontext);
		converter->memcontext = NULL;
	}
	pfree(converter);
}

mrb_value
tuple_to_mrb_value(tuple_converter *converter, HeapTuple tuple)
{
	mrb_state *mrb = converter->mrb;
	int natts = converter->tupdesc->natts;
	mrb_value hash = mrb_hash_new_capa(converter->mrb, natts);

	for (int c = 0; c < natts; c++)
	{
		Datum datum;
		bool isnull;

		if (converter->tupdesc->attrs[c]->attisdropped)
			continue;

		datum = heap_getattr(tuple, c + 1, converter->tupdesc, &isnull);

		mrb_hash_set(converter->mrb, hash, converter->colnames[c],
					 datum_to_mrb_value(mrb, datum, isnull, &converter->coltypes[c]));
	}

	return hash;
}

Datum
mrb_value_to_tuple_datum(tuple_converter *converter, mrb_value value, Tuplestorestate *tupstore, bool is_scalar)
{
	Datum result;
	mrb_state *mrb = converter->mrb;
	TupleDesc tupdesc = converter->tupdesc;
	int natts = tupdesc->natts;

	if (!is_scalar && !mrb_hash_p(value))
		elog(ERROR, "argument must be hash");

	Datum *values = (Datum *) palloc(sizeof(Datum) * natts);
	bool *nulls = (bool *) palloc(sizeof(bool) * natts);

	if (!is_scalar)
	{
		mrb_value keys = mrb_hash_keys(mrb, value);

		for (int c = 0; c < natts; ++c)
		{
			if (tupdesc->attrs[c]->attisdropped)
				continue;

			bool found = false;
			mrb_value colname = converter->colnames[c];
			for (int d = 0; d < tupdesc->natts; ++d)
			{
				mrb_value key = mrb_ary_ref(mrb, keys, d);
				if (mrb_str_equal(mrb, colname, key))
				{
					found = true;
					break;
				}
			}
			if (!found)
				elog(ERROR, "field name / property name mismatch");
		}
	}

	for (int c = 0; c < natts; ++c)
	{
		/* Make sure dropped columns are skipped by backend code. */
		if (tupdesc->attrs[c]->attisdropped)
		{
			nulls[c] = true;
			continue;
		}

		mrb_value attr = is_scalar ? value : mrb_hash_get(mrb, value, converter->colnames[c]);
		if (mrb_nil_p(attr) || mrb_undef_p(attr))
			nulls[c] = true;
		else
			values[c] = mrb_value_to_datum(mrb, attr, &nulls[c], &converter->coltypes[c]);
	}

	if (tupstore)
	{
		tuplestore_putvalues(tupstore, tupdesc, values, nulls);
		result = (Datum) 0;
	}
	else
	{
		result = HeapTupleGetDatum(heap_form_tuple(tupdesc, values, nulls));
	}

	pfree(values);
	pfree(nulls);

	return result;
}
