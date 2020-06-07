SECTION = "kernel"
DESCRIPTION = "Linux kernel for OMAP devices"
LICENSE = "GPLv2"
KERNEL_IMAGETYPE = "uImage"
COMPATIBLE_MACHINE = "board-tv"

DEFAULT_PREFERENCE = "-1"

DEPENDS = "libelf-native openssl-native"

inherit kernel

require linux-dtb.inc

FILESPATHPKG =. "linux-ti-omap419:"

KERNEL_DEVICETREE_board-tv = "omap3-igep0030.dtb omap4-panda.dtb omap4-panda-es.dtb \
                              omap5-igep0050.dtb am57xx-beagle-x15-revb1.dtb"

COMPATIBLE_HOST = "arm.*-linux"

export ARCH = "arm"
export OS = "Linux"

SRCREV = "5a23bc00e08d26bb83952953d909c95b42fab70c"

SRC_URI = "git://git.ti.com/ti-linux-kernel/ti-linux-kernel.git;protocol=git;branch=ti-linux-4.19.y \
           file://use_local_elf.patch \
           file://0001-cpufreq-opp-dont-fail-_opp_add_static_v2-temp-till-o.patch \
           file://0001-bootup-hacks-move-mmc-early.patch \
           file://0002-HACK-PandaBoard-Bring-back-twl6030-clk32kg-regulator.patch \
           file://twl6030.dtsi.patch \
           file://0003-hack-gpiolib-yes-we-have-drivers-stomping-on-each-ot.patch \
           file://fixed_name_hdmi_audio.patch \
           file://smsc95xx-add-macaddr-module-parameter.patch \
           file://panda-bt-fixes.patch \
           file://0003-bootup-hacks-xor-select-neon-or-arm4regs.patch \
           file://remote-proc-map-fix.patch \
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
	HOSTCFLAGS=-I${STAGING_INCDIR_NATIVE}
	HOSTLDFLAGS=-L${STAGING_LIBDIR_NATIVE}
	unset CFLAGS CPPFLAGS CXXFLAGS LDFLAGS MACHINE
	oe_runmake ${KERNEL_IMAGETYPE} ${KERNEL_EXTRA_OEMAKE} \
		HOSTCFLAGS=${HOSTCFLAGS} HOSTLDFLAGS=${HOSTLDFLAGS} LOADADDR=${UBOOT_LOADADDRESS}
}

do_compile_kernelmodules() {
	HOSTCFLAGS=-I${STAGING_INCDIR_NATIVE}
	HOSTLDFLAGS=-L${STAGING_LIBDIR_NATIVE}
	unset CFLAGS CPPFLAGS CXXFLAGS LDFLAGS MACHINE
	if (grep -q -i -e '^CONFIG_MODULES=y$' .config); then
		oe_runmake modules ${KERNEL_EXTRA_OEMAKE} \
			HOSTCFLAGS=${HOSTCFLAGS} HOSTLDFLAGS=${HOSTLDFLAGS}
	else
		oenote "no modules to compile"
	fi
}

do_install_append() {
	oe_runmake headers_install INSTALL_HDR_PATH=${D}${exec_prefix}/src/linux-${KERNEL_VERSION} ARCH=$ARCH
}

PACKAGES =+ "kernel-headers"
FILES_kernel-headers = "${exec_prefix}/src/linux*"
