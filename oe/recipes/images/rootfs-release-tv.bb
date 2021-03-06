
require common-base.inc

DESCRIPTION = "<description>"

PV = "1.0.0"
PR = "r0"

INSTALL_PKGS += "u-boot-pandaboard omap4-sgx-libs omap4-sgx-modules omap4-sgx-pvrsrvinit \
                 libdce libdce-firmware libavcodec libavformat libavutil libavresample \
                 libswscale libswresample uim-sysfs wl127x-fw abefw \
"

RDEPENDS += ""
RRECOMMENDS += ""

IMAGE_BASENAME = "rootfs-release-tv"
IMAGE_INSTALL += "${INSTALL_PKGS} "

ROOTFS_POSTPROCESS_COMMAND += "rm -f ${IMAGE_ROOTFS}/boot/*;${MA_ROOTFS_POSTPROCESS};"
