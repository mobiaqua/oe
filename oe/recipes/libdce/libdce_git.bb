DEPENDS = "libdce-firmware libdrm virtual/kernel libmmrpc"

LICENSE = "BSD"

inherit autotools lib_package

PV = "1.0"
PR = "r0"
PR_append = "+gitr-${SRCREV}"

SRCREV = "36533bfb6c18e3536c84511a1e4c5a7cae1bb5bf"
SRC_URI = "git://github.com/mobiaqua/libdce.git;protocol=git"

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
