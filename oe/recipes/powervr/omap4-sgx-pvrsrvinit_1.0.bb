DESCRIPTION = "Simple program to init SGX services."
PR = "r0"

DEPENDS = "omap4-sgx-libs"

SRC_URI = "file://pvrsrvinit.c"
S = "${WORKDIR}"

DEBUG_BUILD = "${@['no','yes'][bb.data.getVar('BUILD_DEBUG', d, 1) == '1']}"

do_compile() {
	${CC} -o pvrsrvinit pvrsrvinit.c ${CFLAGS} ${LDFLAGS} -g -lsrv_init
}

do_install() {
	install -d ${D}${bindir}
	install pvrsrvinit ${D}${bindir}
}

do_rm_work() {
	if [ "${DEBUG_BUILD}" == "no" ]; then
		cd ${WORKDIR}
		for dir in *
		do
			if [ `basename ${dir}` = "temp" ]; then
				echo "Not removing temp"
			else
				echo "Removing $dir" ; rm -rf $dir
			fi
		done
	fi
}

PACKAGE_STRIP = "no"
