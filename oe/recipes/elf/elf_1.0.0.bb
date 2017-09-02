DESCRIPTION = "elf include"
LICENSE = "GPL"
SECTION = "base"
PRIORITY = "required"
PR = "r1"

SRC_URI = "file://elf.h"

do_configure() {
	:
}

do_compile() {
	:
}

do_install_virtclass-native() {
	install -d ${D}${includedir}/
	install -m 0644 ${WORKDIR}/elf.h ${D}${includedir}/
}

do_install() {
	:
}

BBCLASSEXTEND = "native"

NATIVE_INSTALL_WORKS = "1"
