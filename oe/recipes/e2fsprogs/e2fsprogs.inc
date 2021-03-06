DESCRIPTION = "EXT2 Filesystem Utilities"
HOMEPAGE = "http://e2fsprogs.sf.net"
LICENSE = "GPLv2+|LGPLv2+|NewBSD|MIT"
SECTION = "base"
DEPENDS = "util-linux"

SRC_URI = "git://git.kernel.org/pub/scm/fs/ext2/e2fsprogs.git;protocol=git"
S = "${WORKDIR}/git"

INC_PR = "r1"

inherit autotools gettext

EXTRA_OECONF = "${@['','--disable-tls'][bb.data.getVar('ARM_ABI', d, 1) == 'oabi' and \
	bb.data.getVar('TARGET_OS', d, 1).find('uclibc') >= 0 ]} \
	--enable-elf-shlibs \
	--enable-dynamic-e2fsck --disable-nls --sbindir=${base_sbindir} \
	--disable-libuuid --disable-uuidd --disable-libblkid --disable-fsck"

PARALLEL_MAKE = ""

do_compile_prepend () {
	find ./ -print|xargs chmod u=rwX
	( cd util; ${BUILD_CC} subst.c -o subst )
}

do_install_append () {
    	oe_runmake install-libs DESTDIR=${D}
	ln -sf tune2fs ${D}/${base_sbindir}/findfs
	ln -sf tune2fs ${D}/${base_sbindir}/e2label
}

do_install_append_pn-e2fsprogs () {
	for f in e2fsck mke2fs; do
		mv ${D}${base_sbindir}/$f ${D}${base_sbindir}/$f.${PN}
	done
	for f in chattr; do
		rm ${D}${bindir}/$f
	done
	fsck=$(cd ${D}${base_sbindir}/ && ls -1 fsck.ext* | sed s/[^\.]*//)
	for e in $fsck; do
		rm ${D}${base_sbindir}/fsck$e
		rm ${D}${base_sbindir}/mkfs$e
		ln -sf e2fsck.${PN} ${D}${base_sbindir}/fsck$e.${PN}
		ln -sf mke2fs.${PN} ${D}${base_sbindir}/mkfs$e.${PN}
	done
}


pkg_postinst_e2fsprogs-blkid () {
    update-alternatives --install ${base_sbindir}/blkid blkid blkid.${PN} 100
}

pkg_prerm_e2fsprogs-blkid () {
    update-alternatives --remove blkid blkid.${PN}
}

pkg_postinst_e2fsprogs-e2fsck () {
	update-alternatives --install ${base_sbindir}/e2fsck e2fsck e2fsck.${PN} 100
	update-alternatives --install ${base_sbindir}/fsck.ext2 fsck.ext2 fsck.ext2.${PN} 100
	update-alternatives --install ${base_sbindir}/fsck.ext3 fsck.ext3 fsck.ext3.${PN} 100
	update-alternatives --install ${base_sbindir}/fsck.ext4 fsck.ext4 fsck.ext4.${PN} 100
	update-alternatives --install ${base_sbindir}/fsck.ext4dev fsck.ext4dev fsck.ext4dev.${PN} 100
}

pkg_prerm_e2fsprogs-e2fsck () {
	update-alternatives --remove e2fsck e2fsck.${PN}
	update-alternatives --remove fsck.ext2 fsck.ext2.${PN}
	update-alternatives --remove fsck.ext3 fsck.ext3.${PN}
	update-alternatives --remove fsck.ext4 fsck.ext4.${PN}
	update-alternatives --remove fsck.ext4dev fsck.ext4dev.${PN}
}

pkg_postinst_e2fsprogs-mke2fs () {
	update-alternatives --install ${base_sbindir}/mke2fs mke2fs mke2fs.${PN} 100
	update-alternatives --install ${base_sbindir}/mkfs.ext2 mkfs.ext2 mkfs.ext2.${PN} 100
	update-alternatives --install ${base_sbindir}/mkfs.ext3 mkfs.ext3 mkfs.ext3.${PN} 100
	update-alternatives --install ${base_sbindir}/mkfs.ext4 mkfs.ext4 mkfs.ext4.${PN} 100
	update-alternatives --install ${base_sbindir}/mkfs.ext4dev mkfs.ext4dev mkfs.ext4dev.${PN} 100
}

pkg_prerm_e2fsprogs-mke2fs () {
	update-alternatives --remove mke2fs mke2fs.${PN}
	update-alternatives --remove mkfs.ext2 mkfs.ext2.${PN}
	update-alternatives --remove mkfs.ext3 mkfs.ext3.${PN}
	update-alternatives --remove mkfs.ext4 mkfs.ext4.${PN}
	update-alternatives --remove mkfs.ext4dev mkfs.ext4dev.${PN}
}

# blkid used to be part of e2fsprogs but is useful outside, add it
# as an RDEPENDS_${PN} so that anything relying on it being in e2fsprogs
# still works
RDEPENDS_e2fsprogs = "util-linux e2fsprogs-badblocks"

FILES_${PN} += "${libdir}/e2initrd_helper"

PACKAGES =+ "e2fsprogs-e2fsck e2fsprogs-mke2fs e2fsprogs-tune2fs e2fsprogs-badblocks libcomerr libss"
PACKAGES =+ "libe2p libext2fs"
FILES_e2fsprogs-e2fsck = "${base_sbindir}/e2fsck.${PN} ${base_sbindir}/fsck.ext*.${PN}"
FILES_e2fsprogs-mke2fs = "${base_sbindir}/mke2fs.${PN} ${base_sbindir}/mkfs.ext*.${PN}"
FILES_e2fsprogs-tune2fs = "${base_sbindir}/tune2fs ${base_sbindir}/e2label ${base_sbindir}/findfs"
FILES_e2fsprogs-badblocks = "${base_sbindir}/badblocks"
FILES_libcomerr = "${libdir}/libcom_err.so.*"
FILES_libss = "${libdir}/libss.so.*"
FILES_libe2p = "${libdir}/libe2p.so.*"
FILES_libext2fs = "${libdir}/e2initrd_helper ${libdir}/libext2fs.so.*"

BBCLASSEXTEND = "native"
