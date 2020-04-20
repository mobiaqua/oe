require rootfs-release-car.bb

require common-debug-tools.inc
require car-debug.inc

INSTALL_PKGS += "evtest"
DEPENDS += ""
RDEPENDS += ""
RRECOMMENDS += ""

IMAGE_BASENAME = "rootfs-devel-car"
