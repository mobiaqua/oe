SECTION = "kernel"
DESCRIPTION = "Linux kernel for OMAP4 kernel"
LICENSE = "GPLv2"
KERNEL_IMAGETYPE = "uImage"
COMPATIBLE_MACHINE = "board-tv"

DEFAULT_PREFERENCE = "-1"
DEFAULT_PREFERENCE_board-tv = "10"

DEPENDS = "elf-native"

inherit kernel

FILESPATHPKG =. "linux-omap4:"

SRCREV = "ti-ubuntu-3.4.0-1491.3"

COMPATIBLE_HOST = "arm.*-linux"

export ARCH = "arm"
export OS = "Linux"

SRC_URI = "git://dev.omapzoom.org/pub/scm/integration/kernel-ubuntu.git;protocol=git;branch=ti-ubuntu-3.4-stable \
           file://fix_nonlinux_compile.patch \
           file://patch-3.4.103.patch \
           file://patch-3.4.103-104.patch \
           file://patch-3.4.104-105.patch \
           file://patch-3.4.105-106.patch \
           file://patch-3.4.106-107.patch \
           file://patch-3.4.107-108.patch \
           file://patch-3.4.108-109.patch \
           file://patch-3.4.109-110.patch \
           file://patch-3.4.110-111.patch \
           file://patch-3.4.111-112.patch \
           file://patch-3.4.112-113.patch \
           file://not_use_IEC958_AES1_PRO_MODE_NOTID.patch \
           file://DM_INH_and_CA.patch \
           file://set_buffer_bytes.patch \
           file://zorder.patch \
           file://reverse_zorder.patch \
           file://fixed-drm-flags.patch \
           file://fixed-bt-mux.patch \
           file://rpmsg-new-ns.patch \
           file://defconfig"

S = "${WORKDIR}/git"

do_configure() {
	install ${WORKDIR}/defconfig ${S}/.config
	install ${WORKDIR}/defconfig ${S}/.config.old
#	yes '' | oe_runmake oldconfig
}

do_compile() {
	HOST_INC=-I${STAGING_INCDIR_NATIVE}
	unset CFLAGS CPPFLAGS CXXFLAGS LDFLAGS MACHINE
	oe_runmake include/linux/version.h ${KERNEL_EXTRA_OEMAKE}
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

	install -d ${STAGING_DIR_TARGET}/${includedir}/omap
	cp -L ${S}/drivers/staging/omapdrm/omap_drv.h ${STAGING_DIR_TARGET}/${includedir}/omap/
	cp -L ${S}/drivers/staging/omapdce/omap_dce.h ${STAGING_DIR_TARGET}/${includedir}/omap/
	cp -L ${S}/drivers/staging/omapdce/dce_rpc.h ${STAGING_DIR_TARGET}/${includedir}/omap/
}

staging_helper_append() {
	rm -f ${STAGING_KERNEL_DIR}/include/linux/omap_drm.h
	rm -f ${STAGING_KERNEL_DIR}/include/linux/omap_drv.h
	if [ -f ${S}/include/linux/omap_drm.h ]; then
		cp -L ${S}/include/linux/omap_drm.h ${STAGING_KERNEL_DIR}/include/linux
		cp -L ${S}/include/linux/omap_drv.h ${STAGING_KERNEL_DIR}/include/linux
	fi
}

PACKAGES =+ "kernel-headers"
FILES_kernel-headers = "${exec_prefix}/src/linux*"
