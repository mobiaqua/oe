SUMMARY = "Open Source multimedia player"
DESCRIPTION = "mpv is a fork of mplayer2 and MPlayer. It shares some features with the former projects while introducing many more."
SECTION = "multimedia"
HOMEPAGE = "http://www.mpv.io/"
DEPENDS = "zlib ffmpeg jpeg libdrm libass lua"

LICENSE = "GPLv2+"

SRCREV = "ad2cda343b1d6001f430867e0c3f502ff27c8675"
SRC_URI = "git://github.com/mpv-player/mpv.git;protocol=git;name=mpv \
           http://www.freehackers.org/~tnagy/release/waf-2.0.9;name=waf;downloadfilename=waf;subdir=git \
          "
SRC_URI[waf.md5sum] = "3bc28bcd4868999798a6d2675211e23f"
SRC_URI[waf.sha256sum] = "2a8e0816f023995e557f79ea8940d322bec18f286917c8f9a6fa2dc3875dfa48"

S = "${WORKDIR}/git"

inherit pkgconfig waf

SIMPLE_TARGET_SYS = "${@'${TARGET_SYS}'.replace('${TARGET_VENDOR}', '')}"

EXTRA_OECONF = " \
    --prefix=${prefix} \
    --target=${SIMPLE_TARGET_SYS} \
    --confdir=${sysconfdir} \
    --datadir=${datadir} \
    --disable-manpage-build \
    --disable-gl \
    --disable-libsmbclient \
    --disable-libbluray \
    --disable-dvdnav \
    --disable-cdda \
    --disable-uchardet \
    --disable-rubberband \
    --disable-lcms2 \
    --disable-vapoursynth \
    --disable-jack \
    --disable-wayland \
    --disable-libarchive \
    --disable-vaapi \
    --disable-vdpau \
    --disable-gbm \
    --enable-drm \
    --enable-libass \
    --enable-lua \
"

addtask fixwaf before do_configure after do_patch

do_fixwaf() {
    mv ${S}/waf-2.0.9 ${S}/waf
    chmod +x ${S}/waf
}

FILES_${PN} += "${datadir}/icons"
