DEPENDS = "libdrm libgbm virtual/egl"

inherit autotools

PV = "0.0.1"
PR = "r0"
PR_append = "+gitr-${SRCREV}"

SRCREV = "1c8a0d26c5b1918432fd94d2ac9894b3dcdb2814"
SRC_URI = "git://git.ti.com/glsdk/kmscube.git;protocol=git \
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
