DESCRIPTION = "<description>"

PV = "1.0"
PR = "r0"

DEPENDS_append=" virtual/kernel"

INSTALL_PKGS = "\
	base-files base-passwd busybox kernel-modules sysvinit sysvinit-pidof netbase modutils-initscripts \
	tinylogin initscripts update-alternatives alsa-utils alsa-state alsa-utils-alsamixer \
	glibc-gconv-ibm850 glibc-gconv-cp1250 module-init-tools-depmod ifupdown modutils-collateral \
	dropbear nfs-utils-client dbus bluez4 util-linux libstdc++ libgcc iw \
"

RRECOMMENDS += ""
RDEPENDS += "ntpdate tzdata tzdata-europe nfs-utils-client"

IMAGE_LINGUAS = ""
IMAGE_INSTALL += "${INSTALL_PKGS} "

IMAGE_PREPROCESS_COMMAND = "create_etc_timestamp"

inherit image
