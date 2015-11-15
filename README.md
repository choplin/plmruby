A Procedural Language in mruby
==============================

TBD

## Features

* Scalar function calls
* Type conversion between mruby and PostgreSQL built-in types
* Trigger functions
* Set returning function calls

## Quick Start

TBD

## Trigger Functions

You can define a trigger in plmruby. When a trigger function is called, values listed below are passed.

Argument        | Type   | Description
----------------|--------|----------------------------------------------------------------------------------------------------
new             | Hash   | The new database row for INSERT/UPDATE operations in row-level triggers. Otherwise, nil.
old             | Hash   | The old database row for UPDATE/DELETE operations in row-level triggers. Otherwise, nil.
tg_name         | String | A trigger name
tg_when         | Symbol | One of :before, :after, or :instead_of
tg_level        | Symbol | One of :row, or :statement
tg_op           | Symbol | One of :insert, :delete, :update, or :truncate
tg_relid        | Fixnum | OID of the table on which the trigger occurred.
tg_table_name   | String | The name of the table on which the trigger occurred.
tg_table_schema | String | The schema of the table on which the trigger occurred.
tg_argv         | Array  | The arguments from the CREATE TRIGGER statement.

### Return Value From Trigger

Trigger function may return nil, :OK, :SKIP and any tuple like value. A value returned is treated as follows. Tuple like value indicates a value which can be converted into PostgreSQL record type. See [Type Conversion](#Type Conversion) section. Otherwise, an error occurred.

Returned Value     | nil        | :ok        | :skip   | Tuple like value
-------------------|------------|------------|---------|-----------------
:STATEMENT         | ignored    | ignored    | ignored | ignored
:ROW + :AFTER      | ignored    | ignored    | ignored | ignored
:ROW + :BEFORE     | unmodified | unmodified | skipped | modified
:ROW + :INSTEAD_OF | unmodified | unmodified | skipped | modified

**ignored**: The returned value is ignored.

**unmodified**: The row value is `new` if trigger is fired by :UPDATE, otherwise row value is `old`.

**skip**: Skip the rest of the operation for this row, and the row value is unmodified (i.e., subsequent triggers are not fired, and the INSERT/UPDATE/DELETE does not occur for this row).

**modified**: the row value will be replaced with the returned value.

## Type Conversion

### From PostgreSQL to mruby

When mruby function is called, the passed values will be automatically converted from PostgreSQL's type into mruby type as follows. Otherwise, the value will be converted into String via normal stringify procedure of PostgreSQL (pg_type.typoutput).

Argument Type in PostgreSQL | Type in mruby
----------------------------|------------------------------------------------------------------------
oid                         | Fixnum
bool                        | true / false
int2                        | Fixnum
int4                        | Fixnum
int8                        | Fixnum
float4                      | Float
float8                      | Float
numeric                     | Float
date                        | Time
timestamp                   | Time
timestamptz                 | Time
text                        | String
varchar                     | String
char                        | String
json                        | Hash or Array (via JSON.parse. see https://github.com/mattn/mruby-json)
array / anyarray            | Array
record                      | Hash
Otherwise                   | String (via pg_type.typoutput)

### From mruby to PostgreSQL

When mruby returns a value to PostgreSQL, plmruby will attempt to convert the value so that it can adapt the type declared as RETURNS. mruby types which can be converted into a particular PostgreSQL type is listed below. When a value with a type which is not listed, the value will be stringified via .to_s at first, then passed to pg_type.typinput, which is common string parse procedure for all type.

RETURNS type in PostgreSQL | Type in mruby
---------------------------|----------------------------------------------------------------------------
oid                        | Fixnum
bool                       | true / false
int2                       | Fixnum
int4                       | Fixnum
int8                       | Fixnum
float4                     | Float
float8                     | Float
numeric                    | Float
date                       | Time
timestamp                  | Time
timestamptz                | Time
text                       | String
varchar                    | String
char                       | String
json                       | Hash or Array (via JSON.stringify. see https://github.com/mattn/mruby-json)
array                      | Array
record                     | Hash
Otherwise                  | call .to_s, then passed to pg_type.typinput

## Set Returning Functions

PostgreSQL can return TBD