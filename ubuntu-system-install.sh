#!/bin/bash

function die() {
    echo "$*"
    exit 1
}

(
    debuild -i -us -uc -b \
        || sudo apt-get install build-essential devscripts dh-autoreconf
    debuild -i -us -uc -b
    ) \
    || die "debuild failed. this is not going to work."

sudo dpkg -i ../fyfsp_1.0_amd64.deb \
    || die "can't install the bin for some reason"

restart fyfsp || \
    echo "NOTE: if the job is unknown, it's probable that your terminal doesn't know about your dbus session"
