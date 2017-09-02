DESCRIPTION = "Linux Kernel for PDA based platforms"
SECTION = "kernel"
LICENSE = "GPL"

#DEFAULT_PREFERENCE = "-1"

DEPENDS = "coreutils-native elf-native"

FILESPATHPKG =. "linux-pda:"

inherit kernel

PR = "r1"

SRC_URI = "https://www.kernel.org/pub/linux/kernel/v4.x/linux-${PV}.tar.xz \
           file://fix_nonlinux_compile.patch \
           file://h3600-fix-micro-port-mod.patch \
           file://h3600-fix-mtd-part.patch \
           file://h3600-fix-micro-keys.patch \
           file://fix-blocking-irq0.patch \
           file://defconfig \
          "

SRC_URI[md5sum] = "6f7f4cb7ceaac770eadf0c7a18ef5225"
SRC_URI[sha256sum] = "e4944ca5bb0bdf63a7e97dc7fbdd38bcc820d8b3b57c4a3a7b3bf9c8a48216b7"

S = "${WORKDIR}/linux-${PV}"

do_configure() {
	install ${WORKDIR}/defconfig ${S}/.config
	yes '' | oe_runmake oldconfig
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
