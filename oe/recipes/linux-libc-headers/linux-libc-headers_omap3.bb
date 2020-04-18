require linux-libc-headers.inc

INHIBIT_DEFAULT_DEPS = "1"
DEPENDS += "unifdef-native"
PR = "r0"

COMPATIBLE_MACHINE = "igep0030"

COMPATIBLE_TARGET_SYS = "."

FILESPATHPKG =. "linux-omap3:"

SRCREV = "df5d3177e220f3a06e42837be913ca7529c897f8"

SRC_URI = "git://git.isee.biz/pub/scm/linux-omap-2.6.git;protocol=git;branch=linux-2.6.37.y \
           file://patch_2.6.37.6.patch \
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
