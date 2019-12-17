DEPENDS = "libdce libdrm ffmpeg libgbm virtual/egl"

inherit autotools

PV = "1.0.6"
PR = "r0"
PR_append = "+gitr-${SRCREV}"

SRCREV = "152713c45d7972ed08f506255c43ce7ff634a9f5"
SRC_URI = "git://git.ti.com/glsdk/omapdrmtest.git;protocol=git \
           file://0004-display-kmscube-align-width-on-128-bytes-to-please-Ducat.patch \
           file://0005-Hack-disp-kmscube-reduce-u-v-by-10.patch \
           file://disable-v4l-vpe.patch \
           file://revert-to-old-dce.patch \
           file://av-fixes.patch \
"

S = "${WORKDIR}/git"

FLAGS="-lm -lavcodec"

EXTRA_OECONF = "LDFLAGS='${FLAGS}' --disable-x11"
