# mruby
MRUBY_DIR := deps/mruby
MRUBY_INCLUDE_DIR := $(MRUBY_DIR)/include
MRUBY_MAK_FILE := $(MRUBY_DIR)/build/host/lib/libmruby.flags.mak

include $(MRUBY_MAK_FILE)

# GEM_DIR := mrbgems/pg_mruby_worker_ext
# GEM_SRC := $(wildcard $(GEM_DIR)/src/*.c) $(wildcard $(GEM_DIR)/src/*.rb)

# extension
MODULE_big := plmruby
OBJS := plmruby.o plmruby_proc.o plmruby_type.o plmruby_env.o plmruby_util.o



EXTENSION := plmruby
EXTVERSION := 0.0.1
DATA := plmruby--$(EXTVERSION).sql

# strip " from library directory options
SHLIB_LINK += $(subst \",,$(MRUBY_LDFLAGS_BEFORE_LIBS)) $(MRUBY_LIBS) $(subst \",,$(MRUBY_LDFLAGS))
PG_CPPFLAGS += -I$(MRUBY_INCLUDE_DIR)

PG_CONFIG := pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

MRUBY_PG_INCLUDE_DIR := "$(shell $(PG_CONFIG) --includedir) $(shell $(PG_CONFIG) --includedir-server)"
MRUBY_ENV := MRUBY_CONFIG=$(CURDIR)/build_config.rb MRUBY_PG_INCLUDE_DIR=$(MRUBY_PG_INCLUDE_DIR)

# rules
$(shlib): $(MRUBY_MAK_FILE)

#$(MRUBY_MAK_FILE): build_config.rb $(MRUBY_DIR) $(GEM_SRC)
$(MRUBY_MAK_FILE): build_config.rb $(MRUBY_DIR)
	env $(MRUBY_ENV) $(MAKE) -C deps/mruby

.PHONY: clean-all
clean-all: clean clean-mruby

.PHONY: clean-mruby
clean-mruby:
	env $(MRUBY_ENV) $(MAKE) -C deps/mruby clean
