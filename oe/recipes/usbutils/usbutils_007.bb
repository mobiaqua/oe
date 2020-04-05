SUMMARY = "Host side USB console utilities"
DESCRIPTION = "Contains the lsusb utility for inspecting the devices connected to the USB bus."
HOMEPAGE = "http://www.linux-usb.org"
SECTION = "base"

LICENSE = "GPLv2+ & (GPLv2 | GPLv3)"
# License files went missing in 010, when 011 is released add LICENSES/* back
LIC_FILES_CHKSUM = "file://lsusb.c;endline=1;md5=7d4861d978ff5ba7cb2b319ed1d4afe3 \
                    file://lsusb.py.in;beginline=2;endline=2;md5=194d6a0226bf90f4f683e8968878b6cd"

DEPENDS = "libusb1 virtual/libiconv"

SRC_URI = "${KERNELORG_MIRROR}/linux/utils/usb/usbutils/usbutils-${PV}.tar.gz \
          "
SRC_URI[md5sum] = "be6c42294be5c940f208190d3479d50c"
SRC_URI[sha256sum] = "e65c234cadf7c81b6b1567c440e3b9b31b44f51c27df3e45741b88848d8b37d3"

inherit autotools pkgconfig

EXTRA_OECONF = "--program-prefix="
sbindir = "${base_sbindir}"
bindir = "${base_bindir}"

PACKAGES =+ "${PN}-update"

FILES_${PN} += "${datadir}/usb*"
FILES_${PN}-update = "${sbindir}/update-usbids.sh"

do_configure_prepend() {
    rm -rf ${S}/libusb
}

do_install_append() {
    # The 0.86 Makefile.am installs both usb.ids and usb.ids.gz.
    if [ -f ${D}${datadir}/usb.ids.gz ]
    then
        rm -f ${D}${datadir}/usb.ids
    fi
}
