require e2fsprogs.inc

PR = "${INC_PR}"

SRC_URI += "file://remove.ldconfig.call.patch \
           file://run-ptest \
           file://ptest.patch \
           file://mkdir_p.patch \
           file://0001-misc-create_inode.c-set-dir-s-mode-correctly.patch \
           file://0001-configure.ac-correct-AM_GNU_GETTEXT.patch \
           file://0001-intl-do-not-try-to-use-gettext-defines-that-no-longe.patch \
           file://CVE-2019-5188.patch \
           file://0001-e2fsck-don-t-try-to-rehash-a-deleted-directory.patch \
           file://e2fsck-fix-use-after-free-in-calculate_tree.patch \
           "

SRC_URI_append_class-native = " file://e2fsprogs-fix-missing-check-for-permission-denied.patch \
                                file://quiet-debugfs.patch \
"

SRCREV = "984ff8d6a0a1d5dc300505f67b38ed5047d51dac"

do_configure() {
	oe_runconf
}
