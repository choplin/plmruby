#include <postgres.h>
#include <access/htup_details.h>
#include <catalog/pg_type.h>
#include <mb/pg_wchar.h>
#include <utils/builtins.h>
#include <utils/date.h>
#include <utils/datetime.h>
#include <utils/lsyscache.h>
#include <utils/array.h>
#include <utils/typcache.h>
#include <mruby.h>
#include <mruby/array.h>
#include <mruby/class.h>
#include <mruby/string.h>
#include <funcapi.h>

#include "plmruby_type.h"
#include "plmruby_util.h"
#include "plmruby_tuple_converter.h"

#define JDATE_OFFSET (POSTGRES_EPOCH_JDATE - UNIX_EPOCH_JDATE)

static mrb_value
		array_datum_to_mrb_value(mrb_state *mrb, Datum datum, plmruby_type *type);

static mrb_value
		record_datum_to_mrb_value(mrb_state *mrb, Datum datum);

static mrb_value
		scalar_datum_to_mrb_value(mrb_state *mrb, Datum datum, plmruby_type *type);

static Datum
		mrb_value_to_array_datum(mrb_state *mrb, mrb_value value, bool *isnull, plmruby_type *type);

static Datum
		mrb_value_to_record_datum(mrb_state *mrb, mrb_value value, bool *isnull, plmruby_type *type);

static Datum
		mrb_value_to_scalar_datum(mrb_state *mrb, mrb_value value, bool *isnull, plmruby_type *type);

static int64
		timestamptz_to_epoch_us(TimestampTz tm);

static int64
		date_to_epoch_us(DateADT date);

static mrb_value
		epoch_us_to_mrb_time(mrb_state *mrb, int64 epoch);

static Datum
		epoch_us_to_timestamptz(int64 epoch);

static Datum
		epoch_us_to_date(int64 epoch);

static int64
		mrb_time_to_epoch_us(mrb_state *mrb, mrb_value time);

static mrb_value
		datum_to_mrb_string(mrb_state *mrb, Datum value, plmruby_type *type);

static mrb_value
		to_mrb_string(mrb_state *mrb, const char *str, size_t len);

static mrb_value
		to_mrb_string_encoding(mrb_state *mrb, const char *str, size_t len, int encoding);

static char *
		mrb_str_to_cstr_palloc(mrb_value mrb_str);

static Datum
		mrb_string_to_text_datum(mrb_value mrb_str);

void
plmruby_fill_type(plmruby_type *type, Oid typid, MemoryContext mcxt)
{
	bool ispreferred;

	if (!mcxt)
		mcxt = CurrentMemoryContext;

	type->typid = typid;
	type->fn_input.fn_mcxt = type->fn_output.fn_mcxt = mcxt;
	get_type_category_preferred(typid, &type->category, &ispreferred);

	if (type->category == TYPCATEGORY_ARRAY)
	{
		Oid elemid = get_element_type(typid);
		if (elemid == InvalidOid)
			ereport(ERROR,
					(errmsg("cannot determine element type of array: %u", typid)));
		type->typid = elemid;
	}

	get_typlenbyvalalign(type->typid, &type->len, &type->byval, &type->align);
}

mrb_value
datum_to_mrb_value(mrb_state *mrb, Datum datum, bool isnull, plmruby_type *type)
{
	if (isnull)
		return mrb_nil_value();
	else if (type->category == TYPCATEGORY_ARRAY || type->typid == RECORDARRAYOID)
		return array_datum_to_mrb_value(mrb, datum, type);
	else if (type->category == TYPCATEGORY_COMPOSITE || type->typid == RECORDOID)
		return record_datum_to_mrb_value(mrb, datum);
	else
		return scalar_datum_to_mrb_value(mrb, datum, type);
}

Datum
mrb_value_to_datum(mrb_state *mrb, mrb_value value, bool *isnull, plmruby_type *type)
{
	if (type->category == TYPCATEGORY_ARRAY)
		return mrb_value_to_array_datum(mrb, value, isnull, type);
	else
		return mrb_value_to_scalar_datum(mrb, value, isnull, type);
}

mrb_value
array_datum_to_mrb_value(mrb_state *mrb, Datum datum, plmruby_type *type)
{
	Datum *values;
	bool *nulls;
	int nelems;

	deconstruct_array(DatumGetArrayTypeP(datum),
					  type->typid, type->len, type->byval, type->align,
					  &values, &nulls, &nelems);
	mrb_value result = mrb_ary_new_capa(mrb, nelems);
	plmruby_type base = {0};
	bool ispreferred;

	base.typid = type->typid;
	if (base.typid == RECORDARRAYOID)
		base.typid = RECORDOID;

	base.fn_input.fn_mcxt = base.fn_output.fn_mcxt = type->fn_input.fn_mcxt;
	get_type_category_preferred(base.typid, &(base.category), &ispreferred);
	get_typlenbyvalalign(base.typid, &(base.len), &(base.byval), &(base.align));

	for (int i = 0; i < nelems; ++i)
		mrb_ary_push(mrb, result, datum_to_mrb_value(mrb, values[i], nulls[i], &base));

	pfree(values);
	pfree(nulls);

	return result;
}

mrb_value
record_datum_to_mrb_value(mrb_state *mrb, Datum datum)
{
	HeapTupleHeader rec = DatumGetHeapTupleHeader(datum);
	Oid tupType;
	int32 tupTypmod;
	TupleDesc tupdesc;
	HeapTupleData tuple;

	/* Extract type info from the tuple itself */
	tupType = HeapTupleHeaderGetTypeId(rec);
	tupTypmod = HeapTupleHeaderGetTypMod(rec);
	tupdesc = lookup_rowtype_tupdesc(tupType, tupTypmod);

	tuple_converter *converter = new_tuple_converter(mrb, tupdesc);

	/* Build a temporary HeapTuple control structure */
	tuple.t_len = HeapTupleHeaderGetDatumLength(rec);
	ItemPointerSetInvalid(&(tuple.t_self));
	tuple.t_tableOid = InvalidOid;
	tuple.t_data = rec;

	mrb_value result = tuple_to_mrb_value(converter, &tuple);

	ReleaseTupleDesc(tupdesc);
	delete_tuple_converter(converter);

	return result;
}


static mrb_value
scalar_datum_to_mrb_value(mrb_state *mrb, Datum datum, plmruby_type *type)
{
	switch (type->typid)
	{
		case OIDOID:
			return mrb_fixnum_value(DatumGetObjectId(datum));
		case BOOLOID:
		{
			if (DatumGetBool(datum))
			{
				return mrb_true_value();
			}
			else
			{
				return mrb_false_value();
			}
		}
		case INT2OID:
			return mrb_fixnum_value(DatumGetInt16(datum));
		case INT4OID:
			return mrb_fixnum_value(DatumGetInt32(datum));
		case INT8OID:
			return mrb_fixnum_value(DatumGetInt64(datum));
		case FLOAT4OID:
			return mrb_float_value(mrb, DatumGetFloat4(datum));
		case FLOAT8OID:
			return mrb_float_value(mrb, DatumGetFloat8(datum));
		case NUMERICOID:
			return mrb_float_value(mrb, DatumGetFloat8(
					DirectFunctionCall1(numeric_float8, datum)));
		case DATEOID:
		{
			int64 epoch = date_to_epoch_us(DatumGetDateADT(datum));
			mrb_value result = epoch_us_to_mrb_time(mrb, epoch);
			if (mrb->exc)
				ereport_exception(mrb);

			return result;
		}
		case TIMESTAMPOID:
		case TIMESTAMPTZOID:
		{
			int64 epoch = timestamptz_to_epoch_us(DatumGetTimestampTz(datum));
			mrb_value result = epoch_us_to_mrb_time(mrb, epoch);
			if (mrb->exc)
				ereport_exception(mrb);

			return result;
		}
		case TEXTOID:
		case VARCHAROID:
		case BPCHAROID:
		{
			void *p = PG_DETOAST_DATUM_PACKED(datum);
			const char *str = VARDATA_ANY(p);
			size_t len = VARSIZE_ANY_EXHDR(p);

			mrb_value result = to_mrb_string(mrb, str, len);

			if (p != DatumGetPointer(datum))
				pfree(p); /* free if detoasted */

			return result;
		}
		/*
		case XMLOID:
		{
			void *p = PG_DETOAST_DATUM_PACKED(datum);
			const char *str = VARDATA_ANY(p);
			size_t len = VARSIZE_ANY_EXHDR(p);

			mrb_value xml_str = to_mrb_string(mrb, str, len);

			mrb_value doc = mrb_obj_new(mrb, XML_DOCUMENT_CLASS, 0, NULL);

			mrb_value result = mrb_funcall(mrb, doc, "parse", 1, xml_str);

			if (p != DatumGetPointer(datum))
				pfree(p); // free if detoasted

			// TODO: error message
			if (mrb_symbol(result) != mrb_intern_cstr(mrb, "XML_SUCCESS"))
				elog(ERROR, "fail to parse xml");

			if (mrb->exc)
				ereport_exception(mrb);

			return doc;
		}
		*/
#if PG_VERSION_NUM >= 90200
		case JSONOID:
		{
			void *p = PG_DETOAST_DATUM_PACKED(datum);
			const char *str = VARDATA_ANY(p);
			size_t len = VARSIZE_ANY_EXHDR(p);

			mrb_value json_class = mrb_obj_value(JSON_CLASS);
			mrb_value json_str = to_mrb_string(mrb, str, len);

			mrb_value result = mrb_funcall(mrb, json_class, "parse", 1, json_str);

			if (p != DatumGetPointer(datum))
				pfree(p); /* free if detoasted */

			if (mrb->exc)
				ereport_exception(mrb);

			return result;
		}
#endif
		default:
			return datum_to_mrb_string(mrb, datum, type);
	}
}


static int64
timestamptz_to_epoch_us(TimestampTz tm)
{
	int64 epoch;

	// TODO: check if TIMESTAMP_NOBEGIN or NOEND
#ifdef HAVE_INT64_TIMESTAMP
	epoch = (int64) tm;
#else
	epoch = (int64) (tm * 1e6)
#endif

	return epoch + JDATE_OFFSET * USECS_PER_DAY;
}

static int64
date_to_epoch_us(DateADT date)
{
	// TODO: check if DATE_NOBEGIN or NOEND
	return (date + JDATE_OFFSET) * USECS_PER_DAY;
}

static mrb_value
epoch_us_to_mrb_time(mrb_state *mrb, int64 epoch)
{
	mrb_value sec = mrb_float_value(mrb, epoch / USECS_PER_SEC);
	mrb_value usec = mrb_float_value(mrb, epoch % USECS_PER_SEC);
	return mrb_funcall(mrb, mrb_obj_value(TIME_CLASS), "at", 2, sec, usec);
}

static int64
mrb_time_to_epoch_us(mrb_state *mrb, mrb_value time)
{
	int64 epoch = 0;
	mrb_value mrb_sec = mrb_funcall(mrb, time, "to_i", 0);
	if (mrb_float_p(mrb_sec))
		epoch += mrb_float(mrb_sec) * USECS_PER_SEC;
	else if (mrb_fixnum_p(mrb_sec))
		epoch += mrb_fixnum(mrb_sec) * USECS_PER_SEC;

	mrb_value mrb_usec = mrb_funcall(mrb, time, "usec", 0);
	if (mrb_float_p(mrb_usec))
		epoch += mrb_float(mrb_usec);
	else if (mrb_fixnum_p(mrb_usec))
		epoch += mrb_fixnum(mrb_usec);

	return epoch;
}

static Datum
epoch_us_to_timestamptz(int64 epoch)
{
	epoch -= JDATE_OFFSET * USECS_PER_DAY;

#ifdef HAVE_INT64_TIMESTAMP
	return Int64GetDatum(epoch);
#else
	return Float8GetDatum(epoch / 1.0e6);
#endif
}

static Datum
epoch_us_to_date(int64 epoch)
{
	epoch -= JDATE_OFFSET * USECS_PER_DAY;
	epoch = epoch / USECS_PER_DAY;
	PG_RETURN_DATEADT((DateADT) epoch);
}

static mrb_value
datum_to_mrb_string(mrb_state *mrb, Datum value, plmruby_type *type)
{
	char *str;

	if (type->fn_output.fn_addr == NULL)
	{
		Oid output_func;
		bool isvarlen;

		getTypeOutputInfo(type->typid, &output_func, &isvarlen);
		fmgr_info_cxt(output_func, &type->fn_output, type->fn_output.fn_mcxt);
	}
	str = OutputFunctionCall(&type->fn_output, value);

	mrb_value result = to_mrb_string(mrb, str, strlen(str));

	pfree(str);

	return result;
}

static mrb_value
to_mrb_string(mrb_state *mrb, const char *str, size_t len)
{
	return to_mrb_string_encoding(mrb, str, len, GetDatabaseEncoding());
}

static mrb_value
to_mrb_string_encoding(mrb_state *mrb, const char *str, size_t len, int encoding)
{
	if (encoding == PG_UTF8)
	{
		return mrb_str_new(mrb, str, len);
	}
	else
	{
		char *utf8;

		utf8 = (char *) pg_do_encoding_conversion(
				(unsigned char *) str, (int) len, encoding, PG_UTF8);

		if (utf8 != str)
			len = strlen(utf8);

		mrb_value result = mrb_str_new(mrb, utf8, len);

		if (utf8 != str)
			pfree(utf8);

		return result;
	}
}

static Datum
mrb_value_to_array_datum(mrb_state *mrb, mrb_value value, bool *isnull, plmruby_type *type)
{
	int length;
	Datum *values;
	bool *nulls;
	int ndims[1];
	int lbs[] = {1};
	ArrayType *result;

	if (mrb_nil_p(value) || mrb_undef_p(value))
	{
		*isnull = true;
		return (Datum) 0;
	}

	if (!mrb_array_p(value))
		elog(ERROR, "value is not an Array");

	length = (int) RARRAY_LEN(value);
	values = (Datum *) palloc(sizeof(Datum) * length);
	nulls = (bool *) palloc(sizeof(bool) * length);
	ndims[0] = length;
	for (int i = 0; i < length; i++)
		values[i] = mrb_value_to_scalar_datum(mrb, mrb_ary_ref(mrb, value, i), &nulls[i], type);

	result = construct_md_array(values, nulls, 1, ndims, lbs,
								type->typid, type->len, type->byval, type->align);
	pfree(values);
	pfree(nulls);

	*isnull = false;
	return PointerGetDatum(result);
}

static Datum
mrb_value_to_scalar_datum(mrb_state *mrb, mrb_value value, bool *isnull, plmruby_type *type)
{
	if (type->category == TYPCATEGORY_COMPOSITE)
		return mrb_value_to_record_datum(mrb, value, isnull, type);

	if (mrb_nil_p(value) || mrb_undef_p(value))
	{
		*isnull = true;
		return (Datum) 0;
	}

	*isnull = false;
	switch (type->typid)
	{
		case OIDOID:
			if (mrb_fixnum_p(value))
				return ObjectIdGetDatum(mrb_fixnum(value));
			break;
		case BOOLOID:
			if (mrb_type(value) == MRB_TT_TRUE || mrb_type(value) == MRB_TT_FALSE)
				return BoolGetDatum(mrb_bool(value));
			break;
		case INT2OID:
			if (mrb_fixnum_p(value))
#ifdef CHECK_INTEGER_OVERFLOW
				return DirectFunctionCall1(int82, Int64GetDatum(mrb_fixnum(value)));
#else
				return Int16GetDatum((int16) mrb_fixnum_p(value));
#endif
			break;
		case INT4OID:
			if (mrb_fixnum_p(value))
#ifdef CHECK_INTEGER_OVERFLOW
				return DirectFunctionCall1(int84, Int64GetDatum(mrb_fixnum(value)));
#else
				return Int32GetDatum((int32) mrb_fixnum(value));
#endif
			break;
		case INT8OID:
			if (mrb_fixnum_p(value))
				return Int64GetDatum((int64) mrb_fixnum(value));
			break;
		case FLOAT4OID:
			if (mrb_float_p(value))
				return Float4GetDatum((float4) mrb_float(value));
			break;
		case FLOAT8OID:
			if (mrb_float_p(value))
				return Float8GetDatum((float8) mrb_float(value));
			break;
		case NUMERICOID:
			if (mrb_float_p(value))
				return DirectFunctionCall1(float8_numeric, Float8GetDatum((float8) mrb_float(value)));
			break;
		case DATEOID:
		{
			if (mrb_obj_is_instance_of(mrb, value, TIME_CLASS))
			{
				int64 epoch = mrb_time_to_epoch_us(mrb, value);
				return epoch_us_to_date(epoch);
			}
			break;
		}
		case TIMESTAMPOID:
		case TIMESTAMPTZOID:
		{
			if (mrb_obj_is_instance_of(mrb, value, TIME_CLASS))
			{
				int64 epoch = mrb_time_to_epoch_us(mrb, value);
				return epoch_us_to_timestamptz(epoch);
			}
			break;
		}
		case TEXTOID:
		case VARCHAROID:
		case BPCHAROID:
		{
			if (mrb_string_p(value))
			{
				return mrb_string_to_text_datum(value);
			}
			break;
		}
		/*
		case XMLOID:
		{
			if (mrb_obj_is_kind_of(mrb, value, XML_DOCUMENT_CLASS))
			{
				mrb_value xml_str = mrb_funcall(mrb, value, "print", 0);
				return mrb_string_to_text_datum(xml_str);
			}
			else if (mrb_string_p(value))
			{
				return mrb_string_to_text_datum(value);
			}
			break;
		}
		*/
#if PG_VERSION_NUM >= 90200
		case JSONOID:
			if (mrb_hash_p(value) || mrb_array_p(value))
			{
				mrb_value json_class = mrb_obj_value(mrb_class_get(mrb, "JSON"));
				mrb_value result = mrb_funcall(mrb, json_class, "stringify", 1, value);
				return mrb_string_to_text_datum(result);
			}
			break;
#endif
		default:
			break;
	}

	Datum result;
	char *str = mrb_str_to_cstr_palloc(mrb_funcall(mrb, value, "to_s", 0));

	if (type->fn_input.fn_addr == NULL)
	{
		Oid input_func;

		getTypeInputInfo(type->typid, &input_func, &type->ioparam);
		fmgr_info_cxt(input_func, &type->fn_input, type->fn_input.fn_mcxt);
	}
	result = InputFunctionCall(&type->fn_input, str, type->ioparam, -1);

	return result;
}

static Datum
mrb_value_to_record_datum(mrb_state *mrb, mrb_value value, bool *isnull, plmruby_type *type)
{
	Datum		result;
	TupleDesc	tupdesc;

	if (mrb_nil_p(value) || mrb_undef_p(value))
	{
		*isnull = true;
		return (Datum) 0;
	}

	tupdesc = lookup_rowtype_tupdesc(type->typid, -1);

	tuple_converter *converter = new_tuple_converter(mrb, tupdesc);

	result = HeapTupleGetDatum(mrb_value_to_heap_tuple(converter, value, NULL, false));

	ReleaseTupleDesc(tupdesc);
	delete_tuple_converter(converter);

	*isnull = false;
	return result;
}

static char *
mrb_str_to_cstr_palloc(mrb_value mrb_str)
{
	size_t len = (size_t) RSTRING_LEN(mrb_str);
	char *str = palloc(len + 1);
	strlcpy(str, RSTRING_PTR(mrb_str), len + 1);
	return str;
}

static Datum
mrb_string_to_text_datum(mrb_value mrb_str)
{
	return CStringGetTextDatum(mrb_str_to_cstr_palloc(mrb_str));
}
