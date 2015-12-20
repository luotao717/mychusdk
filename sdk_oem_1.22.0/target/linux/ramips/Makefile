#
# Copyright (C) 2008-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

ARCH:=mipsel
BOARD:=ramips
BOARDNAME:=Ralink RT288x/RT3xxx
SUBTARGETS:=rt288 xrt305x rt3883 mt7620a mt7621
FEATURES:=squashfs gpio

LINUX_VERSION:=2.6.36

include $(INCLUDE_DIR)/target.mk
DEFAULT_PACKAGES+=\
	kmod-input-core kmod-input-polldev \
	wpad-mini 

define Target/Description
	Build firmware images for Ralink RT288x/RT3xxx based boards.
endef

$(eval $(call BuildTarget))
