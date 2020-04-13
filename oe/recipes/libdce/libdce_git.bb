DEPENDS = "libdce-firmware libdrm linux-omap4"

LICENSE = "BSD"

inherit autotools lib_package

PV = "1.0"
PR = "r0"
PR_append = "+gitr-${SRCREV}"

SRCREV = "36533bfb6c18e3536c84511a1e4c5a7cae1bb5bf"
SRC_URI = "git://github.com/mobiaqua/libdce.git;protocol=git"

S = "${WORKDIR}/git"
