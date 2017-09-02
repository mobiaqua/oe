DESCRIPTION = "x-loader for IGEP based platforms"
SECTION = "bootloader"
LICENSE = "GPL"

DEFAULT_PREFERENCE = "-1"
DEFAULT_PREFERENCE_igep0030 = "1"

DEPENDS = "signgp-native"
PROVIDES += "x-loader"
COMPATIBLE_MACHINE = "igep0030"
PARALLEL_MAKE=""
PACKAGE_ARCH = "${MACHINE_ARCH}"

SRCREV = "7a999c47f6bf629cd98c8722620e08c4f43d4fec"

SRC_URI = "git://git.isee.biz/pub/scm/x-loader.git;protocol=git \
          "

S = "${WORKDIR}/git"

XLOAD_MACHINE = "igep0030-sdcard_config"

EXTRA_OEMAKE = "CROSS_COMPILE=${TARGET_PREFIX}"

MLO_IMAGE ?= "MLO-${MACHINE}-${PV}-${PR}"
MLO_SYMLINK ?= "MLO"
MLO_SYMLINK_NOMACHINE ?= "MLO"

do_compile () {
	unset LDFLAGS
	unset CFLAGS
	unset CPPFLAGS
	oe_runmake distclean
	oe_runmake ${XLOAD_MACHINE}
	oe_runmake
}

do_install () {
	signGP ${S}/x-load.bin

	install -d ${D}/boot
	install -m 0644 ${S}/x-load.bin.ift ${D}/boot/${MLO_IMAGE}
	ln -sf ${MLO_IMAGE} ${D}/boot/${MLO_SYMLINK_NOMACHINE}
}

FILES_${PN} = "/boot"

do_deploy () {
	signGP ${S}/x-load.bin
	install -d ${DEPLOY_DIR_IMAGE}
	install -m 0644 ${S}/x-load.bin.ift ${DEPLOY_DIR_IMAGE}/${MLO_IMAGE}
	package_stagefile_shell ${DEPLOY_DIR_IMAGE}/${MLO_IMAGE}

	cd ${DEPLOY_DIR_IMAGE}
	rm -f ${MLO_SYMLINK}
	ln -sf ${MLO_IMAGE} ${MLO_SYMLINK}
	package_stagefile_shell ${DEPLOY_DIR_IMAGE}/${MLO_SYMLINK}
}
do_deploy[dirs] = "${S}"
addtask deploy before do_build after do_install
