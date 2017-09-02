DESCRIPTION = "Suite of shell scripts and a set of helper programs for \
IGEP-technology based boards"
HOMEPAGE = "http://www.isee.biz"
LICENSE = "GPLv2"

SRCREV = "bc6a5915ef8a09aabd60456a5fffc20848b73ee4"

PV = "1.3"
PR = "r1"

inherit update-rc.d

INITSCRIPT_NAME = "igep-tools.sh"
INITSCRIPT_PARAMS = "start 98 S ."

SRC_URI = "git://git.isee.biz/pub/scm/igep-tools.git;protocol=git"

do_compile() {
	:
}

do_install() {
	install -d ${D}/lib/igep-tools
	install -m 0755 ${S}/scripts/e-functions ${D}/lib/igep-tools/e-functions
	install -d ${D}${bindir}
	install -m 0755 ${S}/scripts/igep-flash ${D}${bindir}
	install -m 0755 ${S}/scripts/igep-media-create ${D}${bindir}
	install -d ${D}${sysconfdir}/init.d
	install -m 0755 ${S}/init/rc.init ${D}${sysconfdir}/init.d/igep-tools.sh
}

S = "${WORKDIR}/git"

FILES_${PN} += "/lib"
