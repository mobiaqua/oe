DESCRIPTION = "GLib bindings for the dbus message bus system"
HOMEPAGE = "http://www.freedesktop.org/Software/dbus"
SECTION = "libs"
LICENSE = "GPLv2+ AFLv2.1"

DEPENDS = "expat glib-2.0 virtual/libintl dbus"

INC_PR = "r2"

SRC_URI = "\
           http://dbus.freedesktop.org/releases/dbus-glib/dbus-glib-${PV}.tar.gz \
           file://no-examples.patch \
           file://no-introspect.patch \
           file://dbus-glib-bindings.h \
          "

inherit autotools pkgconfig gettext

addtask fixinclude before do_configure after do_patch

do_fixinclude() {
    cp ${WORKDIR}/dbus-glib-bindings.h ${S}/tools/
}

FILES_${PN} = "${libdir}/lib*.so.*"

FILES_${PN}-dev += "\
  ${libdir}/dbus-1.0/include \
  ${bindir}/dbus-glib-tool \
  ${bindir}/dbus-binding-tool \
"
