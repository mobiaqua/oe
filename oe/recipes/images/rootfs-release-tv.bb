
require rootfs-base.inc

DESCRIPTION = "<description>"

PV = "1.0.0"
PR = "r0"

INSTALL_PKGS += "u-boot-pandaboard omap4-sgx-libs omap4-sgx-modules"

RDEPENDS += ""
RRECOMMENDS += ""

IMAGE_BASENAME = "rootfs-release-tv"
IMAGE_INSTALL += "${INSTALL_PKGS} "

ROOTFS_POSTPROCESS_COMMAND += "rm -f ${IMAGE_ROOTFS}/boot/*;${MA_ROOTFS_POSTPROCESS};"
