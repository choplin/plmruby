SET TIMEZONE TO UTC;

/*
 * bool
 */
CREATE FUNCTION plmruby_bool_in(v bool) RETURNS void AS $$
	elog(INFO, v)
$$ LANGUAGE plmruby IMMUTABLE STRICT;
SELECT plmruby_bool_in(true);

CREATE FUNCTION plmruby_bool_out() RETURNS bool AS $$
	true
$$ LANGUAGE plmruby IMMUTABLE STRICT;
SELECT plmruby_bool_out();

/*
 * int2
 */
CREATE FUNCTION plmruby_int2_in(v int2) RETURNS void AS $$
	elog(INFO, v)
$$ LANGUAGE plmruby IMMUTABLE STRICT;
SELECT plmruby_int2_in(0::int2);

CREATE FUNCTION plmruby_int2_out() RETURNS int2 AS $$
	0
$$ LANGUAGE plmruby IMMUTABLE STRICT;
SELECT plmruby_int2_out();

/*
 * int4
 */
CREATE FUNCTION plmruby_int4_in(v int4) RETURNS void AS $$
	elog(INFO, v)
$$ LANGUAGE plmruby IMMUTABLE STRICT;
SELECT plmruby_int4_in(0::int4);

CREATE FUNCTION plmruby_int4_out() RETURNS int4 AS $$
	0
$$ LANGUAGE plmruby IMMUTABLE STRICT;
SELECT plmruby_int4_out();

/*
 * int8
 */
CREATE FUNCTION plmruby_int8_in(v int8) RETURNS void AS $$
	elog(INFO, v)
$$ LANGUAGE plmruby IMMUTABLE STRICT;
SELECT plmruby_int8_in(0::int8);

CREATE FUNCTION plmruby_int8_out() RETURNS int8 AS $$
	0
$$ LANGUAGE plmruby IMMUTABLE STRICT;
SELECT plmruby_int8_out();

/*
 * float4
 */
CREATE FUNCTION plmruby_float4_in(v float4) RETURNS void AS $$
	elog(INFO, v)
$$ LANGUAGE plmruby IMMUTABLE STRICT;
SELECT plmruby_float4_in(1.1::float4);

CREATE FUNCTION plmruby_float4_out() RETURNS float4 AS $$
	1.1
$$ LANGUAGE plmruby IMMUTABLE STRICT;
SELECT plmruby_float4_out();

/*
 * float8
 */
CREATE FUNCTION plmruby_float8_in(v float8) RETURNS void AS $$
	elog(INFO, v)
$$ LANGUAGE plmruby IMMUTABLE STRICT;
SELECT plmruby_float8_in(1.1::float8);

CREATE FUNCTION plmruby_float8_out() RETURNS float8 AS $$
	1.1
$$ LANGUAGE plmruby IMMUTABLE STRICT;
SELECT plmruby_float8_out();

/*
 * numeric
 */
CREATE FUNCTION plmruby_numeric_in(v numeric) RETURNS void AS $$
	elog(INFO, v)
$$ LANGUAGE plmruby IMMUTABLE STRICT;
SELECT plmruby_numeric_in(1.1::numeric);

CREATE FUNCTION plmruby_numeric_out() RETURNS numeric AS $$
	1.1
$$ LANGUAGE plmruby IMMUTABLE STRICT;
SELECT plmruby_numeric_out();

/*
 * date
 */
CREATE FUNCTION plmruby_date_in(v date) RETURNS void AS $$
	elog(INFO, v.utc)
$$ LANGUAGE plmruby IMMUTABLE STRICT;
SELECT plmruby_date_in(date '2015-11-15');

CREATE FUNCTION plmruby_date_out() RETURNS date AS $$
	Time.utc(2015,11,15)
$$ LANGUAGE plmruby IMMUTABLE STRICT;
SELECT plmruby_date_out();

/*
 * timestamp
 */
CREATE FUNCTION plmruby_timestamp_in(v timestamp) RETURNS void AS $$
	elog(INFO, v.utc)
$$ LANGUAGE plmruby IMMUTABLE STRICT;
SELECT plmruby_timestamp_in(timestamp '2015-11-15 1:20:33');

CREATE FUNCTION plmruby_timestamp_out() RETURNS timestamp AS $$
	Time.utc(2015,11,15,1,20,33)
$$ LANGUAGE plmruby IMMUTABLE STRICT;
SELECT plmruby_timestamp_out();

/*
 * timestamptz
 */
CREATE FUNCTION plmruby_timestamptz_in(v timestamptz) RETURNS void AS $$
	elog(INFO, v.utc)
$$ LANGUAGE plmruby IMMUTABLE STRICT;
SELECT plmruby_timestamptz_in(timestamptz '2015-11-15 1:20:33 Asia/Tokyo');

CREATE FUNCTION plmruby_timestamptz_out() RETURNS timestamptz AS $$
	ENV['TZ'] = 'Asia/Tokyo'
	Time.new(2015,11,15,1,20,33)
$$ LANGUAGE plmruby IMMUTABLE STRICT;
SELECT plmruby_timestamptz_out();

/*
 * text
 */
CREATE FUNCTION plmruby_text_in(v text) RETURNS void AS $$
	elog(INFO, v)
$$ LANGUAGE plmruby IMMUTABLE STRICT;
SELECT plmruby_text_in('foo'::text);

CREATE FUNCTION plmruby_text_out() RETURNS text AS $$
	'foo'
$$ LANGUAGE plmruby IMMUTABLE STRICT;
SELECT plmruby_text_out();

/*
 * varchar
 */
CREATE FUNCTION plmruby_varchar_in(v varchar(10)) RETURNS void AS $$
	elog(INFO, v)
$$ LANGUAGE plmruby IMMUTABLE STRICT;
SELECT plmruby_varchar_in('foo'::varchar(10));

CREATE FUNCTION plmruby_varchar_out() RETURNS varchar(10) AS $$
	'foo'
$$ LANGUAGE plmruby IMMUTABLE STRICT;
SELECT plmruby_varchar_out();

/*
 * char
 */
CREATE FUNCTION plmruby_char_in(v char(10)) RETURNS void AS $$
	elog(INFO, v)
$$ LANGUAGE plmruby IMMUTABLE STRICT;
SELECT plmruby_char_in('foo'::char(10));

CREATE FUNCTION plmruby_char_out() RETURNS char(10) AS $$
	'foo'
$$ LANGUAGE plmruby IMMUTABLE STRICT;
SELECT plmruby_char_out();

/*
 * JSON
 */
CREATE FUNCTION plmruby_json_in(v json) RETURNS void AS $$
	elog(INFO, v)
$$ LANGUAGE plmruby IMMUTABLE STRICT;
SELECT plmruby_json_in('{"a":1, "b":"c"}'::json);
SELECT plmruby_json_in('[1,2,3]'::json);

CREATE FUNCTION plmruby_json_out() RETURNS json AS $$
	JSON.parse('{"a":1, "b":"c"}')
$$ LANGUAGE plmruby IMMUTABLE STRICT;
SELECT plmruby_json_out();

CREATE FUNCTION plmruby_json_out_array() RETURNS json AS $$
	JSON.parse('[1,2,3]')
$$ LANGUAGE plmruby IMMUTABLE STRICT;
SELECT plmruby_json_out_array();

/*
 * Array
 */
CREATE FUNCTION plmruby_array_in(v int4[]) RETURNS void AS $$
	elog(INFO, v)
$$ LANGUAGE plmruby IMMUTABLE STRICT;
SELECT plmruby_array_in(ARRAY[1,2,3]::int4[]);

CREATE FUNCTION plmruby_anyarray_in(v anyarray) RETURNS void AS $$
	elog(INFO, v)
$$ LANGUAGE plmruby IMMUTABLE STRICT;
SELECT plmruby_anyarray_in(ARRAY['a','b','c']::text[]);

CREATE FUNCTION plmruby_array_out() RETURNS int4[] AS $$
	[1,2,3]
$$ LANGUAGE plmruby IMMUTABLE STRICT;
SELECT plmruby_array_out();