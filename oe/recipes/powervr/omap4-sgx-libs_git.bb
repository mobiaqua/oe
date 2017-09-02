DESCRIPTION = "SGX540 (PowerVR) OpenGL ES 1.1, 2.0 libraries for OMAP4."
LICENSE = "proprietary-binary"

PR = "r0"
PV = "1.9.0.8.1.1"
PR_append = "+gitr-${SRCREV}"

SRCREV = "4df1d8556cf3b4a6d5f2bc156e8730714c294c28"

SRC_URI = "git://github.com/mobiaqua/pvr-omap4.git;protocol=git \
	   file://LICENSE.txt \
	   file://video_raw_update.patch \
	   file://wayland-dummy.c \
	   "

COMPATIBLE_MACHINE = "board-tv"
DEPENDS = "omap4-sgx-modules"
PROVIDES += "virtual/egl"

DEFAULT_PREFERENCE = "10"

S = "${WORKDIR}/git"

do_install_virtclass-native() {
        install -d ${D}${bindir}/
        install -m 0755 ${S}/makedevs ${D}${bindir}/
}

do_configure() {
	install -m 0644 ${WORKDIR}/wayland-dummy.c ${S}/
}

do_compile() {
	${CC} ${CFLAGS} ${LDFLAGS} ${S}/wayland-dummy.c -shared -o ${S}${libdir}/libwayland-server.so.0
}

do_install() {
	install -d ${D}${bindir}
	cp -pR ${S}${bindir}/pvrsrvinit ${D}${bindir}/

	install -d ${D}${includedir}
	cp -pR ${S}${includedir}/* ${D}${includedir}/

	install -d ${D}${libdir}
	cp -pR ${S}${libdir}/* ${D}${libdir}/
	rm -rf ${D}${libdir}/debug
	rm -rf ${D}${libdir}/xorg
	rm -f ${D}${libdir}/libpvrws_OMAPDRI2*
	rm -f ${D}${libdir}/libpvrws_WAYLAND*
	rm -f ${D}${libdir}/libwayland-egl.so*
	rm -f ${D}${libdir}/libpvr_wlegl*

	install -d ${D}/usr/share/doc/${PN}
	install -m 0666 ${WORKDIR}/LICENSE.txt ${D}/usr/share/doc/${PN}
}


INSANE_SKIP = True
INHIBIT_PACKAGE_STRIP = "1"
PACKAGE_STRIP = "no"

PACKAGES = "${PN}"

FILES_${PN} = "${bindir} ${libdir} ${datadir} ${includedir} /usr/share/doc/"
