-- CREATE FUNCTION
CREATE FUNCTION plmruby_test(keys text[], vals text[]) RETURNS text AS
$$
	h = Hash.new
	keys.zip(vals).each do |k,v|
		h[k] = v
	end
	JSON.stringify(h);
$$
LANGUAGE plmruby IMMUTABLE STRICT;
SELECT plmruby_test(ARRAY['name', 'age'], ARRAY['Tom', '29']);

CREATE FUNCTION unnamed_args(text[], text[]) RETURNS text[] AS
$$
	_1 + _2
$$
LANGUAGE plmruby IMMUTABLE STRICT;
SELECT unnamed_args(ARRAY['A', 'B'], ARRAY['C', 'D']);

CREATE FUNCTION concat_strings(VARIADIC args text[]) RETURNS text AS
$$
	args.join
$$
LANGUAGE plmruby IMMUTABLE STRICT;
SELECT concat_strings('A', 'B', NULL, 'C');

CREATE FUNCTION return_void() RETURNS void AS $$ $$ LANGUAGE plmruby;
SELECT return_void();

CREATE FUNCTION return_nil() RETURNS text AS $$ nil $$ LANGUAGE plmruby;
SELECT r, r IS NULL AS isnull FROM return_nil() AS r;
