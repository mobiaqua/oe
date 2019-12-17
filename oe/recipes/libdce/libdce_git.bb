DEPENDS = "libdce-firmware libdrm linux-omap4"

LICENSE = "BSD"

inherit autotools lib_package

PV = "1.0"
PR = "r0"
PR_append = "+gitr-${SRCREV}"

SRCREV = "fc9eb1bc846b1679327c507f5753b3d65d242167"
SRC_URI = "git://github.com/mobiaqua/libdce.git;protocol=git"

S = "${WORKDIR}/git"
