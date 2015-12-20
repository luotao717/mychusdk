#
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define KernelPackage/usb-rt305x-dwc_otg
  TITLE:=RT305X USB controller driver
  DEPENDS:=@TARGET_ramips_rt305x
  KCONFIG:= \
	CONFIG_DWC_OTG \
	CONFIG_DWC_OTG_HOST_ONLY=y \
	CONFIG_DWC_OTG_DEVICE_ONLY=n \
	CONFIG_DWC_OTG_DEBUG=n
  FILES:=$(LINUX_DIR)/drivers/usb/dwc_otg/dwc_otg.ko
  AUTOLOAD:=$(call AutoLoad,54,dwc_otg,1)
  $(call AddDepends/usb)
endef

define KernelPackage/usb-rt305x-dwc_otg/description
 This driver provides USB Device Controller support for the
 Synopsys DesignWare USB OTG Core used in the Ralink RT305X SoCs.
endef

$(eval $(call KernelPackage,usb-rt305x-dwc_otg))

OTHER_MENU:=Other modules
define KernelPackage/sdhci-mt7620
  SUBMENU:=Other modules
  TITLE:=MT7620 SDCI
  DEPENDS:=@TARGET_ramips_mt7620a +kmod-sdhci
  KCONFIG:= \
	CONFIG_MMC_SDHCI_MT7620
  FILES:= \
	$(LINUX_DIR)/drivers/mmc/host/sdhci-mt7620.ko
  AUTOLOAD:=$(call AutoProbe,sdhci-mt7620,1)
endef

$(eval $(call KernelPackage,sdhci-mt7620))

I2C_RALINK_MODULES:= \
  CONFIG_I2C_RALINK:drivers/i2c/busses/i2c-ralink

define KernelPackage/i2c-ralink
  $(call i2c_defaults,$(I2C_RALINK_MODULES),59)
  TITLE:=Ralink I2C Controller
  DEPENDS:=@TARGET_ramips kmod-i2c-core
endef

define KernelPackage/i2c-ralink/description
 Kernel modules for enable ralink i2c controller.
endef

$(eval $(call KernelPackage,i2c-ralink))


define KernelPackage/hw_nat
  CATEGORY:=Ralink Properties
  SUBMENU:=Drivers
  TITLE:=Ralink Hardware NAT
  KCONFIG:=CONFIG_RA_HW_NAT
  FILES:=$(LINUX_DIR)/net/nat/hw_nat/hw_nat.ko
  AUTOLOAD:=$(call AutoProbe,hw_nat)
endef

define KernelPackage/hw_nat/install
	$(INSTALL_DIR) $(1)/lib/modules/ralink/
	mv $(1)/lib/modules/2.6.36/hw_nat.ko $(1)/lib/modules/ralink/
endef

$(eval $(call KernelPackage,hw_nat))


define KernelPackage/nfc6605
  CATEGORY:=Ralink Properties
  SUBMENU:=Drivers
  TITLE:=MediaTek MT6605 NFC
  KCONFIG:=CONFIG_MT6605_NFC
  FILES:=$(LINUX_DIR)/drivers/net/nfc/nfc_drv.ko
  AUTOLOAD:=$(call AutoProbe,nfc_drv)
  DEPENDS:=@TARGET_ramips_mt7621
endef

$(eval $(call KernelPackage,nfc6605))


define KernelPackage/hw_wdg
  CATEGORY:=Ralink Properties
  SUBMENU:=Drivers
  TITLE:=MTK APSoC Watchdog Driver
  KCONFIG:= CONFIG_WATCHDOG=y CONFIG_RALINK_WATCHDOG CONFIG_RALINK_TIMER_WDG_RESET_OUTPUT=y
  FILES:=$(LINUX_DIR)/drivers/watchdog/ralink_wdt.ko
  AUTOLOAD:=$(call AutoProbe,ralink_wdt)
endef

$(eval $(call KernelPackage,hw_wdg))


define KernelPackage/hw_kwdg
  CATEGORY:=Ralink Properties
  SUBMENU:=Drivers
  TITLE:=MTK APSoC Kernel Mode Watchdog
  KCONFIG:=CONFIG_RALINK_TIMER CONFIG_RALINK_TIMER_DFS=y CONFIG_RALINK_TIMER_WDG CONFIG_RALINK_WDG_TIMER=10 CONFIG_RALINK_WDG_REFRESH_INTERVAL=4 CONFIG_RALINK_TIMER_WDG_RESET_OUTPUT=y
  FILES:=$(LINUX_DIR)/arch/mips/ralink/ralink_wdt.ko
  AUTOLOAD:=$(call AutoProbe,ralink_wdt)
endef

$(eval $(call KernelPackage,hw_kwdg))

