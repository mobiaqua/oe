
inherit autotools lib_package

PV = "1.0.0"
PR = "r0"

DEPENDS = "udev libdrm"

SRCREV = "7c469a6d7a92ee702c5852d35564b3942878b5b2"
SRC_URI = "git://github.com/mobiaqua/libgbm.git;protocol=git \
"

S = "${WORKDIR}/git"

DEBUG_BUILD = "${@['no','yes'][bb.data.getVar('BUILD_DEBUG', d, 1) == '1']}"
CFLAGS_append = "${@['',' -O0 -g3'][bb.data.getVar('BUILD_DEBUG', d, 1) == '1']}"

do_rm_work() {
        if [ "${DEBUG_BUILD}" == "no" ]; then
                cd ${WORKDIR}
                for dir in *
                do
                        if [ `basename ${dir}` = "temp" ]; then
                                echo "Not removing temp"
                        else
                                echo "Removing $dir" ; rm -rf $dir
                        fi
                done
        fi
}
