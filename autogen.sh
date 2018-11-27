#!/bin/bash

function do1() {
    echo -- "$@"
    "$@"
    echo
}

do1 autoconf --version
do1 automake --version

set -e
do1 autoreconf -vi
do1 ./configure "$@"
