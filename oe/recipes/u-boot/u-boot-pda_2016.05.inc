DESCRIPTION = "It is modified version of U-Boot - the Universal Boot Loader"
SECTION = "bootloaders"
PRIORITY = "optional"
LICENSE = "GPLv2"
PROVIDES += "u-boot x-load"

DEFAULT_PREFERENCE = "-1"
DEFAULT_PREFERENCE_pda-sa1110 = "10"
DEFAULT_PREFERENCE_pda-pxa250 = "10"

COMPATIBLE_MACHINE = "(pda-sa1110|pda-pxa250)"

SRCREV = "ecbd3dabec1768c5531b650be16983d3c9eee3f2"

PV = "v2016.05+git${SRCPV}"

SRC_URI = "git://github.com/mobiaqua/bootloader.git;branch=master;protocol=git"

S = "${WORKDIR}/git/u-boot-pda"

PACKAGE_ARCH = "${MACHINE_ARCH}"

PARALLEL_MAKE = ""

EXTRA_OEMAKE = 'CROSS_COMPILE=${TARGET_PREFIX}'

MACHINE_CONFIG = `echo ${UBOOT_MACHINE} | sed -e 's/["_config"]*$//g'`
UBOOT_BINARY ?= "u-boot.bin"
UBOOT_IMAGE ?= "u-boot-${MACHINE_CONFIG}.bin"
UBOOT_SYMLINK ?= "u-boot.bin"
UBOOT_MAKE_TARGET ?= "all"

do_compile () {
	unset LDFLAGS
	unset CFLAGS
	unset CPPFLAGS

	oe_runmake clean
	oe_runmake ${UBOOT_MACHINE}
	oe_runmake ${UBOOT_MAKE_TARGET}
}

do_install () {
	install -d ${D}/boot
	install -m 0644 ${S}/${UBOOT_BINARY} ${D}/boot/${UBOOT_IMAGE}
}

FILES_${PN} = "/boot"
# no gnu_hash in uboot.bin, by design, so skip QA
INSANE_SKIP_${PN} = True

do_deploy () {
	install -d ${DEPLOY_DIR_IMAGE}
	install -m 0644 ${S}/${UBOOT_BINARY} ${DEPLOY_DIR_IMAGE}/${UBOOT_IMAGE}
	package_stagefile_shell ${DEPLOY_DIR_IMAGE}/${UBOOT_IMAGE}
}

do_deploy[dirs] = "${S}"
addtask deploy before do_package_stage after do_compile
