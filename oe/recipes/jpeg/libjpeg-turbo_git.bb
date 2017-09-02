DESCRIPTION = "libjpeg-turbo is a derivative of libjpeg that uses SIMD instructions (MMX, SSE2, NEON) to accelerate baseline JPEG compression and decompression"
HOMEPAGE = "http://libjpeg-turbo.org/"

LICENSE = "BSD-3-Clause"

PV = "8d+git${SRCREV}"

SRCREV = "e5f280c4c4821d50012de3429a67971cdd72bd7d"

SRC_URI = "git://github.com/libjpeg-turbo/libjpeg-turbo.git;protocol=git"

S = "${WORKDIR}/git"

# Drop-in replacement for jpeg
PROVIDES = "jpeg"
RPROVIDES_${PN} += "jpeg"
RREPLACES_${PN} += "jpeg"
RCONFLICTS_${PN} += "jpeg"

inherit autotools pkgconfig

EXTRA_OECONF = "--with-jpeg8 "

PACKAGES =+ "jpeg-tools libturbojpeg"

DESCRIPTION_jpeg-tools = "The jpeg-tools package includes the client programs for access libjpeg functionality.  These tools allow for the compression, decompression, transformation and display of JPEG files."
FILES_jpeg-tools = "${bindir}/*"

FILES_libturbojpeg = "${libdir}/libturbojpeg.so"
INSANE_SKIP_libturbojpeg = "dev-so"

LEAD_SONAME = "libjpeg.so.8"
