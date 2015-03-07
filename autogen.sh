#!/bin/bash

function do1() {
    echo -- "$@"
    "$@"
    echo
}

set -e
do1 autoreconf -vi
./configure "$@"
