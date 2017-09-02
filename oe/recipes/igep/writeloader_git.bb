DESCRIPTION = "Writes a loader binary to a OneNAND/NAND flash memory device"
HOMEPAGE = "http://www.isee.biz"
LICENSE = "GPLv2"

SRCREV="ba825f25dae3226e3e5e4b016d66c06f532e2d96"

PV = "1.0+git${SRCPV}"
PR = "r0"

SRC_URI = "git://git.isee.biz/pub/scm/writeloader.git;protocol=git"

do_compile() {
	oe_runmake
}

do_install() {
	install -d ${D}${bindir}
	install -m 0755 ${S}/writeloader ${D}${bindir}/writeloader
}

S = "${WORKDIR}/git"
