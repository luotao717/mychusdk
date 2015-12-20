#
# Copyright (C) 2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/MT7620a_MT7610e
	NAME:=MT7620a+MT7610e
	PACKAGES:=\
		-swconfig \
		ated hwnat reg gpio btnd switch ethstt uci2dat mii_mgr watchdog 8021xd \
		wireless-tools wpad-mini wpa-supplicant luci-mtk xl2tpd \
		kmod-usb-core kmod-usb-ohci kmod-usb2 kmod-usb-storage \
		kmod-ledtrig-usbdev block-mount\
		kmod-mt7610e kmod-mt7620 \
		kmod-hw_nat kmod-mtk-watchdog \
		kmod-fs-vfat kmod-fs-ntfs kmod-nls-base kmod-nls-utf8 kmod-nls-cp936 kmod-nls-cp950
endef

define Profile/MT7620a_MT7610e/Description
	MT7620a SOC board, with MT7610e (AC750)
endef
$(eval $(call Profile,MT7620a_MT7610e))

define Profile/MT7620a_MT7612e
	NAME:=MT7620a+MT7612e
	PACKAGES:=\
		-swconfig \
		ated hwnat reg gpio btnd switch ethstt uci2dat mii_mgr watchdog 8021xd \
		wireless-tools wpad-mini wpa-supplicant luci-mtk \
		kmod-usb-core kmod-usb-ohci kmod-usb2 kmod-usb-storage \
		kmod-ledtrig-usbdev block-mount\
		kmod-mt76x2e kmod-mt7620 kmod-scsi-core \
		kmod-hw_nat kmod-mtk-watchdog \
		kmod-fs-vfat kmod-fs-ntfs kmod-nls-base kmod-nls-utf8 kmod-nls-cp936 kmod-nls-cp950
endef

define Profile/MT7620a_MT7612e/Description
	MT7620a board, with MT7612e
endef
$(eval $(call Profile,MT7620a_MT7612e))


