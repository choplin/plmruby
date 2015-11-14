#!/bin/bash

set -e
set -u

source $(dirname $0)/subtree-deps.sh

IFS=$'\n'
for d in ${deps[@]}; do
    IFS=' '
    set -- $d
    name=$1
    url=$2
    branch=$3

    git subtree pull --prefix deps/$name --squash -m "subtree pull $url $branch" $url $branch
done
