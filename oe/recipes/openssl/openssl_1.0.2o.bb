inherit pkgconfig

require openssl.inc

PR = "${INC_PR}.1"

CFLAG_linux += "-DHAVE_CRYPTODEV -DUSE_CRYPTODEV_DIGESTS -DTERMIO"
CFLAG_append_class-native = " -fPIC"

export DIRS = "crypto ssl apps engines"
export OE_LDFLAGS="${LDFLAGS}"

SRC_URI = "http://www.openssl.org/source/openssl-${PV}.tar.gz \
           file://find.pl \
           file://openssl-c_rehash.sh \
           file://configure-targets.patch \
           file://shared-libs.patch \
           file://oe-ldflags.patch \
           file://engines-install-in-libdir-ssl.patch \
           file://debian1.0.2/block_diginotar.patch \
           file://debian1.0.2/block_digicert_malaysia.patch \
           file://debian/ca.patch \
           file://debian/c_rehash-compat.patch \
           file://debian/debian-targets.patch \
           file://debian/man-dir.patch \
           file://debian/man-section.patch \
           file://debian/no-rpath.patch \
           file://debian/no-symbolic.patch \
           file://debian/pic.patch \
           file://debian1.0.2/version-script.patch \
           file://debian1.0.2/soname.patch \
           file://openssl_fix_for_x32.patch \
           file://openssl-fix-des.pod-error.patch \
           file://configure-musl-target.patch \
           file://parallel.patch \
           file://openssl-util-perlpath.pl-cwd.patch \
           file://Use-SHA256-not-MD5-as-default-digest.patch \
           file://0001-Fix-build-with-clang-using-external-assembler.patch \
           file://0001-openssl-force-soft-link-to-avoid-rare-race.patch \
           file://no-docs.patch \
           "

SRC_URI[md5sum] = "44279b8557c3247cbe324e2322ecd114"
SRC_URI[sha256sum] = "ec3f5c9714ba0fd45cb4e087301eb1336c317e0d20b575a125050470e8089e4d"

PACKAGES =+ "${PN}-engines"
FILES_${PN}-engines = "${libdir}/ssl/engines/*.so ${libdir}/engines"

do_configure_prepend() {
	cp ${WORKDIR}/find.pl ${S}/util/find.pl
}

BBCLASSEXTEND = "native"
