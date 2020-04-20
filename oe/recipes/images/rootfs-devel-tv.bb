require rootfs-release-tv.bb

require common-debug-tools.inc

INSTALL_PKGS += "mpv mplayer"
DEPENDS += ""
RDEPENDS += ""
RRECOMMENDS += ""

IMAGE_BASENAME = "rootfs-devel-tv"
