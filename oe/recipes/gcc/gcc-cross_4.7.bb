require gcc-${PV}.inc
require gcc-cross4.inc

PR = "${INC_PR}.1"

NATIVEDEPS += "libmpc-native libelf-native"

EXTRA_OECONF += " --disable-libunwind-exceptions --with-mpfr=${STAGING_DIR_NATIVE}${prefix_native} \
                 --with-libelf=${STAGING_DIR_NATIVE}${prefix_native} --with-system-zlib --enable-poison-system-directories"

ARCH_FLAGS_FOR_TARGET += "-isystem${STAGING_DIR_TARGET}${target_includedir}"
