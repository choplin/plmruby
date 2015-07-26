CREATE TYPE rec AS (i integer, t text);
CREATE FUNCTION set_of_records_array() RETURNS SETOF rec AS
$$
	[
		{i: 1, t: "a"},
		{i: 2, t: "b"},
		{i: 3, t: "c"},
	]
$$
LANGUAGE plmruby;
SELECT * FROM set_of_records_array();

CREATE FUNCTION set_of_records_enumerable() RETURNS SETOF rec AS
$$
	cls = Class.new do
		ARY = [
			{i: 1, t: "a"},
			{i: 2, t: "b"},
			{i: 3, t: "c"},
		]
		def each
			ARY.each {|e| yield e}
		end
	end
	cls.new
$$
LANGUAGE plmruby;
SELECT * FROM set_of_records_enumerable();

CREATE FUNCTION set_of_records_enumerator() RETURNS SETOF rec AS
$$
	Enumerator.new do |y|
		ARY = [
			{i: 1, t: "a"},
			{i: 2, t: "b"},
			{i: 3, t: "c"},
		]
		ARY.each {|e| y << e}
	end
$$
LANGUAGE plmruby;
SELECT * FROM set_of_records_enumerator();

CREATE FUNCTION set_of_scalars() RETURNS SETOF integer AS
$$
	[1,2,3]
$$
LANGUAGE plmruby;
SELECT * FROM set_of_scalars();

CREATE FUNCTION set_of_unnamed_records() RETURNS SETOF record AS
$$
	[ {i: true} ]
$$
LANGUAGE plmruby;
SELECT set_of_unnamed_records();
SELECT * FROM set_of_unnamed_records() t (i bool);

CREATE FUNCTION set_of_unnamed_records2() RETURNS SETOF record AS
$$
	[{a: 1, b: 2}]
$$ LANGUAGE plmruby;

-- not enough fields specified
SELECT * FROM set_of_unnamed_records2() AS x(a int);
-- field names mismatch
SELECT * FROM set_of_unnamed_records2() AS x(a int, c int);
-- name counts and values match
SELECT * FROM set_of_unnamed_records2() AS x(a int, b int);

DROP TYPE rec CASCADE;
