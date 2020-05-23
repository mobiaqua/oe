require openssh.inc

PR = "${INC_PR}.0"

SRC_URI = "http://ftp.openbsd.org/pub/OpenBSD/OpenSSH/portable/openssh-${PV}.tar.gz \
           file://sshd_config \
           file://ssh_config \
           file://init \
           file://fix-potential-signed-overflow-in-pointer-arithmatic.patch \
          "

SRC_URI[md5sum] = "3076e6413e8dbe56d33848c1054ac091"
SRC_URI[sha256sum] = "43925151e6cf6cee1450190c0e9af4dc36b41c12737619edff8bcebdff64e671"
