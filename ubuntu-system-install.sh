#!/bin/bash

debuild -i -us -uc -b
sudo dpkg -i ../fyfsp_1.0_amd64.deb

restart fyfsp || \
    echo "NOTE: if the job is unknown, it's probable that your terminal doesn't know about your dbus session"
