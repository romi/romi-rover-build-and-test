#!/usr/bin/env bash

adduser romi plugdev

cat <<EOF >> /etc/udev/rules.d/99-dualshock.rules
ATTRS{idVendor}=="054c", ATTRS{idProduct}=="0ba0", SUBSYSTEMS=="usb", ACTION=="add", MODE="0666", GROUP="plugdev"
EOF

udevadm trigger

echo "*** Unplug and replug the controller ***"

