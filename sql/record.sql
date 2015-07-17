CREATE TYPE rec AS (i integer, t text);
CREATE FUNCTION scalar_to_record(i integer, t text) RETURNS rec AS
$$
	{ i: i, t: t }
$$
LANGUAGE plmruby;
SELECT scalar_to_record(1, 'a');

CREATE FUNCTION record_to_text(x rec) RETURNS text AS
$$
	JSON.stringify(x)
$$
LANGUAGE plmruby;
SELECT record_to_text('(1,a)'::rec);

CREATE FUNCTION return_record(i integer, t text) RETURNS record AS
$$
	{ i: i, t: t }
$$
LANGUAGE plmruby;
SELECT * FROM return_record(1, 'a');
SELECT * FROM return_record(1, 'a') AS t(j integer, s text);
SELECT * FROM return_record(1, 'a') AS t(x text, y text);

