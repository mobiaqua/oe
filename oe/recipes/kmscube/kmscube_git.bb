DEPENDS = "libdrm libgbm virtual/egl"

inherit autotools

PV = "0.0.1"
PR = "r0"
PR_append = "+gitr-${SRCREV}"

SRCREV = "1c8a0d26c5b1918432fd94d2ac9894b3dcdb2814"
SRC_URI = "git://git.ti.com/glsdk/kmscube.git;protocol=git \
"

S = "${WORKDIR}/git"
