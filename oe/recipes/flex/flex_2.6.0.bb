SUMMARY = "Flex (The Fast Lexical Analyzer)"
DESCRIPTION = "Flex is a fast lexical analyser generator.  Flex is a tool for generating programs that recognize \
lexical patterns in text."
HOMEPAGE = "http://sourceforge.net/projects/flex/"
SECTION = "devel"
LICENSE = "BSD"

DEPENDS = ""
DEPENDS_class-native = "m4-native"

BBCLASSEXTEND = "native nativesdk"

LIC_FILES_CHKSUM = "file://COPYING;md5=e4742cf92e89040b39486a6219b68067"

SRC_URI = "${SOURCEFORGE_MIRROR}/flex/flex-${PV}.tar.bz2 \
           file://run-ptest \
           file://do_not_create_pdf_doc.patch \
           file://0001-tests-add-a-target-for-building-tests-without-runnin.patch \
           file://0002-avoid-c-comments-in-c-code-fails-with-gcc-6.patch \
           file://CVE-2016-6354.patch \
           "

SRC_URI[md5sum] = "266270f13c48ed043d95648075084d59"
SRC_URI[sha256sum] = "24e611ef5a4703a191012f80c1027dc9d12555183ce0ecd46f3636e587e9b8e9"

inherit autotools gettext

M4 = "${bindir}/m4"
M4_class-native = "${STAGING_BINDIR_NATIVE}/m4"
EXTRA_OECONF += "ac_cv_path_M4=${M4}"
EXTRA_OEMAKE += "m4=${STAGING_BINDIR_NATIVE}/m4"

do_install_append_class-native() {
	create_wrapper ${D}/${bindir}/flex M4=${M4}
}

do_install_append_class-nativesdk() {
	create_wrapper ${D}/${bindir}/flex M4=${M4}
}
