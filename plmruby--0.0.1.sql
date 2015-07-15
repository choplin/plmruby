CREATE FUNCTION plmruby_call_handler() RETURNS language_handler
 AS 'MODULE_PATHNAME' LANGUAGE C;

CREATE TRUSTED LANGUAGE plmruby
	HANDLER plmruby_call_handler;
