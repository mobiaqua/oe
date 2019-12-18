DESCRIPTION = "gdb - GNU debugger"
HOMEPAGE = "http://www.gnu.org/software/gdb/"
LICENSE="GPLv3+"
SECTION = "devel"
DEPENDS = "ncurses readline"

INC_PR = "r0"

LIC_FILES_CHKSUM = "file://COPYING;md5=59530bdf33659b29e73d4adb9f9f6552 \
		file://COPYING.LIB;md5=9f604d8a4f8e74f4f5140845a21b6674 \
		file://COPYING3;md5=d32239bcb673463ab874e80d47fae504 \
		file://COPYING3.LIB;md5=6a6a8e020838b23406c81b19c1d46df6"
inherit autotools

SRC_URI = "${GNU_MIRROR}/gdb/gdb-${PV}.tar.gz \
           file://0001-gdbserver-ctrl-c-handling.patch \
           file://0002-make-man-install-relative-to-DESTDIR.patch \
           file://0003-mips-linux-nat-Define-_ABIO32-if-not-defined.patch \
           file://0004-ppc-ptrace-Define-pt_regs-uapi_pt_regs-on-GLIBC-syst.patch \
           file://0005-Add-support-for-Renesas-SH-sh4-architecture.patch \
           file://0006-Dont-disable-libreadline.a-when-using-disable-static.patch \
           file://0007-use-asm-sgidefs.h.patch \
           file://0008-Use-exorted-definitions-of-SIGRTMIN.patch \
           file://0009-Change-order-of-CFLAGS.patch \
           file://0010-resolve-restrict-keyword-conflict.patch \
           file://0011-Fix-invalid-sigprocmask-call.patch \
           file://CVE-2019-1010180.patch \
"

SRC_URI[md5sum] = "3c61672225a6a80875a5eea8cd25e2a0"
SRC_URI[sha256sum] = "26ce655216cd03f4611518a7a1c31d80ec8e884c16715e9ba8b436822e51434b"

export CC_FOR_BUILD = "${BUILD_CC}"
export CXX_FOR_BUILD = "${BUILD_CXX}"
export CPP_FOR_BUILD = "${BUILD_CPP}"
export CFLAGS_FOR_BUILD = "${BUILD_CFLAGS}"
export CXXFLAGS_FOR_BUILD = "${BUILD_CXXFLAGS}"
export CPPFLAGS_FOR_BUILD = "${BUILD_CPPFLAGS}"

B = "${WORKDIR}/build-${TARGET_SYS}"

EXTRA_OEMAKE = "'SUBDIRS=intl mmalloc libiberty opcodes bfd sim gdb etc utils'"

EXTRA_OECONF = "--disable-gdbtk --disable-tui --disable-x --disable-werror \
                --with-curses --disable-multilib --with-system-readline --disable-sim \
                --without-lzma --without-guile \
                ${GDBPROPREFIX} \
                ${@base_contains('DISTRO_FEATURES', 'multiarch', '--enable-64-bit-bfd', '', d)} \
                --disable-rpath \
                --disable-gas --disable-binutils \
                --disable-ld --disable-gold \
                --disable-gprof \
               "

GDBPROPREFIX = "--program-prefix=''"

do_configure () {
	# override this function to avoid the autoconf/automake/aclocal/autoheader
	# calls for now
	(cd ${S} && gnu-configize) || die "failure in running gnu-configize"
	oe_runconf
}

# we don't want gdb to provide bfd/iberty/opcodes, which instead will override the
# right bits installed by binutils.
do_install_append() {
	rm -rf ${D}${libdir}
	rm -rf ${D}${includedir}
	rm -rf ${D}${datadir}/locale
}

RRECOMMENDS_gdb_append_linux = " glibc-thread-db "
RRECOMMENDS_gdb_append_linux-gnueabi = " glibc-thread-db "
RRECOMMENDS_gdbserver_append_linux = " glibc-thread-db "
RRECOMMENDS_gdbserver_append_linux-gnueabi = " glibc-thread-db "

