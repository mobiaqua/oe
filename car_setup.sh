#!/bin/sh

[ "x$0" = "x./car_setup.sh" ] && echo "Script must run via sourcing like '. car_setup.sh'" && exit 1

source setup.sh car
