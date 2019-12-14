DESCRIPTION = "libass is a portable subtitle renderer for the ASS/SSA (Advanced Substation Alpha/Substation Alpha) subtitle format. It is mostly compatible with VSFilter."
HOMEPAGE = "https://github.com/libass/libass"
SECTION = "libs/multimedia"

LICENSE = "ISC"

DEPENDS = "fontconfig freetype libpng fribidi"

SRC_URI = "git://github.com/libass/libass.git;protocol=git"
SRCREV = "73284b676b12b47e17af2ef1b430527299e10c17"
S = "${WORKDIR}/git"

inherit autotools pkgconfig

EXTRA_OECONF = " \
    --enable-fontconfig \
    --disable-harfbuzz \
"

PACKAGES =+ "${PN}-tests"

FILES_${PN}-tests = " \
    ${libdir}/test/test \
"
