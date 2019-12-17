DESCRIPTION = "Lossless data compression library"
HOMEPAGE = "http://www.oberhumer.com/opensource/lzo/"
SECTION = "libs"
LICENSE = "GPLv2+"
PR = "r1"

SRC_URI = "http://www.oberhumer.com/opensource/lzo/download/lzo-${PV}.tar.gz \
           file://0001-Use-memcpy-instead-of-reinventing-it.patch \
           file://0001-Add-pkgconfigdir-to-solve-the-undefine-error.patch \
           "

SRC_URI[md5sum] = "39d3f3f9c55c87b1e5d6888e1420f4b5"
SRC_URI[sha256sum] = "c0f892943208266f9b6543b3ae308fab6284c5c90e627931446fb49b4221a072"

inherit autotools

EXTRA_OECONF = "--enable-shared"

BBCLASSEXTEND = "native nativesdk"
