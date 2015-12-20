#
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=pppoe
PKG_VERSION:=3.6
PKG_RELEASE:=1

PKG_LICENSE:=GPLv2 LGPLv2.1
PKG_LICENSE_FILES:=

PAPS=pap-secrets
OPTIONS=pppoe-server-options
APP=pppoe-server
PKG_MAINTAINER:=Felix Fietkau <nbd@openwrt.org>

include $(INCLUDE_DIR)/package.mk

define Package/pppoe
  SECTION:=net
  CATEGORY:=Network
  TITLE:=pppoe
  DEPENDS:=ppp
endef

define Package/pppoe/description
 This package contains a stripped down version of libnl
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

#TARGET_CFLAGS += $(FPIC)

define Build/Compile
	$(MAKE) -C src \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		all
endef


define Package/pppoe/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_DIR) $(1)/etc/ppp/
	$(CP) src/$(APP) $(1)/bin/
	$(CP) src/$(OPTIONS) $(1)/etc/ppp/
	$(CP) src/$(PAPS) $(1)/etc/ppp/
endef

$(eval $(call BuildPackage,pppoe))
