DESCRIPTION = "mpg123 is a fast and free console based real time MPEG \
Audio Player for Layer 1, 2 and 3."
LICENSE = "LGPL"
DESCRIPTION = "multimedia"
HOMEPAGE = "http://www.mpg123.de"
RCONFLICTS_${PN} = "mpg321"
RREPLACES_${PN} = "mpg321"
PR = "r0"

SRC_URI = "http://www.mpg123.de/download/mpg123-${PV}.tar.bz2"

EXTRA_OECONF="--with-cpu=generic_nofpu --enable-network=no"
EXTRA_OECONF_arm="--with-cpu=arm_nofpu --with-optimization=4 --enable-network=no"
EXTRA_OECONF_arm6="--with-cpu=arm_nofpu --with-optimization=4 --enable-network=no"
EXTRA_OECONF_armv7a-hf="--with-cpu=neon --with-optimization=4 --enable-network=no"

inherit autotools

SRC_URI[md5sum] = "0a535c0b501c584f43c919dfe6be9144"
SRC_URI[sha256sum] = "b5678bfe0cd1c6d2e29561d50bbef6bbc14ed055404453d18b6f4044c7b0a7ae"

S = "${WORKDIR}/mpg123-${PV}"

#MobiAqua: do not run parallel
PARALLEL_MAKE = ""
