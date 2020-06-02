require linux-libc-headers.inc

INHIBIT_DEFAULT_DEPS = "1"
DEPENDS += "unifdef-native"
PR = "r0"

COMPATIBLE_MACHINE = "board-tv"

COMPATIBLE_TARGET_SYS = "."

FILESPATHPKG =. "linux-omap4:"

SRCREV = "ti-ubuntu-3.4.0-1491.3"

SRC_URI = "git://dev.omapzoom.org/pub/scm/integration/kernel-ubuntu.git;protocol=git;branch=ti-ubuntu-3.4-stable \
           file://patch-3.4.103.patch \
           file://patch-3.4.103-104.patch \
           file://patch-3.4.104-105.patch \
           file://patch-3.4.105-106.patch \
           file://patch-3.4.106-107.patch \
           file://patch-3.4.107-108.patch \
           file://patch-3.4.108-109.patch \
           file://patch-3.4.109-110.patch \
           file://patch-3.4.110-111.patch \
           file://patch-3.4.111-112.patch \
           file://patch-3.4.112-113.patch \
"

S = "${WORKDIR}/git"

do_configure() {
	cd ${S}
	oe_runmake allnoconfig ARCH=$ARCH
}

do_compile () {
}

do_install() {
	oe_runmake headers_install INSTALL_HDR_PATH=${D}${exec_prefix} ARCH=$ARCH
}
