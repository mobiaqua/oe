DESCRIPTION = "open source processor emulator"
HOMEPAGE = "http://qemu.org"
LICENSE = "GPLv2 & LGPLv2.1"
DEPENDS = "glib-2.0-native zlib-native"

inherit autotools
inherit native

SRC_URI = "http://wiki.qemu.org/download/qemu-${PV}.tar.bz2 \
    file://fix-configure-checks.patch \
    file://larger_default_ram_size.patch \
    "

SRC_URI[md5sum] = "b6f3265b8ed39d77e8f354f35cc26e16"
SRC_URI[sha256sum] = "b22b30ee9712568dfb4eedf76783f4a76546e1cbc41659b909646bcf0b4867bb"

S = "${WORKDIR}/qemu-${PV}"

EXTRA_OECONF="--target-list=i386-softmmu --disable-sdl --disable-gtk --disable-virtfs --disable-vnc \
--disable-cocoa --audio-drv-list= --disable-xen --disable-brlapi --disable-curl --disable-curses \
--disable-fdt --disable-bluez --disable-slirp --disable-kvm --disable-nptl --disable-vde \
--disable-linux-aio --disable-cap-ng --disable-attr --disable-blobs --disable-docs --disable-vhost-net \
--disable-spice --disable-libiscsi --disable-smartcard-nss --disable-libusb --disable-usb-redir \
--disable-guest-agent --disable-seccomp --disable-glusterfs --disable-libssh2 \
"

do_configure() {
	export CPP=gcc
	export CPP=g++
	export PKG_CONFIG=host-pkg-config
	${S}/configure --prefix=${prefix} --sysconfdir=${sysconfdir} --libexecdir=${libexecdir} ${EXTRA_OECONF}
}

INSANE_SKIP_${PN} = "arch"
