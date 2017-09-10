require binutils.inc
PR = "${INC_PR}.3"
LICENSE = "GPLv3"

SRCREV = "8f2db47d13b6afdba8e7b09a01ac2a0ca41d708e"
SRC_URI = "\
     git://sourceware.org/git/binutils-gdb.git;branch=binutils-2_25-branch;protocol=git \
     file://0004-Only-generate-an-RPATH-entry-if-LD_RUN_PATH-is-not-e.patch \
     file://0006-Use-libtool-2.4.patch \
     file://0007-Add-the-armv5e-architecture-to-binutils.patch \
     file://0008-don-t-let-the-distro-compiler-point-to-the-wrong-ins.patch \
     file://0009-Upstream-Status-Inappropriate-distribution-codesourc.patch \
     file://0010-Fix-rpath-in-libtool-when-sysroot-is-enabled.patch \
     file://0013-Fix-an-internal-error-in-do_print_to_mapfile-seen-wi.patch \
     file://0014-gold-arm-Skip-pic-check-for-R_ARM_REL32.patch \
     file://0015-Fix-dynamic-list-so-that-symbols-not-in-the-list-are.patch \
     file://0016-This-patch-adds-IFUNC-support-for-arm-gold-backend.patch \
     file://fix-undef-behavior.patch \
     file://use-m4.patch \
     "

SRC_URI[md5sum] = "e0f71a7b2ddab0f8612336ac81d9636b"
SRC_URI[sha256sum] = "e5e8c5be9664e7f7f96e0d09919110ab5ad597794f5b1809871177a0f0f14137"

S  = "${WORKDIR}/git"

FILES_${PN}-symlinks += "${bindir}/elfedit"
# When we enable gold this might need to be make conditional
FILES_${PN}-symlinks += "${bindir}/ld.bfd"
