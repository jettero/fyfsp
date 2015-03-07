#!/bin/bash

set -e -x
PS4="$0 exec: "

if [ -f configure ]; then
    ./configure --with-upstart --prefix=/usr

else
    ./autogen.sh --with-upstart --prefix=/usr
fi

make -j 10

if [ ! "$(id -u)" = 0 ]; then
    sudo make install

else
    make install
fi

restart fyfsp
