#!/bin/sh

[ "x$0" = "x./tv_setup.sh" ] && echo "Script must run via sourcing like '. tv_setup.sh'" && exit 1

source setup.sh tv $1
