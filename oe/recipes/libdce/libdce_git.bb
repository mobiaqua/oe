DEPENDS = "libdce-firmware libdrm linux-omap4"

LICENSE = "BSD"

inherit autotools lib_package

PV = "1.0"
PR = "r0"
PR_append = "+gitr-${SRCREV}"

SRCREV = "3f310293e3d115796df028509329e7068f2cd34c"
SRC_URI = "git://github.com/mobiaqua/libdce.git;protocol=git"

S = "${WORKDIR}/git"
