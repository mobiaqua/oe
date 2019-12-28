DESCRIPTION = "Simple program to init SGX services."
PR = "r0"

DEPENDS = "omap4-sgx-libs"

SRC_URI = "file://pvrsrvinit.c"
S = "${WORKDIR}"

do_compile() {
	${CC} -o pvrsrvinit pvrsrvinit.c ${CFLAGS} ${LDFLAGS} -lsrv_init
}

do_install() {
	install -d ${D}${bindir}
	install pvrsrvinit ${D}${bindir}
}
