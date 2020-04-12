DESCRIPTION = "Open Source multimedia player."
SECTION = "multimedia"
PRIORITY = "optional"
HOMEPAGE = "http://www.mplayerhq.hu/"
DEPENDS = "ffmpeg zlib libpng jpeg freetype fontconfig alsa-lib lzo libmpg123 ncurses virtual/kernel"
DEPENDS_append_board-tv = " libdce libdrm libgbm virtual/egl"
RDEPENDS_${PN} = "mplayer-common glibc-gconv-cp1250 ttf-dejavu-sans"

LICENSE = "GPL"

SRCREV = "38184"
SRC_URI = "svn://svn.mplayerhq.hu/mplayer;module=trunk \
	   file://makefile-nostrip-svn.patch \
	   file://mplayer-arm-pld.patch \
"
SRC_URI_append_armv7a-hf = " \
	file://yuv420_to_nv12.S \
	file://vo_omap_drm_egl.c \
	file://vo_omap_drm_egl.h \
	file://vd_omap_dce.c \
	file://add-level-to-sh-video.patch \
	file://fix_h264.patch \
	file://fix_xvid.patch \
	file://fix_wmv3.patch \
	file://omap_drm_egl.patch \
	file://omap_dce.patch \
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

EXTRA_OECONF_append_board-tv = " --enable-fbdev --enable-omapdrmegl --enable-omapdce"

FULL_OPTIMIZATION = "-fexpensive-optimizations -fomit-frame-pointer -frename-registers -O4 -ffast-math"
FULL_OPTIMIZATION_armv7a-hf = "-fno-tree-vectorize -fomit-frame-pointer -O4 -frename-registers -ffast-math"
BUILD_OPTIMIZATION = "${FULL_OPTIMIZATION}"

do_configure_prepend_armv7a-hf() {
	cp ${WORKDIR}/yuv420_to_nv12.S ${S}/libvo
	cp ${WORKDIR}/vo_omap_drm_egl.c ${S}/libvo
	cp ${WORKDIR}/vo_omap_drm_egl.h ${S}/libvo
	cp ${WORKDIR}/vd_omap_dce.c ${S}/libmpcodecs
	export DCE_CFLAGS=`pkg-config --cflags libdce`
	export DCE_LIBS=`pkg-config --libs libdce`
	export DRM_CFLAGS=`pkg-config --cflags libdrm`
	export DRM_LIBS=`pkg-config --libs libdrm`
	export GBM_CFLAGS=`pkg-config --cflags gbm`
	export GBM_LIBS=`pkg-config --libs gbm`
	export EGL_CFLAGS=`pkg-config --cflags egl`
	export EGL_LIBS=`pkg-config --libs egl`
}

do_configure() {
	sed -i 's|/usr/include|${STAGING_INCDIR}|g' ${S}/configure
	sed -i 's|/usr/lib|${STAGING_LIBDIR}|g' ${S}/configure
	sed -i 's|/usr/\S*include[\w/]*||g' ${S}/configure
	sed -i 's|/usr/\S*lib[\w/]*||g' ${S}/configure
	sed -i 's|HOST_CC|BUILD_CC|' ${S}/Makefile

	./configure ${EXTRA_OECONF} \
		--extra-libs="-lmpg123 ${DCE_LIBS} ${DRM_LIBS} ${GBM_LIBS} ${EGL_LIBS} -lGLESv2" \
		--extra-cflags="${DCE_CFLAGS} ${DRM_CFLAGS} ${GBM_CFLAGS} ${EGL_CFLAGS}"
}

do_compile () {
	oe_runmake
}
