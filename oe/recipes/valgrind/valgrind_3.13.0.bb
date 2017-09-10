SUMMARY = "Valgrind memory debugger and instrumentation framework"
HOMEPAGE = "http://valgrind.org/"
BUGTRACKER = "http://valgrind.org/support/bug_reports.html"
LICENSE = "GPLv2 & GPLv2+ & BSD"

SRC_URI = "ftp://sourceware.org/pub/valgrind/valgrind-${PV}.tar.bz2 \
           file://fixed-perl-path.patch \
           file://0002-remove-rpath.patch \
           file://0004-Fix-out-of-tree-builds.patch \
           file://use-appropriate-march-mcpu-mfpu-for-ARM-test-apps.patch \
           file://avoid-neon-for-targets-which-don-t-support-it.patch \
           file://valgrind-make-ld-XXX.so-strlen-intercept-optional.patch \
           file://0001-makefiles-Drop-setting-mcpu-to-cortex-a8-on-arm-arch.patch \
           file://0001-str_tester.c-Limit-rawmemchr-test-to-glibc.patch \
           file://0003-correct-include-directive-path-for-config.h.patch \
           file://0006-pth_detached3.c-Dereference-pthread_t-before-adding-.patch \
           file://0003-tests-seg_override-Replace-__modify_ldt-with-syscall.patch \
           file://link-gz-tests.patch \
           "
SRC_URI[md5sum] = "817dd08f1e8a66336b9ff206400a5369"
SRC_URI[sha256sum] = "d76680ef03f00cd5e970bbdcd4e57fb1f6df7d2e2c071635ef2be74790190c3b"

COMPATIBLE_HOST = '(i.86|x86_64|arm|aarch64|mips|powerpc|powerpc64).*-linux'

# valgrind supports armv7 and above
COMPATIBLE_HOST_armv4 = 'null'
COMPATIBLE_HOST_armv5 = 'null'
COMPATIBLE_HOST_armv6 = 'null'

inherit autotools

EXTRA_OECONF = "--enable-tls --without-mpicc --enable-only32bit"

# valgrind checks host_cpu "armv7*)", so we need to over-ride the autotools.bbclass default --host option
EXTRA_OECONF_append_arm = " --host=armv7a-hf${HOST_VENDOR}-${HOST_OS}"

EXTRA_OEMAKE = "-w"

CACHED_CONFIGUREVARS += "ac_cv_path_PERL='/usr/bin/env perl'"

# valgrind likes to control its own optimisation flags. It generally defaults
# to -O2 but uses -O0 for some specific test apps etc. Passing our own flags
# (via CFLAGS) means we interfere with that. Only pass DEBUG_FLAGS to it
# which fixes build path issue in DWARF.
SELECTED_OPTIMIZATION = ""

do_configure_prepend () {
    rm -rf ${S}/config.h
}

do_install_append () {
    install -m 644 ${B}/default.supp ${D}/${libdir}/valgrind/
}
