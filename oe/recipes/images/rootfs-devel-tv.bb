
require rootfs-base.inc
require rootfs-release-tv.bb
require rootfs-tools.inc
require rootfs-tools-addons.inc

INSTALL_PKGS += "u-boot-pandaboard libdce libdce-firmware libavcodec libavformat libavutil omapdrmtest kmscube \
		libmpg123 omap4-sgx-libs omap4-sgx-modules openssh-sftp-server"

DEPENDS += "gdb-cross"
RDEPENDS += ""
RRECOMMENDS += ""

IMAGE_BASENAME = "rootfs-devel-tv"
