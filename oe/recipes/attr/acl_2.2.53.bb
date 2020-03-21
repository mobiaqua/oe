SUMMARY = "Utilities for managing POSIX Access Control Lists"
HOMEPAGE = "http://savannah.nongnu.org/projects/acl/"
SECTION = "libs"

LICENSE = "LGPLv2.1+ & GPLv2+"
LICENSE_${PN} = "GPLv2+"
LICENSE_lib${BPN} = "LGPLv2.1+"
LIC_FILES_CHKSUM = "file://doc/COPYING;md5=c781d70ed2b4d48995b790403217a249 \
                    file://doc/COPYING.LGPL;md5=9e9a206917f8af112da634ce3ab41764"

DEPENDS = "attr"

SRC_URI = "http://download.savannah.gnu.org/releases/acl/${BP}.tar.gz \
           "

SRC_URI[md5sum] = "007aabf1dbb550bcddde52a244cd1070"
SRC_URI[sha256sum] = "06be9865c6f418d851ff4494e12406568353b891ffe1f596b34693c387af26c7"

inherit autotools gettext

PACKAGES =+ "lib${BPN}"

FILES_lib${BPN} = "${libdir}/lib*${SOLIBS}"

PTEST_BUILD_HOST_FILES = "builddefs"
PTEST_BUILD_HOST_PATTERN = "^RPM"

BBCLASSEXTEND = "native nativesdk"
