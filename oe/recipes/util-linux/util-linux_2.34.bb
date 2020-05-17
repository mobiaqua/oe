require util-linux.inc

SRC_URI = "${KERNELORG_MIRROR}/linux/utils/util-linux/v2.34/util-linux-${PV}${RC}.tar.xz;name=archive"

SRC_URI += "file://configure-sbindir.patch \
"

SRC_URI[archive.md5sum] = "a78cbeaed9c39094b96a48ba8f891d50"
SRC_URI[archive.sha256sum] = "743f9d0c7252b6db246b659c1e1ce0bd45d8d4508b4dfa427bbb4a3e9b9f62b5"
