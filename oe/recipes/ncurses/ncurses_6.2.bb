DESCRIPTION = "Ncurses library"
HOMEPAGE = "http://www.gnu.org/software/ncurses/ncurses.html"
LICENSE = "MIT"
SECTION = "libs"
PR = "r1"

DEPENDS = "ncurses-native"

inherit autotools binconfig

SRC_URI = "${GNU_MIRROR}/ncurses/ncurses-${PV}.tar.gz \
        file://0001-tic-hang.patch \
        file://0002-configure-reproducible.patch \
        file://hex.diff \
        file://config.cache \
"

SRC_URI[md5sum] = "e812da327b1c2214ac1aed440ea3ae8d"
SRC_URI[sha256sum] = "30306e0c76e0f9f1f0de987cf1c82a5c21e1ce6568b9227f7da5b71cbea86c9d"

PARALLEL_MAKE = ""
EXTRA_AUTORECONF = "-I m4 --with-abi-version=5"
CONFIG_SITE =+ "${WORKDIR}/config.cache"

# Whether to enable separate widec libraries; must be 'true' or 'false'
#
# TODO: remove this variable when widec is supported in every setup?
ENABLE_WIDEC = "true"

# _GNU_SOURCE is required for widec stuff and is detected automatically
# for target objects.  But it must be set manually for native and sdk
# builds.
BUILD_CPPFLAGS += "-D_GNU_SOURCE"

# Override the function from the autotools class; ncurses requires a
# patched autoconf213 to generate the configure script. This autoconf
# is not available so that the shipped script will be used.
do_configure() {
        # check does not work with cross-compiling and is generally
        # broken because it requires stdin to be pollable (which is
        # not the case for /dev/null redirections)
        export cf_cv_working_poll=yes

        for i in \
        'narrowc' \
        'widec   --enable-widec --without-progs'; do
                set -- $i
                mkdir -p $1
                cd $1
                shift

                oe_runconf \
                        --disable-static \
                        --without-debug \
                        --without-ada \
                        --without-gpm \
                        --enable-hard-tabs \
                        --enable-xmc-glitch \
                        --enable-colorfgbg \
                        --with-termpath='${sysconfdir}/termcap:${datadir}/misc/termcap' \
                        --with-terminfo-dirs='${sysconfdir}/terminfo:${datadir}/terminfo' \
                        --with-shared \
                        --disable-big-core \
                        --program-prefix= \
                        --with-ticlib \
                        --with-termlib=tinfo \
                        --enable-sigwinch \
                        --enable-pc-files \
                        --disable-rpath-hack \
                        --disable-mixed-case \
                        --with-manpage-format=normal \
                        --without-manpage-renames \
                        --disable-stripping \
                        --with-build-cc="${BUILD_CC}" \
                        --with-build-cpp="${BUILD_CPP}" \
                        --with-build-ld="${BUILD_LD}" \
                        --with-build-cflags="${BUILD_CFLAGS}" \
                        --with-build-cppflags='${BUILD_CPPFLAGS}' \
                        --with-build-ldflags='${BUILD_LDFLAGS}' \
                        "$@"
                cd ..
        done
}

do_compile() {
        oe_runmake -C narrowc libs
        oe_runmake -C narrowc/progs

        ! ${ENABLE_WIDEC} || \
            oe_runmake -C widec libs
}

do_test[dirs] = "${S}"
do_test() {
        :
}

_install_opts = "\
  DESTDIR='${D}' \
  PKG_CONFIG_LIBDIR='${libdir}/pkgconfig' \
  install.libs install.includes install.man \
"

do_install() {
        # Order of installation is important; widec installs a 'curses.h'
        # header with more definitions and must be installed last hence.
        # Compatibility of these headers will be checked in 'do_test()'.
        oe_runmake -C narrowc ${_install_opts} \
                install.data install.progs

        ! ${ENABLE_WIDEC} || \
            oe_runmake -C widec ${_install_opts}


        cd narrowc

        # include some basic terminfo files
        # stolen ;) from gentoo and modified a bit
        for x in ansi console dumb linux rxvt screen sun vt{52,100,102,200,220} xterm-color xterm-xfree86
        do
                local termfile="$(find "${D}${datadir}/terminfo/" -name "${x}" 2>/dev/null)"
                local basedir="$(basename $(dirname "${termfile}"))"

                if [ -n "${termfile}" ]
                then
                        install -d ${D}${sysconfdir}/terminfo/${basedir}
                        mv ${termfile} ${D}${sysconfdir}/terminfo/${basedir}/
                        ln -s /etc/terminfo/${basedir}/${x} \
                                ${D}${datadir}/terminfo/${basedir}/${x}
                fi
        done
        # i think we can use xterm-color as default xterm
        if [ -e ${D}${sysconfdir}/terminfo/x/xterm-color ]
        then
                ln -sf xterm-color ${D}${sysconfdir}/terminfo/x/xterm
        fi

        if [ "${PN}" = "ncurses" ]; then
                mv ${D}${bindir}/clear ${D}${bindir}/clear.${PN}
                mv ${D}${bindir}/reset ${D}${bindir}/reset.${PN}
        fi


        # create linker scripts for libcurses.so and libncurses to
        # link against -ltinfo when needed. Some builds might break
        # else when '-Wl,--no-copy-dt-needed-entries' has been set in
        # linker flags.
        for i in libncurses libncursesw; do
                f=${D}${libdir}/$i.so
                test -h $f || continue
                rm -f $f
                echo '/* GNU ld script */'  >$f
                echo "INPUT($i.so.6 AS_NEEDED(-ltinfo))" >>$f
        done

        # create libtermcap.so linker script for backward compatibility
        f=${D}${libdir}/libtermcap.so
        echo '/* GNU ld script */' >$f
        echo 'INPUT(AS_NEEDED(-ltinfo))' >>$f
}

python populate_packages_prepend () {
        libdir = bb.data.expand("${libdir}", d)
        pnbase = bb.data.expand("${PN}-lib%s", d)
        do_split_packages(d, libdir, '^lib(.*)\.so\..*', pnbase, 'ncurses %s library', prepend=True, extra_depends = '', allow_links=True)
}


pkg_postinst_ncurses-tools () {
        if [ "${PN}" = "ncurses" ]; then
                update-alternatives --install ${bindir}/clear clear clear.${PN} 100
                update-alternatives --install ${bindir}/reset reset reset.${PN} 100
        fi
}

pkg_prerm_ncurses-tools () {
        if [ "${PN}" = "ncurses" ]; then
                update-alternatives --remove clear clear.${PN}
                update-alternatives --remove reset reset.${PN}
        fi
}

#MobiAqua: disable native to keep old version
BBCLASSEXTEND = "sdk"
#BBCLASSEXTEND = "native sdk"

PACKAGES += " \
  ${PN}-tools \
  ${PN}-terminfo \
  ${PN}-terminfo-base \
"

FILES_${PN} = "\
  ${bindir}/tput \
  ${bindir}/tset \
  ${datadir}/tabset \
"

# This keeps only tput/tset in ncurses
# clear/reset are in already busybox
FILES_${PN}-tools = "\
  ${bindir}/tic \
  ${bindir}/toe \
  ${bindir}/infotocap \
  ${bindir}/captoinfo \
  ${bindir}/infocmp \
  ${bindir}/clear.${PN} \
  ${bindir}/reset.${PN} \
  ${bindir}/tack \
  ${bindir}/tabs \
"
# 'reset' is a symlink to 'tset' which is in the 'ncurses' package
RDEPENDS_${PN}-tools = "${PN}"

FILES_${PN}-terminfo = "\
  ${datadir}/terminfo \
"

FILES_${PN}-terminfo-base = "\
  ${sysconfdir}/terminfo \
"

RSUGGESTS_${PN}-libtinfo = "${PN}-terminfo"
RRECOMMENDS_${PN}-libtinfo = "${PN}-terminfo-base"