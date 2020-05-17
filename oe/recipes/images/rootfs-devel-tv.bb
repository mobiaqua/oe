require rootfs-release-tv.bb

require common-debug-tools.inc
require tv-debug.inc

INSTALL_PKGS += "omapdrmtest kmscube mplayer-mini"
DEPENDS += ""
RDEPENDS += ""
RRECOMMENDS += ""

IMAGE_BASENAME = "rootfs-devel-tv"
