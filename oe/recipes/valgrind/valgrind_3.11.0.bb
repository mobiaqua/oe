SUMMARY = "Valgrind memory debugger and instrumentation framework"
HOMEPAGE = "http://valgrind.org/"
BUGTRACKER = "http://valgrind.org/support/bug_reports.html"
LICENSE = "GPLv2 & GPLv2+ & BSD"
LIC_FILES_CHKSUM = "file://COPYING;md5=b234ee4d69f5fce4486a80fdaf4a4263 \
                    file://include/pub_tool_basics.h;beginline=1;endline=29;md5=ebb8e640ef633f940c425686c873f9fa \
                    file://include/valgrind.h;beginline=1;endline=56;md5=4b5e24908e53016ea561c45f4234a327 \
                    file://COPYING.DOCS;md5=24ea4c7092233849b4394699333b5c56"

SRC_URI = "http://www.valgrind.org/downloads/valgrind-${PV}.tar.bz2 \
           file://fixed-perl-path.patch \
           file://0002-remove-rpath.patch \
           file://0004-Fix-out-of-tree-builds.patch \
           file://use-appropriate-march-mcpu-mfpu-for-ARM-test-apps.patch \
           file://avoid-neon-for-targets-which-don-t-support-it.patch \
"

SRC_URI[md5sum] = "4ea62074da73ae82e0162d6550d3f129"
SRC_URI[sha256sum] = "6c396271a8c1ddd5a6fb9abe714ea1e8a86fce85b30ab26b4266aeb4c2413b42"

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

# valgrind likes to control its own optimisation flags. It generally defaults
# to -O2 but uses -O0 for some specific test apps etc. Passing our own flags
# (via CFLAGS) means we interfere with that.
SELECTED_OPTIMIZATION = ""

do_install_append () {
    install -m 644 ${B}/default.supp ${D}/${libdir}/valgrind/
}
