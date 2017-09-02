DESCRIPTION = "libftdi is a library to talk to FTDI chips.\
FT232BM/245BM, FT2232C/D and FT232/245R using libusb,\
including the popular bitbang mode."
HOMEPAGE = "http://www.intra2net.com/en/developer/libftdi/"
LICENSE = "LGPL GPLv2+linking exception"
SECTION = "libs"

DEPENDS = "virtual/libusb1"
DEPENDS_virtclass-native = "virtual/libusb1-native"
SRC_URI = "http://www.intra2net.com/en/developer/libftdi/download/libftdi1-${PV}.tar.bz2 \
           file://autotools.patch \
          "

SRC_URI[md5sum] = "89dff802d89c4c0d55d8b4665fd52d0b"
SRC_URI[sha256sum] = "a6ea795c829219015eb372b03008351cee3fb39f684bff3bf8a4620b558488d6"

inherit autotools binconfig pkgconfig

BBCLASSEXTEND = "native"

