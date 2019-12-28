DESCRIPTION = "Simple EGL wrapper for SGX library."
PR = "r0"

DEPENDS = "omap4-sgx-libs"

SRC_URI = "file://egl-wrapper.c"
S = "${WORKDIR}"

do_compile() {
	${CC} ${CFLAGS} ${LDFLAGS} ${S}/egl-wrapper.c -shared -o ${S}/libEGL.so
}

do_install() {
	install -d ${D}${libdir}
	cp -p ${S}/libEGL.so ${D}${libdir}/
	ln -s libEGL.so ${D}${libdir}/libEGL.so.1
}
