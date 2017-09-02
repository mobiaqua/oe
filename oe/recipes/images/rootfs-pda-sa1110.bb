
require rootfs-base.inc
require rootfs-tools.inc

DESCRIPTION = "<description>"

PV = "1.0.0"
PR = "r0"

INSTALL_PKGS += ""

DEPENDS +="gdb-cross urjtag-mini-native u-boot-pda-h3600"
RDEPENDS += "kernel-image"
RRECOMMENDS += ""

IMAGE_BASENAME = "rootfs-pda-sa1110"
IMAGE_INSTALL += "${INSTALL_PKGS} "

ROOTFS_POSTPROCESS_COMMAND += "rm -f ${IMAGE_ROOTFS}/boot/*;${MA_ROOTFS_POSTPROCESS};"
