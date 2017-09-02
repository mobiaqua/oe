UBOOT_MACHINE = "omap4_panda_config"
DESCRIPTION = "U-Boot - the Universal Boot Loader"
HOMEPAGE = "http://www.denx.de/wiki/U-Boot/WebHome"
SECTION = "bootloaders"
PRIORITY = "optional"
LICENSE = "GPLv2"
PROVIDES += "u-boot x-load"

DEFAULT_PREFERENCE = "-1"
DEFAULT_PREFERENCE_board-tv = "10"

COMPATIBLE_MACHINE = "board-tv"

SRCREV = "dc80e007bc12f30e087891873332ddb3c710c6f3"

PV = "2012.04.01+${PR}+gitr${SRCREV}"
PE = "1"

FILESPATHPKG =. "files/pandaboard:files:"

SRC_URI = "git://git.linaro.org/boot/u-boot-linaro-stable.git;protocol=git \
	   file://panda_no_delay.patch \
	   file://fix_older_cards.patch \
	   file://boot-panda-label.script \
	   file://boot-panda-sdcard.script \
	   file://boot-panda-nfs.script \
	  "

S = "${WORKDIR}/git"

PACKAGE_ARCH = "${MACHINE_ARCH}"
PARALLEL_MAKE = ""

EXTRA_OEMAKE = "CROSS_COMPILE=${TARGET_PREFIX}"

UBOOT_MACHINE = "omap4_panda_config"

UBOOT_BINARY ?= "u-boot.bin"
UBOOT_IMAGE ?= "u-boot-${MACHINE}-${PV}-${PR}.bin"
UBOOT_SYMLINK ?= "u-boot.bin"
UBOOT_MAKE_TARGET ?= "all"

MLO_IMAGE ?= "MLO-${MACHINE}-${PV}-${PR}"
MLO_SYMLINK ?= "MLO"

do_configure () {
	oe_runmake ${UBOOT_MACHINE}

	sed -i -e s,NFS_IP,${MA_NFS_IP},g ${WORKDIR}/boot-panda-nfs.script
	sed -i -e s,NFS_PATH,${MA_NFS_PATH},g ${WORKDIR}/boot-panda-nfs.script
	sed -i -e s,TARGET_MAC,${MA_TARGET_MAC},g ${WORKDIR}/boot-panda-nfs.script
	sed -i -e s,TARGET_MAC,${MA_TARGET_MAC},g ${WORKDIR}/boot-panda-label.script
	sed -i -e s,TARGET_MAC,${MA_TARGET_MAC},g ${WORKDIR}/boot-panda-sdcard.script
}

do_compile () {
	unset LDFLAGS
	unset CFLAGS
	unset CPPFLAGS
	oe_runmake ${UBOOT_MAKE_TARGET}
	oe_runmake tools env HOSTCC="${CC}"
}

do_install () {
	install -d ${D}/boot
	install -m 0644 ${S}/${UBOOT_BINARY} ${D}/boot/${UBOOT_IMAGE}
	ln -sf ${UBOOT_IMAGE} ${D}/boot/${UBOOT_BINARY}
	install -m 0644 ${WORKDIR}/boot-panda-sdcard.script ${D}/boot/uEnv-sdcard.txt
	install -m 0644 ${WORKDIR}/boot-panda-label.script ${D}/boot/uEnv-label.txt
	install -m 0644 ${WORKDIR}/boot-panda-nfs.script ${D}/boot/uEnv-nfs.txt
	ln -sf uEnv-label.txt ${D}/boot/uEnv.txt
	install -m 0644 ${S}/MLO ${D}/boot/${MLO_IMAGE}
	ln -sf ${MLO_IMAGE} ${D}/boot/${MLO_SYMLINK}
}

FILES_${PN} = "/boot"
# no gnu_hash in uboot.bin, by design, so skip QA
INSANE_SKIP_${PN} = True

PACKAGES += "${PN}-fw-utils"
FILES_${PN}-fw-utils = "${sysconfdir} ${base_sbindir}"
# u-boot doesn't use LDFLAGS for fw files, needs to get fixed, but until then:
INSANE_SKIP_${PN}-fw-utils = True

do_deploy () {
	install -d ${DEPLOY_DIR_IMAGE}
	install -m 0644 ${S}/${UBOOT_BINARY} ${DEPLOY_DIR_IMAGE}/${UBOOT_IMAGE}
	package_stagefile_shell ${DEPLOY_DIR_IMAGE}/${UBOOT_IMAGE}

	cd ${DEPLOY_DIR_IMAGE}
	rm -f ${UBOOT_SYMLINK}
	ln -sf ${UBOOT_IMAGE} ${UBOOT_SYMLINK}
	package_stagefile_shell ${DEPLOY_DIR_IMAGE}/${UBOOT_SYMLINK}

	install -m 0644 ${D}/boot/uEnv-label.txt ${DEPLOY_DIR_IMAGE}
	install -m 0644 ${D}/boot/uEnv-sdcard.txt ${DEPLOY_DIR_IMAGE}
	install -m 0644 ${D}/boot/uEnv-nfs.txt ${DEPLOY_DIR_IMAGE}
	rm -f uEnv.txt
	ln -sf uEnv-label.txt uEnv.txt
	package_stagefile_shell ${DEPLOY_DIR_IMAGE}/uEnv.txt

	install -m 0644 ${S}/MLO ${DEPLOY_DIR_IMAGE}/${MLO_IMAGE}
	ln -sf ${MLO_IMAGE} ${DEPLOY_DIR_IMAGE}/${MLO_SYMLINK}
}
do_deploy[dirs] = "${S}"
addtask deploy before do_package_stage after do_compile
