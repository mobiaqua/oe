ARM_INSTRUCTION_SET = "arm"

require gcc-common.inc

PR = "r1"

# Third digit in PV should be incremented after a minor release
# happens from this branch on gcc e.g. currently its 4.7.1
# when 4.7.2 is releases and we bump SRCREV beyond the release
# on branch then PV should be incremented to 4.7.2+svnr${SRCPV}
# to reflect that change

PV = "4.7.2"

# BINV should be incremented after updating to a revision
# after a minor gcc release (e.g. 4.7.1 or 4.7.2) has been made
# the value will be minor-release+1 e.g. if current minor release was
# 4.7.1 then the value below will have 2 which will mean 4.7.2
# which will be next minor release and so on.

BINV = "4.7.2"

INC_PR = "r1"

FILESDIR = "${@os.path.dirname(bb.data.getVar('FILE',d,1))}/gcc-4.7"

DEPENDS =+ "mpfr gmp libmpc zlib"
NATIVEDEPS = "mpfr-native gmp-native libmpc-native zlib-native"

LICENSE="GPL-3.0-with-GCC-exception & GPLv3"

SRC_URI = "${GNU_MIRROR}/gcc/gcc-${PV}/gcc-${PV}.tar.bz2 \
	   file://gcc-4.3.1-ARCH_FLAGS_FOR_TARGET.patch \
	   file://cache-amnesia.patch \
	   file://gcc-flags-for-build.patch \
	   file://zecke-xgcc-cpp.patch \
           file://304-index_macro.patch  \
           file://305-libmudflap-susv3-legacy.patch  \
           file://arm-nolibfloat.patch \
           file://gcc-poison-system-directories.patch \
           file://gcc-poison-dir-extend.patch \
           file://drop_define.patch \
           file://gcc-4.3.3-SYSROOT_CFLAGS_FOR_TARGET.patch \
	   file://64bithack.patch \
	   file://optional_libstdc.patch \
	   file://disable_relax_pic_calls_flag.patch \
	   file://COLLECT_GCC_OPTIONS.patch \
	   file://pr32219.patch \
	   file://fortran-cross-compile-hack.patch \
	   file://libgcc-sjlj-check.patch \
	   file://cpp-honor-sysroot.patch \
	   file://GLIBC_DYNAMIC_LINKER.patch \
	   file://arm-hard-float-loader.patch \
	   file://gcc-argument-list-too-long.patch \
	   file://fix-g++-sysroot.patch \
	   file://disablesdt.patch \
	   file://libtool.patch \
	   file://gcc-armv4-pass-fix-v4bx-to-ld.patch \
	   file://0001-crtstuff.c-USE_PT_GNU_EH_FRAME-Define-for-systems-us.patch \
	   file://0001-Makefile.in-vis_hide-gen-hide-list-Do-not-make-defin.patch \
	   file://wcast-qual-PR55383.patch \
	   file://disable-texinfo-5.patch \
	   file://avoid-oob-array-access.patch \
	  "
SRC_URI[md5sum] = "cc308a0891e778cfda7a151ab8a6e762"
SRC_URI[sha256sum] = "8a9283d7010fb9fe5ece3ca507e0af5c19412626384f8a5e9434251ae100b084"

# Language Overrides
FORTRAN = ""
JAVA = ""

EXTRA_OECONF_BASE = " --enable-lto --enable-libssp --disable-bootstrap --disable-libgomp --disable-libmudflap"
EXTRA_OECONF_INITIAL = "--disable-libmudflap --disable-libgomp --disable-libssp --enable-decimal-float=no --disable-libquadmath"
EXTRA_OECONF_INTERMEDIATE = "--disable-libmudflap --disable-libgomp --disable-libssp --disable-libquadmath"
