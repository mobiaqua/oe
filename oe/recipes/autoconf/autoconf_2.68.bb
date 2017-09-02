require autoconf.inc

PR = "r0"

PARALLEL_MAKE = ""

DEPENDS += "m4-native"
RDEPENDS_${PN} = "m4 gnu-config"
LICENSE = "GPLv2|GPLv3"
LIC_FILES_CHKSUM = "file://COPYING;md5=751419260aa954499f7abaabaa882bbe \
		    file://COPYINGv3;md5=d32239bcb673463ab874e80d47fae504"
SRC_URI += "file://autoreconf-include.patch;apply=yes \
	   file://autoreconf-exclude.patch;apply=yes \
	   file://autoreconf-foreign.patch;apply=yes \
	   file://autoreconf-gnuconfigize.patch;apply=yes \
	   file://autoheader-nonfatal-warnings.patch;apply=yes \
	   ${@['file://path_prog_fixes.patch;apply=yes', ''][bb.data.inherits_class('native', d)]} \
           file://config_site.patch;apply=yes"

SRC_URI[md5sum] = "864d785215aa60d627c91fcb21b05b07"
SRC_URI[sha256sum] = "c491fb273fd6d4ca925e26ceed3d177920233c76d542b150ff35e571454332c8"

DEPENDS_virtclass-native = "m4-native gnu-config-native"
RDEPENDS_${PN}_virtclass-native = "m4-native gnu-config-native"

SRC_URI_append_virtclass-native = " file://fix_path_xtra.patch;apply=yes"

BBCLASSEXTEND = "native"
