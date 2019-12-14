DESCRIPTION = "Lua is a powerful light-weight programming language designed \
for extending applications."
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://doc/readme.html;beginline=318;endline=352;md5=60aa5cfdbd40086501778d9b6ebf29ee"
HOMEPAGE = "http://www.lua.org/"

SRC_URI = "http://www.lua.org/ftp/lua-${PV}.tar.gz;name=tarballsrc \
           file://lua.pc.in \
           file://0001-Allow-building-lua-without-readline-on-Linux.patch \
           "

# if no test suite matches PV release of Lua exactly, download the suite for the closest Lua release.
PV_testsuites = "5.3.4"

SRC_URI[tarballsrc.md5sum] = "efbb645e897eae37cad4344ce8b0a614"
SRC_URI[tarballsrc.sha256sum] = "3fd67de3f5ed133bf312906082fa524545c6b9e1b952e8215ffbd27113f49f00"

inherit pkgconfig binconfig

TARGET_CC_ARCH += " -fPIC ${LDFLAGS}"
EXTRA_OEMAKE = "'CC=${CC} -fPIC' 'MYCFLAGS=${CFLAGS} -fPIC' MYLDFLAGS='${LDFLAGS}' 'AR=${TARGET_SYS}-ar rcu' 'RANLIB=${TARGET_SYS}-ranlib'"

do_configure_prepend() {
    sed -i -e s:/usr/local:${prefix}:g src/luaconf.h
}

do_compile () {
    oe_runmake linux-no-readline
}

do_install () {
    oe_runmake \
        'INSTALL_TOP=${D}${prefix}' \
        'INSTALL_BIN=${D}${bindir}' \
        'INSTALL_INC=${D}${includedir}/' \
        'INSTALL_MAN=${D}${mandir}/man1' \
        'INSTALL_SHARE=${D}${datadir}/lua' \
        'INSTALL_LIB=${D}${libdir}' \
        'INSTALL_CMOD=${D}${libdir}/lua/5.2' \
        install
    install -d ${D}${libdir}/pkgconfig

    sed -e s/@VERSION@/${PV}/ ${WORKDIR}/lua.pc.in > ${WORKDIR}/lua.pc
    install -m 0644 ${WORKDIR}/lua.pc ${D}${libdir}/pkgconfig/
    rmdir ${D}${datadir}/lua/5.2
    rmdir ${D}${datadir}/lua
}

BBCLASSEXTEND = "native nativesdk"
