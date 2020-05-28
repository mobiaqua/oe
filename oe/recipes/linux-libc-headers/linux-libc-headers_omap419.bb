require linux-libc-headers.inc

INHIBIT_DEFAULT_DEPS = "1"
DEPENDS += "unifdef-native"
PR = "r0"

COMPATIBLE_MACHINE = "board-tv"

COMPATIBLE_TARGET_SYS = "."

PV = "4.19.124"

SRC_URI = "${KERNELORG_MIRROR}/linux/kernel/v4.x/linux-${PV}.tar.xz"

SRC_URI[md5sum] = "0b7e8139efeb20c69dc375649aedc7b8"
SRC_URI[sha256sum] = "d5d9001879d7a77309dca203656490326d26b068b7b0b9d8003548dba8fdad00"

S = "${WORKDIR}/linux-${PV}"

do_configure() {
	cd ${S}
	oe_runmake allnoconfig ARCH=$ARCH
}

do_compile () {
}

do_install() {
	oe_runmake headers_install INSTALL_HDR_PATH=${D}${exec_prefix} ARCH=$ARCH
}
