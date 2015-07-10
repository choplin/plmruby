/*
** mrb_uname.c - Uname class
**
** Copyright (c) MATSUMOTO Ryosuke 2014
**
** See Copyright Notice in LICENSE
*/

#include "mruby.h"
#include "mruby/variable.h"
#include "mruby/class.h"
#include "mruby/data.h"
#include "mrb_uname.h"
#include <sys/utsname.h>

#define DONE mrb_gc_arena_restore(mrb, 0);

typedef struct {
  struct utsname uname_b;
} mrb_uname_data;

static const struct mrb_data_type mrb_uname_data_type = {
  "mrb_uname_data", mrb_free,
};

static mrb_value mrb_uname_wrap(mrb_state *mrb, struct RClass *uname,
    mrb_uname_data *data)
{
    return mrb_obj_value(Data_Wrap_Struct(mrb, uname, &mrb_uname_data_type,
          data));
}

static mrb_uname_data *mrb_uname_get_uname_data(mrb_state *mrb, mrb_value self)
{
  struct utsname uname_b;
  mrb_uname_data *data = NULL;
  mrb_sym uname_sym = mrb_intern_lit(mrb, "@@uname");

  if (mrb_cv_defined(mrb, self, uname_sym)) {
    Data_Get_Struct(mrb, mrb_cv_get(mrb, self, uname_sym),
          &mrb_uname_data_type, data);
  } else {
    data = (mrb_uname_data *)mrb_malloc(mrb, sizeof(mrb_uname_data));
    if (uname(&uname_b) != 0) {
      mrb_raise(mrb, E_RUNTIME_ERROR, "uname failed");
    }
    data->uname_b = uname_b;
    mrb_cv_set(mrb, self, uname_sym,
        mrb_uname_wrap(mrb, mrb_class_ptr(self), data));
  }
  return data;
}

#define MRB_UNAME_GET_VALUE(mrb, self, name) \
  mrb_str_new_cstr(mrb, (mrb_uname_get_uname_data(mrb, self))->uname_b.name)

static mrb_value mrb_uname_sysname(mrb_state *mrb, mrb_value self)
{
  return MRB_UNAME_GET_VALUE(mrb, self, sysname);
}

static mrb_value mrb_uname_machine(mrb_state *mrb, mrb_value self)
{
  return MRB_UNAME_GET_VALUE(mrb, self, machine);
}

static mrb_value mrb_uname_nodename(mrb_state *mrb, mrb_value self)
{
  return MRB_UNAME_GET_VALUE(mrb, self, nodename);
}

static mrb_value mrb_uname_release(mrb_state *mrb, mrb_value self)
{
  return MRB_UNAME_GET_VALUE(mrb, self, release);
}

static mrb_value mrb_uname_version(mrb_state *mrb, mrb_value self)
{
  return MRB_UNAME_GET_VALUE(mrb, self, version);
}

void mrb_mruby_uname_gem_init(mrb_state *mrb)
{
    struct RClass *uname;

    uname = mrb_define_class(mrb, "Uname", mrb->object_class);
    mrb_define_class_method(mrb, uname, "sysname", mrb_uname_sysname, MRB_ARGS_NONE());
    mrb_define_class_method(mrb, uname, "machine", mrb_uname_machine, MRB_ARGS_NONE());
    mrb_define_class_method(mrb, uname, "nodename", mrb_uname_nodename, MRB_ARGS_NONE());
    mrb_define_class_method(mrb, uname, "release", mrb_uname_release, MRB_ARGS_NONE());
    mrb_define_class_method(mrb, uname, "version", mrb_uname_version, MRB_ARGS_NONE());
    DONE;
}

void mrb_mruby_uname_gem_final(mrb_state *mrb)
{
}

