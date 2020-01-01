DESCRIPTION = "SGX540 (PowerVR) OpenGL ES 1.1, 2.0 libraries for OMAP4."
LICENSE = "proprietary-binary"

PR = "r0"
PV = "1.9.6"

SRCREV = "4df1d8556cf3b4a6d5f2bc156e8730714c294c28"

SRC_URI = "git://github.com/mobiaqua/pvr-omap4.git;protocol=git \
	   file://LICENSE.txt \
	   file://wayland-dummy.c \
	   file://includes \
	   "

COMPATIBLE_MACHINE = "board-tv"
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
	install -d ${D}${includedir}
	cp -pR ${WORKDIR}/includes/* ${D}${includedir}/

	install -d ${D}${libdir}
	for i in GLESv1_CM GLESv2 IMGegl PVRScopeServices glslcompiler pvr2d srv_init srv_um usc
	do
		cp -p ${S}${libdir}/lib${i}.so.1.9.6.0 ${D}${libdir}/lib${i}.so
	done
	ln -s libGLESv1_CM.so ${D}${libdir}/libGLESv1_CM.so.1
	ln -s libGLESv2.so ${D}${libdir}/libGLESv2.so.2

	cp -p ${S}${libdir}/libwayland-server.so.0 ${D}${libdir}/

	install -d ${D}${libdir}/pkgconfig
	for i in egl glesv1_cm glesv2
	do
		cp -p ${S}${libdir}/pkgconfig/${i}.pc ${D}${libdir}/pkgconfig/
	done

	install -d ${D}/usr/share/doc/${PN}
	install -m 0666 ${WORKDIR}/LICENSE.txt ${D}/usr/share/doc/${PN}
}


INSANE_SKIP = True
INHIBIT_PACKAGE_STRIP = "1"
PACKAGE_STRIP = "no"

PACKAGES = "${PN}"

FILES_${PN} = "${bindir} ${libdir} ${datadir} ${includedir} /usr/share/doc/"
