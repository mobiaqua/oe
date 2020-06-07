SECTION = "kernel"
DESCRIPTION = "Linux kernel for OMAP5 kernel"
LICENSE = "GPLv2"
KERNEL_IMAGETYPE = "uImage"
COMPATIBLE_MACHINE = "board-tv"

DEFAULT_PREFERENCE = "-1"
DEFAULT_PREFERENCE_board-tv = "10"

DEPENDS = "elf-native"

inherit kernel

FILESPATHPKG =. "linux-omap5:"

SRCREV = "21a15144ad912b3377babaf366ca47e3a956dd84"

KERNEL_DEVICETREE_board-tv = "omap5-igep0050.dtb"

COMPATIBLE_HOST = "arm.*-linux"

export ARCH = "arm"
export OS = "Linux"

SRC_URI = "git://github.com/mobiaqua/igep-linux.git;protocol=git;branch=linux-3.8.y-omap5 \
           file://compiler-gcc5.h \
           file://fix_nonlinux_compile.patch \
           file://defconfig"

S = "${WORKDIR}/git"

do_configure_prepend () {
	cp ${WORKDIR}/compiler-gcc5.h ${S}/include/linux
}

do_configure() {
	install ${WORKDIR}/defconfig ${S}/.config
	install ${WORKDIR}/defconfig ${S}/.config.old
#	yes '' | oe_runmake oldconfig
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
	oe_runmake headers_install INSTALL_HDR_PATH=${D}${exec_prefix}/src/linux-${KERNEL_VERSION} ARCH=$ARCH
	install -d ${D}${exec_prefix}/include/linux
}

staging_helper_append() {
}

PACKAGES =+ "kernel-headers"
FILES_kernel-headers = "${exec_prefix}/src/linux*"
