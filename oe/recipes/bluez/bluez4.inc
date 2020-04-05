SUMMARY = "Linux Bluetooth Stack Userland V4"
DESCRIPTION = "Linux Bluetooth stack V4 userland components.  These include a system configurations, daemons, tools and system libraries."
HOMEPAGE = "http://www.bluez.org"
SECTION = "libs"
LICENSE = "GPLv2+ & LGPLv2.1+"
LIC_FILES_CHKSUM = "file://COPYING;md5=12f884d2ae1ff87c09e5b7ccc2c4ca7e \
                    file://COPYING.LIB;md5=fb504b67c50331fc78734fed90fb0e09 \
                    file://src/main.c;beginline=1;endline=24;md5=9bc54b93cd7e17bf03f52513f39f926e \
                    file://sbc/sbc.c;beginline=1;endline=25;md5=1a40781ed30d50d8639323a184aeb191"
DEPENDS = "udev libusb dbus-glib glib-2.0 readline alsa-lib"
RDEPENDS_${PN}-dev = "bluez-hcidump"

ASNEEDED = ""

SRC_URI = "\
  ${KERNELORG_MIRROR}/bluetooth/bluez-${PV}.tar.gz \
"
S = "${WORKDIR}/bluez-${PV}"

inherit autotools

EXTRA_OECONF = "\
  --disable-gstreamer \
  --enable-usb \
  --enable-tools \
  --enable-bccmd \
  --enable-hid2hci \
  --enable-alsa \
  --disable-dfutool \
  --disable-hidd \
  --disable-pand \
  --disable-dund \
  --disable-cups \
  --disable-test \
  --enable-datafiles \
  --with-udevdir=`pkg-config --variable=udevdir udev` \
  --with-udevrulesdir=`pkg-config --variable=udevdir udev`/rules.d \
"
