#!/bin/bash

git subtree add --prefix deps/mruby              --squash -m "subtree add https://github.com/mruby/mruby              1.1.0"  https://github.com/mruby/mruby              1.1.0
git subtree add --prefix deps/mruby-digest       --squash -m "subtree add https://github.com/iij/mruby-digest         master" https://github.com/iij/mruby-digest         master
git subtree add --prefix deps/mruby-dir          --squash -m "subtree add https://github.com/iij/mruby-dir            master" https://github.com/iij/mruby-dir            master
git subtree add --prefix deps/mruby-env          --squash -m "subtree add https://github.com/iij/mruby-env            master" https://github.com/iij/mruby-env            master
git subtree add --prefix deps/mruby-io           --squash -m "subtree add https://github.com/iij/mruby-io             master" https://github.com/iij/mruby-io             master
git subtree add --prefix deps/mruby-json         --squash -m "subtree add https://github.com/mattn/mruby-json         master" https://github.com/mattn/mruby-json         master
git subtree add --prefix deps/mruby-onig-regexp  --squash -m "subtree add https://github.com/mattn/mruby-onig-regexp  master" https://github.com/mattn/mruby-onig-regexp  master
git subtree add --prefix deps/mruby-pack         --squash -m "subtree add https://github.com/iij/mruby-pack           master" https://github.com/iij/mruby-pack           master
git subtree add --prefix deps/mruby-process      --squash -m "subtree add https://github.com/iij/mruby-process        master" https://github.com/iij/mruby-process        master
git subtree add --prefix deps/mruby-socket       --squash -m "subtree add https://github.com/iij/mruby-socket         master" https://github.com/iij/mruby-socket         master
git subtree add --prefix deps/mruby-uname        --squash -m "subtree add https://github.com/matsumoto-r/mruby-uname  master" https://github.com/matsumoto-r/mruby-uname  master
git subtree add --prefix deps/mruby-tinyxml2     --squash -m "subtree add https://github.com/h2so5/mruby-tinyxml2     master" https://github.com/h2so5/mruby-tinyxml2     master












