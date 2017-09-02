
require rootfs-base.inc

DESCRIPTION = "<description>"

PV = "1.0.0"
PR = "r0"

INSTALL_PKGS += "x-loader-igep-mlo igep-tools writeloader"

RDEPENDS += "kernel-module-mailbox kernel-module-bridgedriver"
RRECOMMENDS += ""

IMAGE_BASENAME = "rootfs-release-car"
IMAGE_INSTALL += "${INSTALL_PKGS} "

ROOTFS_POSTPROCESS_COMMAND += "rm -f ${IMAGE_ROOTFS}/boot/*;${MA_ROOTFS_POSTPROCESS};"
