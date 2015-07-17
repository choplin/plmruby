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
	converter->mrb = mrb;
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

	for (int i = 0; i < tupdesc->natts; ++i)
	{
		if (tupdesc->attrs[i]->attisdropped)
			continue;

		converter->colnames[i] = mrb_str_new_cstr(mrb, NameStr(tupdesc->attrs[i]->attname));

		plmruby_fill_type(&converter->coltypes[i],
						  tupdesc->attrs[i]->atttypid,
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

	for (int i = 0; i < natts; ++i)
	{
		Datum datum;
		bool isnull;

		if (converter->tupdesc->attrs[i]->attisdropped)
			continue;

		datum = heap_getattr(tuple, i + 1, converter->tupdesc, &isnull);

		mrb_hash_set(converter->mrb, hash, converter->colnames[i],
					 datum_to_mrb_value(mrb, datum, isnull, &converter->coltypes[i]));
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

		for (int i = 0; i < natts; ++i)
		{
			if (tupdesc->attrs[i]->attisdropped)
				continue;

			bool found = false;
			mrb_value colname = converter->colnames[i];
			for (int j = 0; j < tupdesc->natts; ++j)
			{
				mrb_value key = mrb_ary_ref(mrb, keys, j);
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

	for (int i = 0; i < natts; ++i)
	{
		/* Make sure dropped columns are skipped by backend code. */
		if (tupdesc->attrs[i]->attisdropped)
		{
			nulls[i] = true;
			continue;
		}

		mrb_value attr = is_scalar ? value : mrb_hash_get(mrb, value, converter->colnames[i]);
		if (mrb_nil_p(attr) || mrb_undef_p(attr))
			nulls[i] = true;
		else
			values[i] = mrb_value_to_datum(mrb, attr, &nulls[i], &converter->coltypes[i]);
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
