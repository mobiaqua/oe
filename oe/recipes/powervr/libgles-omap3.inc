DESCRIPTION = "libGLES for the omap3"
LICENSE = "proprietary-binary"

PR = "r15"

COMPATIBLE_MACHINE = "igep0030"

PROVIDES += "virtual/egl"

SRC_URI = "http://software-dl.ti.com/dsps/dsps_public_sw/sdo_sb/targetcontent/gfxsdk/${SGXPV}/exports/OMAP35x_Graphics_SDK_setuplinux_${SGXPV}.bin \
		   file://cputype \
		   file://rc.pvr \
		   file://99-bufferclass.rules  \
"

S = "${WORKDIR}/OMAP35x_Graphics_SDK_${SGXPV}"

# Logic to unpack installjammer file
TI_BIN_UNPK_CMDS="Y: qY:workdir:Y"
require ti-eula-unpack.inc

BINLOCATION ?= "${S}/gfx_rel"
ES2LOCATION ?= "${S}/gfx_rel_es2.x"
ES3LOCATION ?= "${S}/gfx_rel_es3.x"
ES5LOCATION ?= "${S}/gfx_rel_es5.x"
ES6LOCATION ?= "${S}/gfx_rel_es6.x"

LIBGLESWINDOWSYSTEM ?= "libpvrPVR2D_FRONTWSEGL.so.1"

do_configure() {
	# Attempt to fix up the worst offenders for file permissions
	for i in $(find ${S} -name "*.h") $(find ${S} -name "*.c") $(find ${S} -name "Make*") ; do
		chmod 0644 $i
	done 

	# Attempt to create proper library softlinks
	for sofile in $(find ${S} -name "lib*Open*.so") $(find ${S} -name "lib*srv*.so") $(find ${S} -name "lib*gl*.so") $(find ${S} -name "libpvr*.so") $(find ${S} -name "lib*GL*.so"); do
		if [ "$(readlink -n ${sofile})" = "" ] ; then
			mv $sofile ${sofile}.${IMGPV}
			ln -sf $(basename ${sofile}.${IMGPV}) ${sofile}
			ln -sf $(basename ${sofile}.${IMGPV}) ${sofile}$(echo ${IMGPV} | awk -F. '{print "." $1}')
			ln -sf $(basename ${sofile}.${IMGPV}) ${sofile}$(echo ${IMGPV} | awk -F. '{print "." $1 "." $2}')
		fi
	done

	# Due to recursive make PLAT_* isn't always passed down correctly, so use sed to fix those
	for mak in $(find ${S} -name "*.mak") ; do
		sed -i -e s:arm-none-linux-gnueabi-:${TARGET_PREFIX}:g $mak
	done

	# clear out old stuff
	find Binaries/ | xargs rm -f || true
}

# Force in GNU_HASH and paths to libs
TARGET_CC_ARCH += " ${TARGET_LINK_HASH_STYLE} -Wl,-rpath-link,${BINLOCATION} -L${BINLOCATION} \
-L${STAGING_DIR_TARGET}${libdir} -Wl,-rpath-link,${STAGING_DIR_TARGET}${libdir}"
PARALLEL_MAKE = ""

do_compile() {
	export TOOLCHAIN="${TOOLCHAIN_PATH}"
	export PLATFORM="LinuxOMAP3"

	export PLAT_CC="${CC}"
	export PLAT_CPP="${CXX}"
	export PLAR_AR="${AR}"

	mkdir -p ${S}/demos/raw

	# Rebuild demos for Raw
	for demo in ${S}/GFX_Linux_SDK/OGLES/SDKPackage/Demos/* ; do
		cd $demo/OGLES/Build/LinuxGeneric
		oe_runmake Common=1 PLATFORM=$PLATFORM X11BUILD=0
		rm $demo/OGLES/Build/LinuxOMAP3/Release*/*.o
		install -m 0755 $demo/OGLES/Build/LinuxOMAP3/ReleaseRaw/* ${S}/demos/raw || true	
	done

	mkdir -p ${S}/trainingcourses/raw

	find ${S} -name "*_org" -delete

	# Build OGLES2 Trainingcourses for Raw
	for training in ${S}/GFX_Linux_SDK/OGLES2/SDKPackage/TrainingCourse/* ; do
		if [ -e $training/OGLES2/Build/LinuxGeneric/Makefile ] ; then
			cd $training/OGLES2/Build/LinuxGeneric
		fi

		if [ -e $training/OGLES2/Build/LinuxOMAP3/Makefile ] ; then
			cd $training/OGLES2/Build/LinuxOMAP3
		fi

		oe_runmake Common=1 PLATFORM=$PLATFORM X11BUILD=0
		rm -f $training/OGLES2/Build/LinuxOMAP3/Release*/*.o
		install -m 0755 $training/OGLES2/Build/LinuxOMAP3/ReleaseRaw/* ${S}/trainingcourses/raw || true	
	done
}

do_install () {
	install -d ${D}${libdir}
	cp -pPR ${BINLOCATION}/*.so* ${D}${libdir}
	install -m 0644 ${BINLOCATION}/*.a ${D}${libdir}

	install -d ${D}${bindir}/
	install -m 0755 ${WORKDIR}/cputype ${D}${bindir}/

	install -m 0755 ${BINLOCATION}/*_test ${D}${bindir}/
	install -m 0755 ${BINLOCATION}/gl* ${D}${bindir}/
	install -m 0755 ${BINLOCATION}/p[dv]* ${D}${bindir}/

	install -d ${D}${includedir}
	cp -pPR ${S}/GFX_Linux_KM/include4 ${D}${includedir}/
	cp -pPR ${S}/GFX_Linux_KM/services4 ${D}${includedir}/

	cp -pPR ${S}/GFX_Linux_SDK/OGLES2/SDKPackage/Builds/OGLES2/Include/* ${D}${includedir}/
	cp -pPR ${S}/GFX_Linux_SDK/OGLES/SDKPackage/Builds/OGLES/Include/* ${D}${includedir}/
	cp -pPR ${S}/GFX_Linux_SDK/OGLES/SDKPackage/Builds/OGLES/LinuxOMAP3/Include/GLES/* ${D}${includedir}/GLES/ || true
	cp -pPr ${S}/GFX_Linux_SDK/OGLES2/SDKPackage/Builds/OGLES2/LinuxOMAP3/Include/GLES/* ${D}${includedir}/GLES2/ || true
	cp -pPr ${S}/GFX_Linux_SDK/OVG/SDKPackage/Builds/OVG/Include/v* ${D}${includedir}/ || true
	cp -pPr ${S}/GFX_Linux_SDK/OVG/SDKPackage/Builds/OVG/Include/V* ${D}${includedir}/ || true
	cp -pPr ${S}/include/*.h ${D}${includedir} || true
	cp -pPr ${S}/include/wsegl/*.h ${D}${includedir} || true
	
	install -d ${D}${sysconfdir}/init.d/
	cp -pP ${WORKDIR}/rc.pvr ${D}${sysconfdir}/init.d/pvr-init

	install -d ${D}${sysconfdir}
	echo "[default]" > ${D}${sysconfdir}/powervr.ini
	echo "WindowSystem=${LIBGLESWINDOWSYSTEM}" >> ${D}${sysconfdir}/powervr.ini

	# The ES2.x, ES3.x, ES5.x and ES6.x CPUs have different SGX hardware, so we need to install multiple sets of userspace

	install -d ${D}${libdir}/ES6.0
	install -d ${D}${libdir}/ES5.0
	install -d ${D}${libdir}/ES3.0
	install -d ${D}${libdir}/ES2.0

	install -d ${D}${bindir}/ES6.0
	install -d ${D}${bindir}/ES5.0
	install -d ${D}${bindir}/ES3.0
	install -d ${D}${bindir}/ES2.0


	cp -pPR ${ES2LOCATION}/lib*${IMGPV} ${D}${libdir}/ES2.0/
	cp ${ES2LOCATION}/p[dv]* ${D}${bindir}/ES2.0/

	cp -pPR ${D}${libdir}/lib*${IMGPV} ${D}${libdir}/ES3.0/
	cp ${D}${bindir}/p[dv]* ${D}${bindir}/ES3.0

	if [ -e ${ES5LOCATION} ] ; then 
		cp -pPR ${ES5LOCATION}/lib*${IMGPV} ${D}${libdir}/ES5.0/
		cp ${ES5LOCATION}/p[dv]* ${D}${bindir}/ES5.0/
	fi

	if [ -e ${ES6LOCATION} ] ; then
		cp -pPR ${ES6LOCATION}/lib*${IMGPV} ${D}${libdir}/ES6.0/
		cp ${ES6LOCATION}/p[dv]* ${D}${bindir}/ES6.0/
	fi

	rm ${D}${bindir}/ES*/*.h ${D}${bindir}/ES*/pdsasm ${D}${bindir}/pdsasm -f || true

	install -d ${D}${prefix}/share/applications
	cp ${WORKDIR}/*.desktop ${D}${prefix}/share/applications
	rm ${D}${prefix}/share/applications/sample.desktop

	install -d ${D}${bindir}/SGX/demos/Raw/
	install -m 0755 ${S}/demos/raw/* ${D}${bindir}/SGX/demos/Raw/

	install -d ${D}${bindir}/SGX/trainingcourses/Raw
	install -m 0755 ${S}/trainingcourses/raw/* ${D}${bindir}/SGX/trainingcourses/Raw/

	# Delete objects and linker scripts hidden between the headers
	find ${D} -name "*.o" -delete
	find ${D} -name "*.o.cmd" -delete

	install -d ${D}${sysconfdir}/udev/rules.d
	install -m 0644 ${WORKDIR}/99-bufferclass.rules ${D}${sysconfdir}/udev/rules.d/

}

PACKAGES =+ "${PN}-rawdemos \
             ${PN}-rawtrainingcourses \
"

PACKAGES += "xserver-kdrive-powervrsgx \
             ${PN}-tests "

PACKAGES =+ "${PN}-blitwsegl ${PN}-flipwsegl ${PN}-frontwsegl ${PN}-linuxfbwsegl"

FILES_${PN}-blitwsegl = "${libdir}/libpvrPVR2D_BLITWSEGL.so.* ${libdir}/*/libpvrPVR2D_BLITWSEGL.so.*"
FILES_${PN}-flipwsegl = "${libdir}/libpvrPVR2D_FLIPWSEGL.so.* ${libdir}/*/libpvrPVR2D_FLIPWSEGL.so.*"
FILES_${PN}-frontwsegl = "${libdir}/libpvrPVR2D_FRONTWSEGL.so.* ${libdir}/*/libpvrPVR2D_FRONTWSEGL.so.*"
FILES_${PN}-linuxfbwsegl = "${libdir}/libpvrPVR2D_LINUXFBWSEGL.so.* ${libdir}/*/libpvrPVR2D_LINUXFBWSEGL.so.*"

CONFFILES_${PN} = "${sysconfdir}/powervr.ini"

FILES_${PN} = "${sysconfdir} ${libdir}/lib*.so.* ${libdir}/ES*/* ${bindir}/pvrsrvinit ${bindir}/cputype ${bindir}/*/*"
FILES_${PN}-tests = "${bindir}/*"
FILES_${PN}-dbg = "${libdir}/.debug/* ${bindir}/.debug/* \
                   ${libdir}/ES*/.debug ${bindir}/*/.debug\
                   ${bindir}/SGX/demos/*/.debug/* \
                   ${bindir}/SGX/trainingcourses/*/.debug/* \
	"

FILES_${PN}-rawdemos = "${bindir}/SGX/demos/Raw/*"
FILES_${PN}-rawtrainingcourses = "${bindir}/SGX/trainingcourses/Raw/*"

# The libs need the kernel-modules
RRECOMMENDS_${PN} = "omap3-sgx-modules"

# The initscript calls fbset, cputype calls devmem2
RDEPENDS_${PN} += "fbset devmem2"

#HACK! These are binaries, so we can't guarantee that LDFLAGS match :(
INSANE_SKIP_${PN} = True
INSANE_SKIP_${PN}-blitwsegl = True
INSANE_SKIP_${PN}-flipwsegl = True
INSANE_SKIP_${PN}-frontwsegl = True
INSANE_SKIP_${PN}-linuxfbwsegl = True
INSANE_SKIP_${PN}-tests = True

# Quality control is really poor on these SDKs, so hack around the latest madness:
FILES_${PN} += "${libdir}/*.so "
FILES_${PN}-dev = "${includedir}"

inherit update-rc.d

INITSCRIPT_NAME = "pvr-init"
INITSCRIPT_PARAMS = "start 30 5 2 . stop 40 0 1 6 ."

# Append to update-rc.d postinst
pkg_postinst_${PN}_append() {
rm -f /etc/powervr-esrev
}
