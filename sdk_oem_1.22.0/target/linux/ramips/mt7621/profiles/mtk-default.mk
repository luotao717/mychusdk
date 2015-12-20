#
# Copyright (C) 2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/MT7621_MT7602e_MT7612e
    NAME:=MT7621+MT7602e+MT7612e
    PACKAGES:=\
        -swconfig \
        ated hwnat reg gpio btnd switch ethstt uci2dat mii_mgr watchdog 8021xd \
        wireless-tools wpad-mini wpa-supplicant luci-mtk xl2tpd \
        kmod-usb-core kmod-xhci kmod-usb-storage \
        kmod-ledtrig-usbdev block-mount \
        kmod-mt76x2e \
        kmod-hw_nat kmod-mtk-watchdog \
        kmod-fs-vfat kmod-fs-ntfs kmod-nls-base kmod-nls-utf8 kmod-nls-cp936 kmod-nls-cp950
endef

define Profile/MT7621_MT7602e_MT7612e/Description
	MT7621 SOC board, with MT7602e and MT7612e
endef
$(eval $(call Profile,MT7621_MT7602e_MT7612e))

define Profile/MT7621_MT7603e_MT7612e
    NAME:=MT7621+MT7603e+MT7612e
    PACKAGES:=\
        -swconfig \
        ated hwnat reg gpio btnd switch ethstt uci2dat mii_mgr watchdog 8021xd \
        wireless-tools wpad-mini wpa-supplicant luci-mtk xl2tpd \
        kmod-usb-core kmod-xhci kmod-usb-storage \
        kmod-ledtrig-usbdev block-mount \
        kmod-mt76x2e kmod-mt7603e \
        kmod-hw_nat kmod-mtk-watchdog \
        kmod-fs-vfat kmod-fs-ntfs kmod-nls-base kmod-nls-utf8 kmod-nls-cp936 kmod-nls-cp950
endef

define Profile/MT7621_MT7603e_MT7612e/Description
        MT7621 SOC board, with MT7603e and MT7612e
endef
$(eval $(call Profile,MT7621_MT7603e_MT7612e))
