#!/bin/bash

set -e
set -u

deps=(
    "mruby              https://github.com/mruby/mruby              1.1.0"
    "mruby-digest       https://github.com/iij/mruby-digest         master"
    "mruby-dir          https://github.com/iij/mruby-dir            master"
    "mruby-env          https://github.com/iij/mruby-env            master"
    "mruby-io           https://github.com/iij/mruby-io             master"
    "mruby-json         https://github.com/mattn/mruby-json         master"
    "mruby-onig-regexp  https://github.com/mattn/mruby-onig-regexp  master"
    "mruby-pack         https://github.com/iij/mruby-pack           master"
    "mruby-process      https://github.com/iij/mruby-process        master"
    "mruby-socket       https://github.com/iij/mruby-socket         master"
    "mruby-uname        https://github.com/matsumoto-r/mruby-uname  master"
    "mruby-tinyxml2     https://github.com/h2so5/mruby-tinyxml2     master"
)
