MRuby::Build.new do |conf|
  # load specific toolchain settings

  # Gets set by the VS command prompts.
  if ENV['VisualStudioVersion'] || ENV['VSINSTALLDIR']
    toolchain :visualcpp
  else
    toolchain :gcc
  end

  conf.gem :core => 'mruby-array-ext'
  conf.gem :core => 'mruby-compiler'
  conf.gem :core => 'mruby-enum-ext'
  conf.gem :core => 'mruby-enum-lazy'
  conf.gem :core => 'mruby-enumerator'
  conf.gem :core => 'mruby-error'
  conf.gem :core => 'mruby-eval'
  conf.gem :core => 'mruby-fiber'
  conf.gem :core => 'mruby-hash-ext'
  conf.gem :core => 'mruby-kernel-ext'
  conf.gem :core => 'mruby-math'
  conf.gem :core => 'mruby-numeric-ext'
  conf.gem :core => 'mruby-object-ext'
  conf.gem :core => 'mruby-objectspace'
  conf.gem :core => 'mruby-proc-ext'
  conf.gem :core => 'mruby-random'
  conf.gem :core => 'mruby-range-ext'
  conf.gem :core => 'mruby-sprintf'
  conf.gem :core => 'mruby-string-ext'
  conf.gem :core => 'mruby-struct'
  conf.gem :core => 'mruby-symbol-ext'
  conf.gem :core => 'mruby-time'
  conf.gem :core => 'mruby-toplevel-ext'

  conf.gem 'deps/mruby-io'
  conf.gem 'deps/mruby-env'
  conf.gem 'deps/mruby-dir'
  conf.gem 'deps/mruby-digest'
  conf.gem 'deps/mruby-process'
  conf.gem 'deps/mruby-pack'
  conf.gem 'deps/mruby-json'
  conf.gem 'deps/mruby-onig-regexp'
  conf.gem 'deps/mruby-uname'

  conf.gem 'mrbgems/plmruby'

  conf.cc.include_paths += ENV['MRUBY_PG_INCLUDE_DIR'].split("\s")
  conf.cc.defines = ENV['MRUBY_DEFINES'].split("\s")
end
