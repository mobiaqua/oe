SUMMARY = "Open Source multimedia player"
DESCRIPTION = "mpv is a fork of mplayer2 and MPlayer. It shares some features with the former projects while introducing many more."
SECTION = "multimedia"
HOMEPAGE = "http://www.mpv.io/"
DEPENDS = "zlib ffmpeg jpeg libdrm libass lua"

LICENSE = "GPLv2+"

PV = "0.32+git${SRCPV}"
PR = "r1"

SRCREV = "cae2ffb6eb52f56167aeabf40caa28ecb3ca498b"
SRC_URI = "git://github.com/mpv-player/mpv.git;protocol=git;name=mpv \
           http://www.freehackers.org/~tnagy/release/waf-2.0.9;name=waf;downloadfilename=waf;subdir=git \
           file://yuv420_to_nv12.S \
           file://vo_omap_drm.c \
           file://waf_enable_asm.patch \
           file://add-level-to-sh.patch \
           file://visible_dimensions.patch \
           file://silence-config-option.patch \
           file://remove_vo_drivers.patch \
           file://remove_demuxers.patch \
           file://find_gbm.patch \
           file://omap_drm.patch \
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
    --disable-drm \
    --disable-gl \
    --enable-gbm \
    --enable-omap-drm \
    --enable-lua \
"

addtask fixwaf before do_configure after do_patch

do_fixwaf() {
    mv ${S}/waf-2.0.9 ${S}/waf
    chmod +x ${S}/waf
}

do_configure_prepend() {
    cp ${WORKDIR}/vo_omap_drm.c ${S}/video/out/
    cp ${WORKDIR}/yuv420_to_nv12.S ${S}/video/out/
}

FILES_${PN} += "${datadir}/icons"

DEBUG_BUILD = "${@['no','yes'][bb.data.getVar('BUILD_DEBUG', d, 1) == '1']}"

do_rm_work() {
        if [ "${DEBUG_BUILD}" == "no" ]; then
                cd ${WORKDIR}
                for dir in *
                do
                        if [ `basename ${dir}` = "temp" ]; then
                                echo "Not removing temp"
                        else
                                echo "Removing $dir" ; rm -rf $dir
                        fi
                done
        fi
}
