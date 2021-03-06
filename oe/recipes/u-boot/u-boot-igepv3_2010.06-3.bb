DESCRIPTION = "U-boot for IGEP based platforms"
SECTION = "bootloader"
LICENSE = "GPL"

PROVIDES += "u-boot"
DEPENDS = "mtd-utils"

DEFAULT_PREFERENCE = "-1"
DEFAULT_PREFERENCE_igep0030 = "1"

PR = "r1"

COMPATIBLE_MACHINE = "igep0030"

SRC_URI = "http://downloads.igep.es/sources/u-boot-arm-${PV}.tar.gz \
	   file://no_getline.patch \
	  "

S = "${WORKDIR}/u-boot-arm-${PV}"

SRC_URI[md5sum] = "3f5f8bad8a6d8a6965b54dda6356915a"
SRC_URI[sha256sum] = "ffee2ad7743ac6cfbb44dd3b551487af73f585ff533a08872ae0018c7ff6b907"

PACKAGE_ARCH = "${MACHINE_ARCH}"
PARALLEL_MAKE = ""

EXTRA_OEMAKE = "CROSS_COMPILE=${TARGET_PREFIX}"

UBOOT_MACHINE = "igep0030_config"

UBOOT_BINARY ?= "u-boot.bin"
UBOOT_IMAGE ?= "u-boot-${MACHINE}-${PV}-${PR}.bin"
UBOOT_SYMLINK ?= "u-boot.bin"
UBOOT_MAKE_TARGET ?= "all"

do_configure () {
	oe_runmake ${UBOOT_MACHINE}
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

	if [ -e ${WORKDIR}/fw_env.config ] ; then
		install -d ${D}${base_sbindir}
		install -d ${D}${sysconfdir}
		install -m 644 ${WORKDIR}/fw_env.config ${D}${sysconfdir}/fw_env.config
		install -m 755 ${S}/tools/env/fw_printenv ${D}${base_sbindir}/fw_printenv
	install -m 755 ${S}/tools/env/fw_printenv ${D}${base_sbindir}/fw_setenv
	fi
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
}
do_deploy[dirs] = "${S}"
addtask deploy before do_package_stage after do_compile
