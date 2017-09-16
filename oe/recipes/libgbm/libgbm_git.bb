
inherit autotools lib_package

PV = "9.0.0"
PR = "r0"
PR_append = "+gitr-${SRCREV}"

DEPENDS = "udev libdrm"

SRCREV = "501ff0a00649b339f063f7b88c2477fd6ea9bff4"
SRC_URI = "git://github.com/mobiaqua/libgbm.git;protocol=git \
           file://0001-update-pkgconfig-file.patch \
           file://0002-Update-to-libudev.pc-name.patch \
           file://0003-Makefile.am-add-version-info.patch \
           file://remove-not-used-backends.patch \
           file://fix-MAX_PATH.patch \
"

S = "${WORKDIR}/git"
