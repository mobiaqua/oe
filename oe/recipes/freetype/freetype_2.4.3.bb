DESCRIPTION = "Freetype font rendering library"
SECTION = "libs"
LICENSE = "freetype GPLv2"
PR = "r2"
DEPENDS = "zlib"

SRC_URI = "\
  ${SOURCEFORGE_MIRROR}/freetype/freetype-${PV}.tar.bz2 \
  file://no-hardcode.patch \
  file://libtool-tag.patch \
"
S = "${WORKDIR}/freetype-${PV}"

SRC_URI[md5sum] = "75ac7082bde7b3805dc5d6bc806fa045"
SRC_URI[sha256sum] = "b4e626db62fd1b4549ff5d57f5eca3a41631fd6066adf8a31c11879b51249afc"

inherit autotools pkgconfig binconfig

LIBTOOL = "${S}/builds/unix/${HOST_SYS}-libtool"
EXTRA_OEMAKE = "'LIBTOOL=${LIBTOOL}'"

LDFLAGS_append = " -Wl,-rpath-link -Wl,${STAGING_DIR_TARGET}${libdir}"

do_configure() {
	cd builds/unix
	rm ${S}/builds/unix/ltmain.sh
	rm ${S}/builds/unix/aclocal.m4
	rm ${S}/builds/unix/configure
	rm ${S}/builds/unix/config.*
	libtoolize --force --copy
	gnu-configize --force
	aclocal -I .
	autoconf
	cd ${S}
	oe_runconf
	if [ ! -f "${LIBTOOL}" ]; then
		cp ${S}/builds/unix/libtool ${LIBTOOL}
	fi
}

do_compile_prepend() {
	${BUILD_CC} -o objs/apinames src/tools/apinames.c
}

BBCLASSEXTEND = "native"

FILES_${PN} = "${libdir}/lib*.so.*"
FILES_${PN}-dev += "${bindir}"
