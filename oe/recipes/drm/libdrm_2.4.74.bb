SUMMARY = "Userspace interface to the kernel DRM services"
DESCRIPTION = "The runtime library for accessing the kernel DRM services.  DRM \
stands for \"Direct Rendering Manager\", which is the kernel portion of the \
\"Direct Rendering Infrastructure\" (DRI).  DRI is required for many hardware \
accelerated OpenGL drivers."
HOMEPAGE = "http://dri.freedesktop.org"
SECTION = "x11/base"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://xf86drm.c;beginline=9;endline=32;md5=c8a3b961af7667c530816761e949dc71"
PROVIDES = "drm"
DEPENDS = "libpthread-stubs udev"

SRC_URI = "http://dri.freedesktop.org/libdrm/${BP}.tar.bz2 \
           file://remove-xorg-dep.patch \
          "

FILESPATHPKG =. "libdrm-${PN}:"

SRC_URI[md5sum] = "31964aa15bdea1a40c5941d4ce0962ee"
SRC_URI[sha256sum] = "d80dd5a76c401f4c8756dcccd999c63d7e0a3bad258d96a829055cfd86ef840b"

inherit autotools pkgconfig

EXTRA_OECONF += "--disable-cairo-tests \
                 --without-cunit \
                 --enable-omap-experimental-api \
                 --disable-valgrind \
                 --disable-manpages \
                 --disable-freedreno \
                 --disable-intel \
                 --disable-radeon \
                 --disable-amdgpu \
                 --disable-nouveau \
                 --disable-vmwgfx \
                 --disable-vc4 \
                "
ALLOW_EMPTY_${PN}-drivers = "1"
PACKAGES =+ "${PN}-drivers ${PN}-omap"

RRECOMMENDS_${PN}-drivers = "${PN}-omap"

FILES_${PN}-omap = "${libdir}/libdrm_omap.so.*"
FILES_${PN}-kms = "${libdir}/libkms*.so.*"
