require rootfs-release-tv.bb

require common-debug-tools.inc
require tv-debug.inc

INSTALL_PKGS += "omapdrmtest kmscube mpv mplayer"
DEPENDS += ""
RDEPENDS += ""
RRECOMMENDS += ""

IMAGE_BASENAME = "rootfs-devel-tv"
