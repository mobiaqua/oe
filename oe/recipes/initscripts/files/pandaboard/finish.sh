#!/bin/sh

if ! test -e /etc/.configured; then
	> /etc/.configured
fi

#MobiAqua: temporary hack
#echo 0 > /sys/devices/platform/omapdss/manager1/alpha_blending_enabled
#echo 0 > /sys/devices/platform/omapdss/manager2/alpha_blending_enabled
#dd if=/dev/zero of=/dev/fb0
