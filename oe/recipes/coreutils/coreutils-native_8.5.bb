require coreutils-${PV}.inc
require coreutils-native.inc

PR = "${INC_PR}.1"

SRC_URI += "file://fix-osx-stpncpy.patch"
