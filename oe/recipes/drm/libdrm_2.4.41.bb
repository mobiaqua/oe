SUMMARY = "Userspace interface to the kernel DRM services"
DESCRIPTION = "The runtime library for accessing the kernel DRM services.  DRM \
stands for \"Direct Rendering Manager\", which is the kernel portion of the \
\"Direct Rendering Infrastructure\" (DRI).  DRI is required for many hardware \
accelerated OpenGL drivers."
HOMEPAGE = "http://dri.freedesktop.org"
SECTION = "x11/base"
LICENSE = "MIT"
PROVIDES = "drm"
DEPENDS = "libpthread-stubs udev"

DEFAULT_PREFERENCE_board-tv = "10"

SRCREV = "90296021f8a70c9fab015eaf297438d8b2d64acd"
PV = "2.4.41+git${SRCREV}"

FILESPATHPKG =. "libdrm-2.4.41:"

SRC_URI = "git://github.com/mobiaqua/libdrm.git;protocol=git \
           file://01_default_perms.diff \
           file://03_build_against_librt.diff \
           file://04_libdrm-2.4.37-nouveau-1.diff \
           file://0101_omap-release-lock-also-on-error-paths.patch \
           file://0201-nouveau-use-PACKAGE_VERSION-in-libdrm_nouveau.pc.patch \
           file://0202-man-fix-manpage-build-instructions.patch \
           file://0203-radeon-Fix-1D-tiling-layout-on-SI.patch \
           file://0204-man-Fix-typo-and-use-for-make-expressions.patch \
           file://0205-intel-add-more-VLV-PCI-IDs.patch \
           file://0206-gitignore.patch \
           file://0207-Add-omapdrm-module-to-kmstest-and-vbltest.patch \
"

S = "${WORKDIR}/git"

inherit autotools pkgconfig

PACKAGES =+ "${PN}-tests ${PN}-drivers ${PN}-kms ${@base_contains('MACHINE_FEATURES', 'x86', '${PN}-intel', '',d)}"
FILES_${PN}-tests = "${bindir}/dr* ${bindir}/mode*"
FILES_${PN}-drivers = "${libdir}/libdrm_*.so.*"
FILES_${PN}-intel = "${libdir}/libdrm_intel.so.*"
FILES_${PN}-kms = "${libdir}/libkms*.so.*"

LEAD_SONAME = "libdrm.so"

EXTRA_OECONF += "--disable-cairo-tests \
                 --enable-omap-experimental-api \
                 --enable-udev \
                 --enable-libkms \
                "
EXTRA_OECONF_append = " ${@base_contains('MACHINE_FEATURES', 'x86', '', '--disable-intel --disable-radeon --disable-nouveau',d)}"
