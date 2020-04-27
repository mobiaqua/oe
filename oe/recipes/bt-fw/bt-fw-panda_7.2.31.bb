DESCRIPTION = "Firmware files for Bluetooth"
LICENSE = "TI-TSPA"

PV = "7.2.31"
PR = "r0"

COMPATIBLE_MACHINE = "board-tv"

SRC_URI = "file://TIInit_7.2.31.bts \
           file://fm_rx_ch8_1273.2.bts \
           file://fmc_ch8_1273.2.bts \
           file://TSPA_Object_Code_Software_License_Agreement.txt \
"

S = "${WORKDIR}"

do_compile() {
    :
}

do_install() {
    install -d ${D}${base_libdir}/firmware/ti-connectivity
    install -m 0644 ${S}/*.bts ${D}${base_libdir}/firmware/ti-connectivity/
    install -m 0644 ${S}/TSPA_Object_Code_Software_License_Agreement.txt ${D}${base_libdir}/firmware/ti-connectivity/
}

FILES_${PN} += "${base_libdir}/firmware/ti-connectivity/*"
