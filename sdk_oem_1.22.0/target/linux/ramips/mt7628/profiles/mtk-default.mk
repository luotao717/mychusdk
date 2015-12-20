#
# Copyright (C) 2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/MT7628_MT7612e
	NAME:=MT7628+MT7612e
	PACKAGES:=\
		-swconfig \
		ated hwnat reg gpio btnd switch ethstt uci2dat mii_mgr watchdog \
		wireless-tools wpad-mini wpa-supplicant luci-mtk xl2tpd \
		kmod-usb-core kmod-xhci kmod-usb-storage \
		kmod-ledtrig-usbdev block-mount \
		kmod-mt76x2e \
		kmod-hw_nat kmod-mtk-watchdog \
		kmod-fs-vfat kmod-fs-ntfs kmod-nls-base kmod-nls-utf8 kmod-nls-cp936 kmod-nls-cp950
endef

define Profile/MT7628_MT7612e/Description
	MT7628 SOC board, with MT7612e
endef
$(eval $(call Profile,MT7628_MT7612e))

