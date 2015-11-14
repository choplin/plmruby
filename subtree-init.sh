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

    git subtree add --prefix deps/$name --squash -m "subtree add $url $branch" $url $branch
done
