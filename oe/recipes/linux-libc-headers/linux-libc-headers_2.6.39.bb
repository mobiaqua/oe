require linux-libc-headers.inc

INHIBIT_DEFAULT_DEPS = "1"
DEPENDS += "unifdef-native"
PR = "r0"

COMPATIBLE_TARGET_SYS = "."
SRC_URI = "${KERNELORG_MIRROR}/pub/linux/kernel/v2.6/linux-${PV}.tar.bz2;name=kernel \
          "

SRC_URI[kernel.md5sum] = "1aab7a741abe08d42e8eccf20de61e05"
SRC_URI[kernel.sha256sum] = "584d17f2a3ee18a9501d7ff36907639e538cfdba4529978b8550c461d45c61f6"

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
