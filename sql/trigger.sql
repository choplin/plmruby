SET client_min_messages = INFO;

/* Trigget Arguments */
CREATE TABLE test_tbl (i int4, s text);
INSERT INTO test_tbl VALUES(1, 'abc');

CREATE VIEW test_view AS SELECT i FROM test_tbl;

CREATE FUNCTION test_trigger_arguments() RETURNS trigger AS
$$
	elog(NOTICE, "new = ", JSON.stringify(new))
	elog(NOTICE, "old = ", JSON.stringify(old))
	elog(NOTICE, "tg_name = ", tg_name)
	elog(NOTICE, "tg_when = ", tg_when)
	elog(NOTICE, "tg_level = ", tg_level)
	elog(NOTICE, "tg_op = ", tg_op)
	#elog(NOTICE, "tg_relid = ", tg_relid)
	elog(NOTICE, "tg_table_name = ", tg_table_name)
	elog(NOTICE, "tg_table_schema = ", tg_table_schema)
	elog(NOTICE, "tg_argv = ", tg_argv)
$$
LANGUAGE plmruby;

-- BEFORE
CREATE TRIGGER test_trigger_arguments_before
  BEFORE INSERT OR UPDATE OR DELETE
  ON test_tbl FOR EACH ROW
  EXECUTE PROCEDURE test_trigger_arguments('foo', 'bar');
INSERT INTO test_tbl VALUES(100, 'ABC');
UPDATE test_tbl SET s = 'abc' where i = 100;
DELETE FROM test_tbl where i = 100;
DROP TRIGGER test_trigger_arguments_before ON test_tbl;

-- AFTER
CREATE TRIGGER test_trigger_arguments_after
  AFTER INSERT
  ON test_tbl FOR EACH ROW
  EXECUTE PROCEDURE test_trigger_arguments();
INSERT INTO test_tbl VALUES(1000, 'foo');
DROP TRIGGER test_trigger_arguments_after ON test_tbl;

-- INSTEAD OF
CREATE TRIGGER test_trigger_arguments_instead_of
  INSTEAD OF INSERT
  ON test_view FOR EACH ROW
  EXECUTE PROCEDURE test_trigger_arguments();
INSERT INTO test_view VALUES(10000);
DROP TRIGGER test_trigger_arguments_instead_of ON test_view;

-- STATEMENT
CREATE TRIGGER test_trigger_arguments_statement
  BEFORE TRUNCATE
  ON test_tbl FOR STATEMENT
  EXECUTE PROCEDURE test_trigger_arguments();
TRUNCATE test_tbl;
DROP TRIGGER test_trigger_arguments_statement ON test_tbl;

DROP VIEW test_tbl;
DROP TABLE test_view;

/* Returned Value */
CREATE TABLE test_tbl2 (subject text, val int);
CREATE VIEW test_view2 AS SELECT * FROM test_tbl2;

-- One more trigger
CREATE FUNCTION test_trigger_return_value() RETURNS trigger AS
$$
	tuple = case (tg_op)
			when :insert
				new
			when :update
				new
			when :delete
				old
			end

	case tuple[:subject]
	when "nil"  then nil
	when "ok"   then :ok
	when "skip" then :skip
	when "invalid" then :invalid_symbol
	else
		tuple[:val] += 1
		tuple
	end
$$
LANGUAGE plmruby;

CREATE TRIGGER test_trigger_return_value_insert
  BEFORE INSERT
  ON test_tbl2 FOR EACH ROW
  EXECUTE PROCEDURE test_trigger_return_value();
-- INSERT
INSERT INTO test_tbl2 VALUES('nil', 0), ('ok', 0), ('skip', 2), ('tuple like', 0);
-- insert should fail when an invalid symbol is returned
INSERT INTO test_tbl2 VALUES('invalid', 0);
SELECT * FROM test_tbl2;
DROP TRIGGER test_trigger_return_value_insert ON test_tbl2;

CREATE TRIGGER test_trigger_return_value_update_delete
  BEFORE UPDATE OR DELETE
  ON test_tbl2 FOR EACH ROW
  EXECUTE PROCEDURE test_trigger_return_value();
INSERT INTO test_tbl2 VALUES ('skip', 0);
-- UPDATE
UPDATE test_tbl2 SET val = val + 1;
SELECT * FROM test_tbl2;
-- DELTE
DELETE FROM test_tbl2;
SELECT * FROM test_tbl2;
DROP TRIGGER test_trigger_return_value_update_delete ON test_tbl2;

-- INSTEAD OF
-- TODO

/* ALTER TABLE */
CREATE OR REPLACE FUNCTION plmruby_trigger_handler() RETURNS trigger AS
$$
	new[:sum] = new[:data].values.inject{|sum,i| sum += i; sum} + new[:num]
	new
$$ LANGUAGE plmruby;

CREATE TABLE plmrubytest (id serial primary key, data json, sum integer, num integer);

CREATE TRIGGER plmrubytest_trigger
	BEFORE INSERT OR UPDATE
	ON plmrubytest FOR EACH ROW
	EXECUTE PROCEDURE plmruby_trigger_handler();
	
INSERT INTO plmrubytest (data, sum, num) values ('{"a": 1, "b": 2}', 0, 0);
INSERT INTO plmrubytest (data, sum, num) values ('{"a": 3, "b": 4}', 0, 0);
SELECT * FROM plmrubytest;

-- test OK
UPDATE plmrubytest SET num = 2 where id =2;
-- then add two fields and drop one of them
ALTER TABLE plmrubytest ADD COLUMN repro1 varchar;
ALTER TABLE plmrubytest ADD COLUMN repro2 varchar;
ALTER TABLE plmrubytest DROP COLUMN repro1;
-- dropped columns should work with trigger
UPDATE plmrubytest SET repro2='test';
SELECT * FROM plmrubytest;
DROP TABLE plmrubytest;