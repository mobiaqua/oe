require binutils.inc
PR = "${INC_PR}.3"
LICENSE = "GPLv3"

SRC_URI = "\
     ${GNU_MIRROR}/binutils/binutils-${PV}.tar.bz2 \
     file://libtool-update.patch \
     file://libtool-update-new.patch \
     file://libiberty_path_fix.patch \
     file://binutils-poison.patch \
     file://binutils-armv5e.patch \
     file://fix-pr15815.patch \
     file://fix-pr2404.patch \
     file://fix-pr16476.patch \
     file://fix-pr16428.patch \
     file://replace_macros_with_static_inline.patch \
     file://binutils-uninitialised-warning.patch \
     file://binutils_CVE-2014-8484.patch \
     file://binutils_CVE-2014-8485.patch \
     file://binutils_CVE-2014-8501.patch \
     file://binutils_CVE-2014-8502_1.patch \
     file://binutils_CVE-2014-8502.patch \
     file://binutils_CVE-2014-8503.patch \
     file://binutils_CVE-2014-8504.patch \
     file://binutils_CVE-2014-8737.patch \
     file://fix-undef-behavior.patch \
     "

SRC_URI[md5sum] = "e0f71a7b2ddab0f8612336ac81d9636b"
SRC_URI[sha256sum] = "e5e8c5be9664e7f7f96e0d09919110ab5ad597794f5b1809871177a0f0f14137"

S = "${WORKDIR}/binutils-${PV}"

FILES_${PN}-symlinks += "${bindir}/elfedit"
# When we enable gold this might need to be make conditional
FILES_${PN}-symlinks += "${bindir}/ld.bfd"
