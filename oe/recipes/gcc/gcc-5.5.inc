ARM_INSTRUCTION_SET = "arm"

require gcc-common.inc

PR = "r1"

# Third digit in PV should be incremented after a minor release

PV = "5.5"

# BINV should be incremented to a revision after a minor gcc release

BINV = "5.5"

INC_PR = "r1"

FILESDIR = "${@os.path.dirname(bb.data.getVar('FILE',d,1))}/gcc-5.5"

DEPENDS =+ "mpfr gmp libmpc zlib"
NATIVEDEPS = "mpfr-native gmp-native libmpc-native zlib-native"

LICENSE = "GPL-3.0-with-GCC-exception & GPLv3"

#BASEURI = "${GNU_MIRROR}/gcc/gcc-${PV}/gcc-${PV}.tar.bz2"
BASEURI = "http://releases.linaro.org/components/toolchain/gcc-linaro/5.5-2017.10/gcc-linaro-5.5-2017.10.tar.xz"

SRC_URI = "\
           ${BASEURI} \
           file://0001-gcc-4.3.1-ARCH_FLAGS_FOR_TARGET.patch \
           file://0010-gcc-poison-system-directories.patch \
           file://0011-gcc-poison-dir-extend.patch \
           file://0012-gcc-4.3.3-SYSROOT_CFLAGS_FOR_TARGET.patch \
           file://0013-64-bit-multilib-hack.patch \
           file://0014-optional-libstdc.patch \
           file://0015-gcc-disable-MASK_RELAX_PIC_CALLS-bit.patch \
           file://0016-COLLECT_GCC_OPTIONS.patch \
           file://0019-libgcc-sjlj-check.patch \
           file://0020-cpp-honor-sysroot.patch \
           file://0023-gcc-Fix-argument-list-too-long-error.patch \
           file://0024-Disable-sdt.patch \
           file://0025-libtool.patch \
           file://0026-gcc-armv4-pass-fix-v4bx-to-linker-to-support-EABI.patch \
           file://0029-export-CPP.patch \
           file://0032-Ensure-target-gcc-headers-can-be-included.patch \
           file://0033-gcc-4.8-won-t-build-with-disable-dependency-tracking.patch \
           file://0034-Don-t-search-host-directory-during-relink-if-inst_pr.patch \
           file://0035-Dont-link-the-plugins-with-libgomp-explicitly.patch \
           file://0036-Use-SYSTEMLIBS_DIR-replacement-instead-of-hardcoding.patch \
           file://0039-libcc1-fix-libcc1-s-install-path-and-rpath.patch \
           file://0041-Search-target-sysroot-gcc-version-specific-dirs-with.patch \
           file://0042-Fix-various-_FOR_BUILD-and-related-variables.patch \
           file://0048-ssp_nonshared.patch \
           file://0051-Ignore-fdebug-prefix-map-in-producer-string-by-Danie.patch \
           file://0053-expr.c-PR-target-65358-Avoid-clobbering-partial-argu.patch \
           file://0054-support-ffile-prefix-map.patch \
           file://0055-Reuse-fdebug-prefix-map-to-replace-ffile-prefix-map.patch \
           file://0058-fdebug-prefix-map-support-to-remap-relative-path.patch \
           file://0059-libgcc-use-ldflags.patch \
           file://CVE-2016-6131.patch \
           file://drop_define.patch \
"

S = "${WORKDIR}/gcc-linaro-5.5-2017.10"
B = "${S}/build.${HOST_SYS}.${TARGET_SYS}"

SRC_URI[md5sum] = "8e6dc4d4d9d295077e917d205a186d29"
SRC_URI[sha256sum] = "acbf1eda48439b83bf49f5a24ed3e4c31bd1680f3cebd9282e0c21eb3cdeb25c"

# Language Overrides
FORTRAN = ""
JAVA = ""

EXTRA_OECONF_BASE = "\
 --enable-lto \
 --enable-libssp \
 --enable-libitm \
 --disable-bootstrap \
 --disable-libgomp \
 --disable-libmudflap \
 --with-system-zlib \
 --with-linker-hash-style=sysv \
 --enable-linker-build-id \
 --with-ppl=no \
 --with-cloog=no \
 --enable-checking=release \
 --enable-cheaders=c_global \
 --without-isl \
"

EXTRA_OECONF_INTERMEDIATE = "\
 --disable-libmudflap \
 --disable-libgomp \
 --disable-libitm \
 --disable-libquadmath \
 --disable-libatomic \
 --with-system-zlib \
 --disable-lto \
 --disable-plugin \
 --without-isl \
 --disable-libssp \
 gcc_cv_libc_provides_ssp=yes \
"

EXTRA_OECONF_INITIAL = "\
 --disable-libmudflap \
 --disable-libgomp \
 --disable-libitm \
 --disable-libquadmath \
 --with-system-zlib \
 --disable-lto \
 --disable-plugin \
 --enable-decimal-float=no \
 --without-isl \
 --disable-libssp \
 gcc_cv_libc_provides_ssp=yes \
"

EXTRA_OECONF_PATHS = "\
    --with-gxx-include-dir=/not/exist{target_includedir}/c++/${BINV} \
    --with-sysroot=/not/exist \
    --with-build-sysroot=${STAGING_DIR_TARGET} \
"
