CREATE FUNCTION plmruby_raise() RETURNS int AS
$$
	raise 'foo'
$$
LANGUAGE plmruby IMMUTABLE STRICT;
SELECT plmruby_raise();

CREATE FUNCTION plmruby_no_method_error() RETURNS int AS
$$
	'foo'.bar
$$
LANGUAGE plmruby IMMUTABLE STRICT;
SELECT plmruby_no_method_error();

DO $$ raise 'foo' $$ LANGUAGE plmruby;
DO $$ 'foo'.bar $$ LANGUAGE plmruby;
