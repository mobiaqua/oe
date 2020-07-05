DESCRIPTION = "TI MM RPC"

LICENSE = "BSD"

PV = "1.0"
INC_PR = "r0"

DEPENDS += "virtual/kernel"

SRCREV = "cc633bd24ea64eafcd51189ee1606303ee7da625"

SRC_URI = "git://github.com/mobiaqua/ti-libmmrpc.git;protocol=git"

S = "${WORKDIR}/git"

inherit pkgconfig autotools

STAGING_KERNEL_DIR = "${STAGING_DIR}/${MACHINE_ARCH}${TARGET_VENDOR}-${TARGET_OS}/kernel"

EXTRA_OECONF += "KERNEL_INSTALL_DIR=${STAGING_KERNEL_DIR}"

do_configure() {
    ( cd ${S}; autoreconf -f -i -s )
    oe_runconf
}
