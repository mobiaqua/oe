SUMMARY = "Free Implementation of the Unicode Bidirectional Algorithm"
SECTION = "libs"
LICENSE = "LGPLv2.1+"
LIC_FILES_CHKSUM = "file://COPYING;md5=a916467b91076e631dd8edb7424769c7"

SRC_URI = "https://github.com/${BPN}/${BPN}/releases/download/v${PV}/${BP}.tar.bz2 \
           "
SRC_URI[md5sum] = "4c020b0f5136dd012ee00f1e1122f6aa"
SRC_URI[sha256sum] = "5ab5f21e9f2fc57b4b40f8ea8f14dba78a5cc46d9cf94bc5e00a58e6886a935d"

inherit lib_package pkgconfig autotools

BBCLASSEXTEND = "native nativesdk"
