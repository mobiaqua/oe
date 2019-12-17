DESCRIPTION = "Open Source multimedia player."
SECTION = "multimedia"
PRIORITY = "optional"
HOMEPAGE = "http://www.mplayerhq.hu/"
DEPENDS = "ffmpeg zlib libpng jpeg freetype fontconfig alsa-lib lzo libmpg123 ncurses virtual/kernel"
DEPENDS_append_board-tv = " libdce"
RDEPENDS_${PN} = "mplayer-common glibc-gconv-cp1250 ttf-dejavu-sans"

LICENSE = "GPL"

SRCREV = "38154"
SRC_URI = "svn://svn.mplayerhq.hu/mplayer;module=trunk \
	   file://makefile-nostrip-svn.patch \
	   file://mplayer-arm-pld.patch \
"
SRC_URI_append_armv7a-hf = " \
	file://yuv420_to_yuv422.S \
	file://yuv420_to_nv12.S \
	file://vo_omapfb.c \
	file://vo_omap4_v4l2.c \
	file://vd_omap4_dce.c \
	file://vd_omap4_dce.h \
#	file://omapfb.patch \
#	file://omap4.patch \
	file://vo_omap4_osd.patch \
#	file://fix_h264.patch \
#	file://fix_wmv3.patch \
	file://add-level-to-sh-video.patch \
	"

ARM_INSTRUCTION_SET = "ARM"

PV = "1.4+svnr${SRCPV}"
PR = "r1"

PARALLEL_MAKE = ""

S = "${WORKDIR}/trunk"

FILES_${PN} = "${bindir}/mplayer ${libdir}"

inherit autotools pkgconfig

# We want a kernel header for armv7a, but we don't want to make mplayer machine specific for that
STAGING_KERNEL_DIR = "${STAGING_DIR}/${MACHINE_ARCH}${TARGET_VENDOR}-${TARGET_OS}/kernel"

EXTRA_OECONF = " \
	--prefix=/usr \
	--mandir=${mandir} \
	--target=${TARGET_SYS} \
	\
	--disable-mencoder \
	--disable-gui \
	--disable-lirc \
	--disable-lircc \
	--disable-joystick \
	--disable-vm \
	--disable-xf86keysym \
	--disable-tv \
	--enable-networking \
	--enable-rtc \
	--disable-smb \
	--disable-live \
	--disable-dvdnav \
	--disable-dvdread \
	--disable-cdparanoia \
	--enable-freetype \
	--disable-menu \
	--enable-sortsub \
	--disable-fribidi \
	--disable-enca \
	--disable-ftp \
	--disable-vstream \
	\
	--disable-gif \
	--disable-png \
	--disable-jpeg \
	--disable-libcdio \
	--disable-liblzo \
	--disable-qtx \
	--disable-xanim \
	--disable-real \
	--disable-xvid \
	--disable-x264 \
	\
	--disable-ffmpeg_so \
	\
	--disable-speex \
	--disable-theora \
	--disable-faac \
	--disable-faad \
	--disable-ladspa \
	--disable-libdv \
	--disable-mad \
	--disable-toolame \
	--disable-twolame \
	--disable-xmms \
	--enable-mpg123 \
	--disable-libmpeg2 \
	--disable-musepack \
	\
	--disable-gl \
	--disable-vesa \
	--disable-svga \
	--disable-sdl \
	--disable-aa \
	--disable-caca \
	--disable-ggi \
	--disable-ggiwmh \
	--disable-directx \
	--disable-dxr2 \
	--disable-dxr3 \
	--disable-dvb \
	--disable-mga \
	--disable-xmga \
	--disable-xv \
	--disable-xvmc \
	--disable-vm \
	--disable-xinerama \
	--disable-x11 \
	--disable-fbdev \
	--disable-mlib \
	--disable-3dfx \
	--disable-tdfxfb \
	--disable-s3fb \
	--disable-directfb \
	--disable-zr \
	--disable-bl \
	--disable-tdfxvid \
	--disable-tga \
	--disable-pnm \
	--disable-md5sum \
	--disable-xss \
	--disable-dga1 \
	--disable-dga2 \
	--disable-v4l2 \
	--disable-dvb \
	--disable-yuv4mpeg \
	--disable-vcd \
	\
	--enable-alsa \
	--disable-ossaudio \
	--disable-arts \
	--disable-esd \
	--disable-pulse \
	--disable-jack \
	--disable-openal \
	--disable-nas \
	--disable-sgiaudio \
	--disable-sunaudio \
	--disable-win32waveout \
	--disable-encoder=vorbis_encoder \
	--enable-select \
	--ar=${TARGET_PREFIX}ar \
	\
	--enable-protocol='file_protocol' \
"

EXTRA_OECONF_append_armv6 = " --enable-armv6"
EXTRA_OECONF_append_armv7a-hf = " --enable-armv6 --enable-neon"

#EXTRA_OECONF_append_board-tv = " --enable-omap4"
EXTRA_OECONF_append_board-tv = " --enable-fbdev"
EXTRA_OECONF_append_igep0030 = " --enable-omapfb"

FULL_OPTIMIZATION = "-fexpensive-optimizations -fomit-frame-pointer -frename-registers -O4 -ffast-math"
FULL_OPTIMIZATION_armv7a-hf = "-fno-tree-vectorize -fomit-frame-pointer -O4 -frename-registers -ffast-math"
BUILD_OPTIMIZATION = "${FULL_OPTIMIZATION}"

do_configure_prepend_armv7a-hf() {
	cp ${WORKDIR}/yuv420_to_yuv422.S ${S}/libvo
	cp ${WORKDIR}/yuv420_to_nv12.S ${S}/libvo
	cp ${WORKDIR}/vo_omapfb.c ${S}/libvo
	cp ${WORKDIR}/vo_omap4_v4l2.c ${S}/libvo
	cp ${WORKDIR}/vd_omap4_dce.c ${S}/libmpcodecs
	cp ${WORKDIR}/vd_omap4_dce.h ${S}/libmpcodecs
	cp ${STAGING_INCDIR}/linux/omapfb.h ${S}/libvo/omapfb.h || true
	sed -e 's/__user//g' -i ${S}/libvo/omapfb.h || true
	export DCE_CFLAGS=`pkg-config --cflags libdce`
	export DCE_LIBS=`pkg-config --libs libdce`
}

do_configure() {
	sed -i 's|/usr/include|${STAGING_INCDIR}|g' ${S}/configure
	sed -i 's|/usr/lib|${STAGING_LIBDIR}|g' ${S}/configure
	sed -i 's|/usr/\S*include[\w/]*||g' ${S}/configure
	sed -i 's|/usr/\S*lib[\w/]*||g' ${S}/configure
	sed -i 's|HOST_CC|BUILD_CC|' ${S}/Makefile

	./configure ${EXTRA_OECONF} \
		--extra-libs="-lmpg123 ${DCE_LIBS}" \
		--extra-cflags="${DCE_CFLAGS}"
}

do_compile () {
	oe_runmake
}
