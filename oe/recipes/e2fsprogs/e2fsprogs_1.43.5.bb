require e2fsprogs.inc

PR = "${INC_PR}"

SRC_URI += "file://acinclude.m4 \
            file://remove.ldconfig.call.patch \
            file://quiet-debugfs.patch \
            file://run-ptest \
            file://ptest.patch \
            file://mkdir.patch \
            file://Revert-mke2fs-enable-the-metadata_csum-and-64bit-fea.patch \
            file://mkdir_p.patch \
            file://reproducible-doc.patch \
            file://fix-stat.patch \
"

SRC_URI_append_class-native = " file://e2fsprogs-fix-missing-check-for-permission-denied.patch"

SRCREV = "2a13c84b513aa094d1cda727e92d35a89dd777da"

do_configure() {
	oe_runconf
}
