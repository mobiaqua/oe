require glibc.inc

DEFAULT_PREFERENCE = "10"

DEPENDS += "gperf-native kconfig-frontends-native"

PV = "2.20"

SRCREV = "b8079dd0d360648e4e8de48656c5c38972621072"

SRC_URI = "git://sourceware.org/git/glibc.git;protocol=git;branch=release/${PV}/master \
           file://IO-acquire-lock-fix.patch \
           file://mips-rld-map-check.patch \
           file://etc/ld.so.conf \
           file://generate-supported.mk \
           file://glibc.fix_sqrt2.patch \
           file://ppc-sqrt_finite.patch \
           file://ppc_slow_ieee754_sqrt.patch \
           file://add_resource_h_to_wait_h.patch \
           file://fsl-ppc-no-fsqrt.patch \
           file://0001-R_ARM_TLS_DTPOFF32.patch \
           file://0001-eglibc-run-libm-err-tab.pl-with-specific-dirs-in-S.patch \
           file://fix-tibetian-locales.patch \
           file://ppce6500-32b_slow_ieee754_sqrt.patch \
           file://grok_gold.patch \
           file://fix_am_rootsbindir.patch \
           ${EGLIBCPATCHES} \
           ${CVEPATCHES} \
           file://fix-compile-fs-noncase.patch \
           file://fix-rpc.patch \
           file://skip-broken-locales.patch \
          "

EGLIBCPATCHES = "\
           file://timezone-re-written-tzselect-as-posix-sh.patch \
           file://eglibc.patch \
           file://option-groups.patch \
           file://GLRO_dl_debug_mask.patch \
           file://eglibc-header-bootstrap.patch \
           file://eglibc-resolv-dynamic.patch \
           file://eglibc-ppc8xx-cache-line-workaround.patch \
           file://eglibc-sh4-fpscr_values.patch \
           file://eglibc-use-option-groups.patch \
          "

CVEPATCHES = "\
        file://CVE-2014-7817-wordexp-fails-to-honour-WRDE_NOCMD.patch \
        file://CVE-2012-3406-Stack-overflow-in-vfprintf-BZ-16617.patch \
    "

S = "${WORKDIR}/git"
B = "${WORKDIR}/build-${TARGET_SYS}"

PACKAGES_DYNAMIC = "libc6*"
RPROVIDES_${PN}-dev = "libc6-dev virtual-libc-dev"
PROVIDES_${PN}-dbg = "glibc-dbg"

BUILD_CPPFLAGS = "-I${STAGING_INCDIR_NATIVE}"
TARGET_CPPFLAGS = "-I${STAGING_DIR_TARGET}${layout_includedir}"

GLIBC_BROKEN_LOCALES = " _ER _ET so_ET yn_ER sid_ET tr_TR mn_MN gez_ET gez_ER bn_BD te_IN es_CR.ISO-8859-1"

#
# For now, we will skip building of a gcc package if it is a uclibc one
# and our build is not a uclibc one, and we skip a glibc one if our build
# is a uclibc build.
#
# See the note in gcc/gcc_3.4.0.oe
#

python __anonymous () {
    import bb, re
    uc_os = (re.match('.*uclibc$', bb.data.getVar('TARGET_OS', d, 1)) != None)
    if uc_os:
        raise bb.parse.SkipPackage("incompatible with target %s" %
                                   bb.data.getVar('TARGET_OS', d, 1))
}

EXTRA_OECONF = "--enable-kernel=${OLDEST_KERNEL} \
                --without-cvs --disable-profile --disable-debug --without-gd \
                --enable-clocale=gnu \
                --enable-add-ons \
                --with-headers=${STAGING_INCDIR} \
                --without-selinux \
                --enable-obsolete-rpc \
                --with-kconfig=${STAGING_BINDIR_NATIVE} \
                ${GLIBC_EXTRA_OECONF}"

do_configure () {
# override this function to avoid the autoconf/automake/aclocal/autoheader
# calls for now
# don't pass CPPFLAGS into configure, since it upsets the kernel-headers
# version check and doesn't really help with anything
        if [ -z "`which rpcgen`" ]; then
                echo "rpcgen not found.  Install glibc-devel."
                exit 1
        fi
        (cd ${S} && gnu-configize) || die "failure in running gnu-configize"
        find ${S} -name "configure" | xargs touch
        CPPFLAGS="" oe_runconf
}

rpcsvc = "bootparam_prot.x nlm_prot.x rstat.x \
	  yppasswd.x klm_prot.x rex.x sm_inter.x mount.x \
	  rusers.x spray.x nfs_prot.x rquota.x key_prot.x"

do_compile () {
	# -Wl,-rpath-link <staging>/lib in LDFLAGS can cause breakage if another glibc is in staging
	unset LDFLAGS
	oe_runmake libdir='${libdir}' slibdir='${base_libdir}' \
        localedir='${libdir}/locale'
	(
		cd ${S}/sunrpc/rpcsvc
		for r in ${rpcsvc}; do
			h=`echo $r|sed -e's,\.x$,.h,'`
			rpcgen -h $r -o $h || oewarn "unable to generate header for $r"
		done
	)
}

require glibc-package.inc
