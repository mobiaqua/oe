DESCRIPTION = "udev is a daemon which dynamically creates and removes device nodes from \
/dev/, handles hotplug events and loads drivers at boot time. It replaces \
the hotplug package and requires a kernel not older than 2.6.12."
LICENSE = "GPLv2+"

PR = "r7"

#MobiAqua: use this working version as newer doesn't work properly

# Needed for udev-extras
DEPENDS = "usbutils acl glib-2.0 mtd-utils"
RDEPENDS_${PN} += "module-init-tools-depmod udev-utils"

SRC_URI = "http://pkgs.fedoraproject.org/repo/pkgs/udev/udev-166.tar.bz2/4db27d73fdbe94f47fd89fdd105c2dfb/udev-166.tar.bz2 \
	   file://mount.blacklist \
	   file://run.rules \
	   file://fix-PRIO_PROCESS.patch \
	   "

SRC_URI[md5sum] = "4db27d73fdbe94f47fd89fdd105c2dfb"
SRC_URI[sha256sum] = "f5d27381a16ad56ed76e472fc05a7116f7ffede0898b4ccbaf501fe2384354d7"

#MobiAqua: remove devfs.rules, added symlink into udev.rules
SRC_URI += " \
       file://udev.rules \
       file://links.conf \
       file://permissions.rules \
       file://mount.sh \
       file://network.sh \
       file://local.rules \
       file://default \
       file://init \
       file://udev-compat-wrapper-patch \
       file://udev-166-v4l1-1.patch \
"

inherit update-rc.d autotools

EXTRA_OECONF += " --with-udev-prefix= \
                  --with-libdir-name=${base_libdir} \
                  --with-pci-ids-path=/usr/share/misc \
                  --disable-introspection \
                  ac_cv_file__usr_share_pci_ids=no \
                  ac_cv_file__usr_share_hwdata_pci_ids=no \
                  ac_cv_file__usr_share_misc_pci_ids=yes \
                  --sbindir=${base_sbindir} \
                  --libexecdir=${base_libdir}/udev \
                  --with-rootlibdir=${base_libdir} \
"

INITSCRIPT_NAME = "udev"
INITSCRIPT_PARAMS = "start 04 S ."

PACKAGES =+ "libudev libgudev udev-utils"

FILES_libudev = "${base_libdir}/libudev.so.*"
FILES_libgudev = "${base_libdir}/libgudev*.so.*"

FILES_udev-utils = "${bindir}/udevinfo ${bindir}/udevtest ${base_sbindir}/udevadm"

RPROVIDES_${PN} = "hotplug"
FILES_${PN} += "${usrbindir}/* ${usrsbindir}/udevd"
FILES_${PN}-dbg += "${usrbindir}/.debug ${usrsbindir}/.debug"

# udev installs binaries under $(udev_prefix)/lib/udev, even if ${libdir}
# is ${prefix}/lib64
FILES_${PN} += "/lib/udev* ${libdir}/ConsoleKit"
FILES_${PN}-dbg += "/lib/udev/.debug"

# Package up systemd files
FILES_${PN} += "${base_libdir}/systemd"

RPROVIDES_udev_append = " udev-compat-wrapper"

# Modify init script on platforms that need to boot old kernels:
do_apply_compat_wrapper() {
	cd ${WORKDIR}
	sed -i "s:/sbin/udevd:\$UDEVD:g;s:/sbin/udevadm:\$UDEVADM:g" init
	patch <udev-compat-wrapper-patch
	cd -
}

do_install () {
	install -d ${D}${usrsbindir} \
		   ${D}${sbindir}
	oe_runmake 'DESTDIR=${D}' INSTALL=install install
	install -d ${D}${sysconfdir}/init.d
	install -m 0755 ${WORKDIR}/init ${D}${sysconfdir}/init.d/udev

	install -d ${D}${sysconfdir}/default
	install -m 0755 ${WORKDIR}/default ${D}${sysconfdir}/default/udev

 	cp ${S}/rules/rules.d/* ${D}${sysconfdir}/udev/rules.d/

	install -m 0644 ${WORKDIR}/mount.blacklist     ${D}${sysconfdir}/udev/
	install -m 0644 ${WORKDIR}/local.rules         ${D}${sysconfdir}/udev/rules.d/local.rules
	install -m 0644 ${WORKDIR}/permissions.rules   ${D}${sysconfdir}/udev/rules.d/permissions.rules
	install -m 0644 ${WORKDIR}/run.rules           ${D}${sysconfdir}/udev/rules.d/run.rules
	install -m 0644 ${WORKDIR}/udev.rules          ${D}${sysconfdir}/udev/rules.d/udev.rules
	install -m 0644 ${WORKDIR}/links.conf          ${D}${sysconfdir}/udev/links.conf

	touch ${D}${sysconfdir}/udev/saved.uname
	touch ${D}${sysconfdir}/udev/saved.cmdline
	touch ${D}${sysconfdir}/udev/saved.devices
	touch ${D}${sysconfdir}/udev/saved.atags

	install -d ${D}${sysconfdir}/udev/scripts/

	install -m 0755 ${WORKDIR}/mount.sh ${D}${sysconfdir}/udev/scripts/mount.sh
	install -m 0755 ${WORKDIR}/network.sh ${D}${sysconfdir}/udev/scripts
}

do_install_append () {
	# hid2hci has moved to bluez4. removed in udev as of version 169
	rm -f ${D}${base_libdir}/udev/hid2hci
}
