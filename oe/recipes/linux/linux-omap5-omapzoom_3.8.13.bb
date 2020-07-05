SECTION = "kernel"
DESCRIPTION = "Linux kernel for OMAP5 kernel"
LICENSE = "GPLv2"
KERNEL_IMAGETYPE = "uImage"
COMPATIBLE_MACHINE = "board-tv"

DEFAULT_PREFERENCE = "-1"
DEFAULT_PREFERENCE_board-tv = "10"

DEPENDS = "elf-native"

inherit kernel

FILESPATHPKG =. "linux-omap5-omapzoom:"

SRCREV = "2c871a879dbb4234232126f7075468d5bf0a50e3"

KERNEL_DEVICETREE_board-tv = "omap5-igep0050.dtb"

COMPATIBLE_HOST = "arm.*-linux"

export ARCH = "arm"
export OS = "Linux"

SRC_URI = "git://git.omapzoom.org/kernel/omap.git;protocol=git;branch=p-ti-glsdk-3.8.y \
           file://compiler-gcc5.h \
           file://omap5-igep0050.dts \
           file://Kconfig \
           file://Makefile \
           file://dce.c \
           file://dce_rpc.h \
           file://omap_dce.h \
           file://fix_nonlinux_compile.patch \
           file://omapdce.patch \
           file://defconfig"

S = "${WORKDIR}/git"

do_configure_prepend () {
	cp ${WORKDIR}/compiler-gcc5.h ${S}/include/linux
	cp ${WORKDIR}/omap5-igep0050.dts ${S}/arch/arm/boot/dts
	mkdir -p ${S}/drivers/staging/omapdce
	cp ${WORKDIR}/Kconfig ${S}/drivers/staging/omapdce
	cp ${WORKDIR}/Makefile ${S}/drivers/staging/omapdce
	cp ${WORKDIR}/dce.c ${S}/drivers/staging/omapdce
	cp ${WORKDIR}/dce_rpc.h ${S}/drivers/staging/omapdce
	cp ${WORKDIR}/omap_dce.h ${S}/drivers/staging/omapdce
}

do_configure() {
	install ${WORKDIR}/defconfig ${S}/.config
	install ${WORKDIR}/defconfig ${S}/.config.old
}

do_compile() {
	HOST_INC=-I${STAGING_INCDIR_NATIVE}
	unset CFLAGS CPPFLAGS CXXFLAGS LDFLAGS MACHINE
	oe_runmake ${KERNEL_IMAGETYPE} ${KERNEL_EXTRA_OEMAKE} HOST_INC=${HOST_INC}
}

do_compile_kernelmodules() {
	HOST_INC=-I${STAGING_INCDIR_NATIVE}
	unset CFLAGS CPPFLAGS CXXFLAGS LDFLAGS MACHINE
	if (grep -q -i -e '^CONFIG_MODULES=y$' .config); then
		oe_runmake modules ${KERNEL_EXTRA_OEMAKE} HOST_INC=${HOST_INC}
	else
		oenote "no modules to compile"
	fi
}

do_install_append() {
	#oe_runmake headers_install INSTALL_HDR_PATH=${D}${exec_prefix}/src/linux-${KERNEL_VERSION} ARCH=$ARCH
	install -d ${D}${exec_prefix}/include/linux
}

staging_helper_append() {
}

PACKAGES =+ "kernel-headers"
FILES_kernel-headers = "${exec_prefix}/src/linux*"
