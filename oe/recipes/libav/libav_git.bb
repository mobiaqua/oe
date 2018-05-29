SUMMARY = "A complete, cross-platform solution to record, convert and stream audio and video."
DESCRIPTION = "FFmpeg is the leading multimedia framework, able to decode, encode, transcode, \
               mux, demux, stream, filter and play pretty much anything that humans and machines \
               have created. It supports the most obscure ancient formats up to the cutting edge."
HOMEPAGE = "https://www.ffmpeg.org/"
SECTION = "libs"

LICENSE = "GPLv2+"
LICENSE_FLAGS = "commercial"

LIC_FILES_CHKSUM = "file://COPYING.GPLv2;md5=b234ee4d69f5fce4486a80fdaf4a4263 \
                    file://COPYING.GPLv3;md5=d32239bcb673463ab874e80d47fae504 \
                    file://COPYING.LGPLv2.1;md5=bd7a443320af8c812e4c18d1b79df004 \
                    file://COPYING.LGPLv3;md5=e6a600fd5e1d9cbde2d983680233ad02"

ARM_INSTRUCTION_SET = "arm"

inherit autotools pkgconfig

LEAD_SONAME = "libavcodec.so"

PACKAGES += "${PN}-vhook-dbg ${PN}-vhook"

FILES_${PN} = "${bindir}"
FILES_${PN}-dev = "${includedir}/${PN}"

FILES_${PN}-vhook = "${libdir}/vhook"
FILES_${PN}-vhook-dbg += "${libdir}/vhook/.debug"

PACKAGES += "libav-x264-presets \
             libavcodec  libavcodec-dev  libavcodec-dbg \
             libavdevice libavdevice-dev libavdevice-dbg \
             libavformat libavformat-dev libavformat-dbg \
             libavresample libavresample-dev libavresample-dbg \
             libavutil   libavutil-dev   libavutil-dbg \
             libpostproc libpostproc-dev libpostproc-dbg \
             libswscale  libswscale-dev  libswscale-dbg \
             libavfilter libavfilter-dev libavfilter-dbg \
            "

FILES_libav-x264-presets = "${datadir}/*.ffpreset"

FILES_${PN}-dev = "${includedir}"
FILES_libavcodec = "${libdir}/libavcodec*.so.*"
FILES_libavcodec-dev = "${libdir}/libavcodec*.so ${libdir}/pkgconfig/libavcodec.pc ${libdir}/libavcodec*.a"
FILES_libavcodec-dbg += "${libdir}/.debug/libavcodec*"

FILES_libavdevice = "${libdir}/libavdevice*.so.*"
FILES_libavdevice-dev = "${libdir}/libavdevice*.so ${libdir}/pkgconfig/libavdevice.pc ${libdir}/libavdevice*.a"
FILES_libavdevice-dbg += "${libdir}/.debug/libavdevice*"

FILES_libavformat = "${libdir}/libavformat*.so.*"
FILES_libavformat-dev = "${libdir}/libavformat*.so ${libdir}/pkgconfig/libavformat.pc ${libdir}/libavformat*.a"
FILES_libavformat-dbg += "${libdir}/.debug/libavformat*"

FILES_libavresample = "${libdir}/libavresample*.so.*"
FILES_libavresample-dev = "${libdir}/libavresample*.so ${libdir}/pkgconfig/libavresample.pc ${libdir}/libavresample*.a"
FILES_libavresample-dbg += "${libdir}/.debug/libavresample*"

FILES_libavutil = "${libdir}/libavutil*.so.*"
FILES_libavutil-dev = "${libdir}/libavutil*.so ${libdir}/pkgconfig/libavutil.pc ${libdir}/libavutil*.a"
FILES_libavutil-dbg += "${libdir}/.debug/libavutil*"

FILES_libpostproc = "${libdir}/libpostproc*.so.*"
FILES_libpostproc-dev = "${libdir}/libpostproc*.so  ${libdir}/pkgconfig/libpostproc.pc ${libdir}/libpostproc*.a ${includedir}/postproc"
FILES_libpostproc-dbg += "${libdir}/.debug/libpostproc*"

FILES_libswscale = "${libdir}/libswscale*.so.*"
FILES_libswscale-dev = "${libdir}/libswscale*.so ${libdir}/pkgconfig/libswscale.pc ${libdir}/libswscale*.a"
FILES_libswscale-dbg += "${libdir}/.debug/libswscale*"

FILES_libavfilter = "${libdir}/libavfilter*.so.*"
FILES_libavfilter-dev = "${libdir}/libavfilter*.so ${libdir}/pkgconfig/libavfilter.pc ${libdir}/libavfilter*.a"
FILES_libavfilter-dbg += "${libdir}/.debug/libavfilter*"

DEPENDS_i586 += "nasm-native"
DEPENDS_i686 += "nasm-native"

SRCREV = "d89eea3455862a97963ac0701c51d9a1bdd21f19"
SRCPV = "4.0"

PV = "git+stable+r${SRCPV}"
PR = "r1"

SRC_URI = "git://source.ffmpeg.org/ffmpeg.git;protocol=git;branch=release/${SRCPV} \
          "

S = "${WORKDIR}/git"
B = "${S}/build.${HOST_SYS}.${TARGET_SYS}"

FULL_OPTIMIZATION_armv7a = "-fexpensive-optimizations -fno-tree-vectorize -fomit-frame-pointer -O4 -ffast-math"
BUILD_OPTIMIZATION = "${FULL_OPTIMIZATION}"

EXTRA_FFCONF_board-tv = "--cpu=cortex-a9"
EXTRA_FFCONF_igep0030 = "--cpu=cortex-a8"
EXTRA_FFCONF ?= ""

EXTRA_OECONF = " \
        --enable-shared \
        --enable-pthreads \
        --enable-gpl \
        \
        --cross-prefix=${TARGET_PREFIX} \
        --prefix=${prefix} \
        \
        --disable-all \
        --disable-bzlib \
        --enable-avcodec \
        --enable-avformat \
        --enable-avutil \
        --enable-swscale \
        --enable-avresample \
        --enable-protocol=file \
        --enable-bsf=mov2textsub,h264_mp4toannexb,hevc_mp4toannexb,mpeg4_unpack_bframes \
        --enable-demuxer=matroska,mov,flac,mp3,wav,mpegps,mpegts,avi,m4v,mpegvideo,asf,flv,rm,rtmp,swf,srt,ass \
        --enable-parser=aac,ac3,dca,flac,h261,h263,h264,hevc,mjpeg,mpegaudio,mpegvideo,mpeg4video,rv30,rv40,vc1,vorbis,\
vp3,vp8,vp9\
        --enable-decoder=aac,ac3,aic,eac3,dca,alac,als,flac,flv,h261,h263,h263i,h263p,h264,hevc,mlp,mp1,mp1float,mp2,mp2float,\
mp3,mp3adu,mp3adufloat,mp3float,mp3on4,mp3on4float,mpegvideo,mpeg1video,mpeg2video,mpeg4,msmpeg4v1,msmpeg4v2,msmpeg4v3,mjpeg,\
ffv1,dvvideo,ralf,rv10,rv20,rv30,rv40,svq1,sqv3,truehd,vc1,vorbis,vp3,vp5,vp6,vp6a,vp6f,vp7,vp8,vp9,webp,wmapro,wmav1,wmav2,\
wmalossless,wmv1,wmv2,wmv3,theora,ra_144,ra_288,pcm_u16le,pcm_u16be,pcm_s16le,pcm_s16be,pcm_u24le,pcm_u24be,pcm_s24le,\
pcm_s24be,pcm_u32le,pcm_u32be,pcm_s32le,pcm_s32be,pcm_bluray,pcm_dvd \
        --arch=${TARGET_ARCH} \
        --target-os="linux" \
        --enable-cross-compile \
        --extra-cflags="${TARGET_CFLAGS} ${HOST_CC_ARCH}${TOOLCHAIN_OPTIONS}" \
        --extra-ldflags="${TARGET_LDFLAGS}" \
        --sysroot="${STAGING_DIR_TARGET}" \
        --enable-hardcoded-tables \
        ${EXTRA_FFCONF} \
"
do_configure_prepend() {
        # We don't have TARGET_PREFIX-pkgconfig
        sed -i '/pkg_config_default="${cross_prefix}${pkg_config_default}"/d' ${S}/configure
}

do_configure() {
        mkdir -p ${B}
        cd ${B}
        #MobiAqua: on mac os x with llvm compiler use clang instead
        if gcc --version | grep llvm-gcc > /dev/null ; then
                ${S}/configure ${EXTRA_OECONF} --host-cc=clang
        else
                ${S}/configure ${EXTRA_OECONF}
        fi
        sed -i -e s:Os:O4:g ${B}/config.h
}
