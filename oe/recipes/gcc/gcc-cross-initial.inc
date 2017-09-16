DEPENDS = "virtual/${TARGET_PREFIX}binutils ${NATIVEDEPS}"
PROVIDES = "virtual/${TARGET_PREFIX}gcc-initial"
PACKAGES = ""

# This is intended to be a -very- basic config
# sysroot is needed in case we use libc-initial
EXTRA_OECONF = "--with-local-prefix=${STAGING_DIR_TARGET}${target_prefix} \
    --with-newlib \
    --without-headers \
    --disable-shared \
    --disable-threads \
    --disable-multilib \
    --disable-__cxa_atexit \
    --enable-languages=c \
    --program-prefix=${TARGET_PREFIX} \
    ${EXTRA_OECONF_PATHS} \
    ${EXTRA_OECONF_INITIAL} \
    ${@get_gcc_fpu_setting(bb, d)} \
    ${@get_gcc_mips_plt_setting(bb, d)} \
"

do_compile () {
    oe_runmake all-gcc configure-target-libgcc
    (cd ${B}/${TARGET_SYS}/libgcc; oe_runmake enable-execute-stack.c unwind.h md-unwind-support.h sfp-machine.h gthr-default.h)
}

do_install () {
	( cd ${B}/${TARGET_SYS}/libgcc; oe_runmake 'DESTDIR=${D}' install-unwind_h-forbuild install-unwind_h)
	oe_runmake 'DESTDIR=${D}' install-gcc

	# We don't really need this (here shares/ contains man/, info/, locale/).
	rm -rf ${D}${datadir}/

	# We use libiberty from binutils
	find ${D}${exec_prefix}/lib -name libiberty.a | xargs rm -f
	find ${D}${exec_prefix}/lib -name libiberty.h | xargs rm -f

	# Insert symlinks into libexec so when tools without a prefix are searched for, the correct ones are
	# found. These need to be relative paths so they work in different locations.
	dest=${D}${libexecdir}/gcc/${TARGET_SYS}/${BINV}/
	install -d $dest
	# fixed limits.h infact includes the so called real limits.h
	# which should come from libc but when we build libc-initial
	# then bunch of configure tests include fixed limits.h which in turn
	# includes real limits.h but this real limits.h is not staged yet
	# so we overwirte the generated include-fixed/limits.h for gcc-cross-initial
	# to get rid references to real limits.h
	cp gcc/include-fixed/limits.h ${D}${gcclibdir}/${TARGET_SYS}/${BINV}/include/limits.h
}
