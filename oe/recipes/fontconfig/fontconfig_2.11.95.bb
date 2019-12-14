DESCRIPTION = "A library for configuring and customizing font access."
SECTION = "libs"
LICENSE = "BSD"
DEPENDS = "freetype zlib libxml2 expat"

SRC_URI = "http://www.freedesktop.org/software/fontconfig/release/${BPN}-${PV}.tar.gz \
          file://revert-static-pkgconfig.patch \
          "
SRC_URI[md5sum] = "da605e910d9c037f8886d65607b06920"
SRC_URI[sha256sum] = "39da7704b348b3c9c83f449e9aa6e0e131ffe77e3533b68017f3b40a95d75a9c"

inherit autotools pkgconfig

PACKAGES =+ "fontconfig-utils"
FILES_${PN} =+ "${datadir}/xml/*"
FILES_fontconfig-utils = "${bindir}/*"

# Work around past breakage in debian.bbclass
RPROVIDES_fontconfig-utils = "libfontconfig-utils"
RREPLACES_fontconfig-utils = "libfontconfig-utils"
RCONFLICTS_fontconfig-utils = "libfontconfig-utils"
DEBIAN_NOAUTONAME_fontconfig-utils = "1"

inherit autotools pkgconfig

FONTCONFIG_CACHE_DIR ?= "${localstatedir}/cache/fontconfig"

# comma separated list of additional directories
# /usr/share/fonts is already included by default (you can change it with --with-default-fonts)
FONTCONFIG_FONT_DIRS ?= "no"

EXTRA_OECONF = " --disable-docs --with-default-fonts=${datadir}/fonts --with-cache-dir=${FONTCONFIG_CACHE_DIR} --with-add-fonts=${FONTCONFIG_FONT_DIRS}"
