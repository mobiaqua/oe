DESCRIPTION = "This is minimalised and modified version of UrJTAG is a universal JTAG tool"
HOMEPAGE = "http://urjtag.org/"
LICENSE = "GPLv2"
DEPENDS = "libftdi1-native libusb1-native gettext-native readline-native"

SRCREV = "497e094721970f1dd566b2593694726a4884d229"

PV = "0.10"
PR = "r1"
PR_append = "+gitr${SRCPV}"

S = "${WORKDIR}/git/urjtag-mini"

SRC_URI = "git://github.com/mobiaqua/tools;protocol=git;branch=master \
          "
inherit autotools native

# no idea why -s would make a difference but without it configure fails.
# guess the symlink is created before the actual content is there
EXTRA_AUTORECONF = "-s"

EXTRA_OECONF = "--enable-jedec-exp"

do_install () {
        oe_runmake DESTDIR=${D} MKINSTALLDIRS="${S}/tools/mkinstalldirs" install
}
