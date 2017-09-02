SOC_FAMILY = "omap3"
TARGET_ARCH = "arm"

require conf/machine/include/tune-cortexa8.inc
PREFERRED_PROVIDER_virtual/kernel = "linux-omap3"

KERNEL_IMAGETYPE = "uImage"

UBOOT_ENTRYPOINT = "0x80008000"
UBOOT_LOADADDRESS = "0x80008000"

EXTRA_IMAGEDEPENDS += "u-boot x-loader"

