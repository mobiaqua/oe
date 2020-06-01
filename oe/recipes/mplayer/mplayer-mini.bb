DESCRIPTION = "Open Source multimedia player."
SECTION = "multimedia"
PRIORITY = "optional"
HOMEPAGE = "http://www.mplayerhq.hu/"
DEPENDS = "ffmpeg zlib freetype fontconfig alsa-lib libmpg123 ncurses"
DEPENDS_append_board-tv = " libdce libdrm libgbm virtual/egl"
RDEPENDS_${PN} = "mplayer-common glibc-gconv-cp1250 ttf-dejavu-sans"

LICENSE = "GPL"

RCONFLICTS_${PN} = "mplayer"

SRCREV = "1e24be806c955a7814ad07b6191539111a0328d3"
SRC_URI = "git://github.com/mobiaqua/mplayer-mini.git;protocol=git"

ARM_INSTRUCTION_SET = "ARM"

PV = "1.0+git"
PR = "r1"

S = "${WORKDIR}/git/src"

FILES_${PN} = "${bindir}/mplayer"

inherit autotools pkgconfig

EXTRA_OECONF = " \
	--prefix=/usr \
	--mandir=${mandir} \
	--target=${TARGET_SYS} \
	--ar=${TARGET_PREFIX}ar \
"

FULL_OPTIMIZATION = "-fexpensive-optimizations -fomit-frame-pointer -frename-registers -O4 -ffast-math"
FULL_OPTIMIZATION_armv7a-hf = "-fno-tree-vectorize -fomit-frame-pointer -O4 -frename-registers -ffast-math"
BUILD_OPTIMIZATION = "${FULL_OPTIMIZATION}"

do_configure_prepend_armv7a-hf() {
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

	./configure ${EXTRA_OECONF} \
		--extra-libs="${DCE_LIBS} ${DRM_LIBS} ${GBM_LIBS} ${EGL_LIBS} -lGLESv2" \
		--extra-cflags="${DCE_CFLAGS} ${DRM_CFLAGS} ${GBM_CFLAGS} ${EGL_CFLAGS}"
}

do_compile () {
	oe_runmake
}

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
