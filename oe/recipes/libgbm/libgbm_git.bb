
inherit autotools lib_package

PV = "1.0.0"
PR = "r0"

DEPENDS = "udev libdrm"

SRCREV = "fbf975f64e1d08587ee30c6e1d8cd9c0690d4721"
SRC_URI = "git://github.com/mobiaqua/libgbm.git;protocol=git \
"

S = "${WORKDIR}/git"
