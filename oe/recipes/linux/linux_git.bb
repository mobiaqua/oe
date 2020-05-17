SECTION = "kernel"
DESCRIPTION = "Linux kernel for OMAP devices"
LICENSE = "GPLv2"
KERNEL_IMAGETYPE = "uImage"
COMPATIBLE_MACHINE = "board-tv"

DEFAULT_PREFERENCE = "-1"

DEPENDS = "libelf-native"

inherit kernel

require linux-dtb.inc

FILESPATHPKG =. "linux-git:"

KERNEL_DEVICETREE_board-tv = "omap3-igep0030.dtb omap4-panda.dtb omap4-panda-es.dtb \
                              omap5-igep0050.dtb am57xx-beagle-x15-revb1.dtb"

SRCREV = "2ef96a5bb12be62ef75b5828c0aab838ebb29cb8"

COMPATIBLE_HOST = "arm.*-linux"

export ARCH = "arm"
export OS = "Linux"

SRC_URI = "git://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git;protocol=git \
           file://use_local_elf.patch \
           file://remove_uuid_usage.patch \
           file://0001-HACK-PandaBoard-Bring-back-twl6030-clk32kg-regulator.patch \
           file://defconfig"

S = "${WORKDIR}/git"

do_configure_prepend () {
	cp ${STAGING_INCDIR_NATIVE}/elf.h ${S}/scripts
}

do_configure() {
	install ${WORKDIR}/defconfig ${S}/.config
	install ${WORKDIR}/defconfig ${S}/.config.old
}

do_compile() {
	HOST_INC=-I${STAGING_INCDIR_NATIVE}
	unset CFLAGS CPPFLAGS CXXFLAGS LDFLAGS MACHINE
	oe_runmake ${KERNEL_IMAGETYPE} ${KERNEL_EXTRA_OEMAKE} \
		HOSTCFLAGS=${HOSTCFLAGS} LOADADDR=${UBOOT_LOADADDRESS}
}

do_compile_kernelmodules() {
	HOSTCFLAGS=-I${STAGING_INCDIR_NATIVE}
	unset CFLAGS CPPFLAGS CXXFLAGS LDFLAGS MACHINE
	if (grep -q -i -e '^CONFIG_MODULES=y$' .config); then
		oe_runmake modules ${KERNEL_EXTRA_OEMAKE} HOSTCFLAGS=${HOSTCFLAGS}
	else
		oenote "no modules to compile"
	fi
}

do_install_append() {
	oe_runmake headers_install INSTALL_HDR_PATH=${D}${exec_prefix}/src/linux-${KERNEL_VERSION} ARCH=$ARCH
	install -d ${D}${exec_prefix}/include/linux
}

PACKAGES =+ "kernel-headers"
FILES_kernel-headers = "${exec_prefix}/src/linux*"
