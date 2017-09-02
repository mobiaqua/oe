require libmpc.inc

DEPENDS = "gmp mpfr"
S = "${WORKDIR}/mpc-${PV}"
NATIVE_INSTALL_WORKS = "1"
BBCLASSEXTEND = "native"
PR = "r1"

SRC_URI = "http://www.multiprecision.org/mpc/download/mpc-${PV}.tar.gz"

EXTRA_OECONF_append_virtclass-native = " --enable-static"
SRC_URI[md5sum] = "0d6acab8d214bd7d1fbbc593e83dd00d"
SRC_URI[sha256sum] = "fd3efe422f0d454592059e80f2c00d1a2e381bf2beda424c5094abd4deb049ac"
