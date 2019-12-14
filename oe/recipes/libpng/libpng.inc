DESCRIPTION = "PNG Library"
HOMEPAGE = "http://www.libpng.org/"
LICENSE = "libpng"
SECTION = "libs"
PRIORITY = "required"

DEPENDS = "zlib"

LIBV = "16"

INC_PR = "r3"

BBCLASSEXTEND = "native"

SRC_URI = "${SOURCEFORGE_MIRROR}/project/${BPN}/${BPN}${LIBV}/${PV}/${BP}.tar.xz"
S = "${WORKDIR}/libpng-${PV}"

inherit autotools pkgconfig binconfig

EXTRA_OECONF="--enable-arm-neon=off"
EXTRA_OECONF_arm="--enable-arm-neon=off"
EXTRA_OECONF_arm6="--enable-arm-neon=off"
EXTRA_OECONF_armv7a="--enable-arm-neon=on"

PACKAGES =+ "${PN}16-dbg ${PN}16 ${PN}16-dev"

FILES_${PN}16-dbg = "${libdir}/.debug/libpng16*"
FILES_${PN}16 = "${libdir}/libpng16.so.*"
FILES_${PN}16-dev = "${libdir}/libpng12.* ${includedir}/libpng12 ${libdir}/pkgconfig/libpng16.pc"
FILES_${PN} = "${libdir}/lib*.so.*"
FILES_${PN}-dev += " ${bindir} ${sbindir}"
