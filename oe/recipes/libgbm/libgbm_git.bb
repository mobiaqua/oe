
inherit autotools lib_package

PV = "1.0.0"
PR = "r0"

DEPENDS = "udev libdrm"

SRCREV = "7c469a6d7a92ee702c5852d35564b3942878b5b2"
SRC_URI = "git://github.com/mobiaqua/libgbm.git;protocol=git \
"

S = "${WORKDIR}/git"
