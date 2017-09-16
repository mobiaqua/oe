inherit cross

DEPENDS = "virtual/${TARGET_PREFIX}binutils virtual/${TARGET_PREFIX}libc-for-gcc ${NATIVEDEPS}"
PROVIDES = "virtual/${TARGET_PREFIX}gcc virtual/${TARGET_PREFIX}g++"

# Compute how to get from libexecdir to bindir in python (easier than shell)
BINRELPATH = "${@os.path.relpath(bb.data.expand("${STAGING_DIR_NATIVE}${prefix_native}/bin/${TARGET_SYS}", d), bb.data.expand("${libexecdir}/gcc/${TARGET_SYS}/${BINV}", d))}"

require gcc-configure-cross.inc
require gcc-package-cross.inc
