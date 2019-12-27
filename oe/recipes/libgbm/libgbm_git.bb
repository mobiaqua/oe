
inherit autotools lib_package

PV = "1.0.0"
PR = "r0"

DEPENDS = "udev libdrm"

SRCREV = "75186341a4d7bb70edd60759e87d782da91bbf59"
SRC_URI = "git://github.com/mobiaqua/libgbm.git;protocol=git \
"

S = "${WORKDIR}/git"
