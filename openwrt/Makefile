include $(TOPDIR)/rules.mk

PKG_NAME:=watercounter
PKG_VERSION:=0.0.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_SOURCE_URL:=https://github.com/Shulyaka/watercounter
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=HEAD

include $(INCLUDE_DIR)/package.mk

define Package/watercounter
	SECTION:=base
	CATEGORY:=Utilities
	TITLE:=Water counter telemetry
	URL:=$(PKG_SOURCE_URL)
	DEPENDS:=+collectd-mod-exec +luci-app-statistics
endef

define Package/watercounter/description
	Water counter telemetry and water valve control
endef

define Package/watercounter/conffiles
/etc/watercounter/counter0_hot_16-046903
/etc/watercounter/counter1_toilet_15-382336
/etc/watercounter/counter24_cold_15-382321
endef

define Package/watercounter/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/watercounter $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/valve $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/counter $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/hot $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/cold $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/toilet $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/watercounter
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/statistics/rrdtool/definitions/
	$(INSTALL_DATA) ./files/exec.lua $(1)/usr/lib/lua/luci/statistics/rrdtool/definitions/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/etc/watercounter/counter0_hot_16-046903 $(1)/etc/watercounter/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/etc/watercounter/counter1_toilet_15-382336 $(1)/etc/watercounter/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/etc/watercounter/counter24_cold_15-382321 $(1)/etc/watercounter/
endef

$(eval $(call BuildPackage,watercounter))
