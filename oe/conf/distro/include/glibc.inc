PREFERRED_PROVIDER_virtual/libiconv ?= "glibc"
PREFERRED_PROVIDER_virtual/libintl ?= "glibc"
PREFERRED_PROVIDER_virtual/libc = "glibc"
PREFERRED_PROVIDER_virtual/${TARGET_PREFIX}libc-initial = "glibc-initial"
PREFERRED_PROVIDER_virtual/${TARGET_PREFIX}libc-for-gcc = "glibc"

# glibc is configured for OLDEST_KERNEL kernel version, so we set minimal
# sane versions there

OLDEST_KERNEL ?= "2.4.0"

# ARM OABI is fine with 2.4.0, EABI is not, so we override on "-gnueabi"
# TARGET_OS here, not on architecture which is 'arm' in both cases
OLDEST_KERNEL_linux-gnueabi ?= "2.6.16"
OLDEST_KERNEL_sparc64 ?= "2.4.21"
OLDEST_KERNEL_ppc64 ?= "2.4.21"
DISTRO_FEATURES_GLIBC ?= "ipv6 backtrace big-macros bsd cxx-tests catgets \
                           charsets crypt crypt-ufc db-aliases envz fcvt fmtmsg \
                           fstab ftraverse getlogin idn inet inet-anl libm \
                           libm-big locales locale-code memusage nis nsswitch \
                           rcmd rtld-debug spawn streams sunrpc utmp utmpx \
                           wordexp posix-clang-wchar posix-regexp \
                           posix-regexp-glibc posix-wchar-io"

