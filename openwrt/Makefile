include $(TOPDIR)/rules.mk

PKG_NAME:=watercounter
PKG_VERSION:=0.0.2
PKG_RELEASE:=6

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
/etc/watercounter/counters.sh
endef

define Package/watercounter/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/watercounter $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/valve $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/counter $(1)/usr/bin/
	ln -s $(PKG_BUILD_DIR)/bin/hot counter
	ln -s $(PKG_BUILD_DIR)/bin/cold counter
	ln -s $(PKG_BUILD_DIR)/bin/toilet counter
	$(INSTALL_DIR) $(1)/etc/watercounter
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/statistics/rrdtool/definitions/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/etc/watercounter/counter0_hot_16-046903 $(1)/etc/watercounter/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/etc/watercounter/counter1_toilet_15-382336 $(1)/etc/watercounter/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/etc/watercounter/counter24_cold_15-382321 $(1)/etc/watercounter/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/etc/watercounter/counters.sh $(1)/etc/watercounter/
	$(INSTALL_DATA) ./files/exec.lua $(1)/usr/lib/lua/luci/statistics/rrdtool/definitions/
	$(INSTALL_DIR) $(1)/www
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/www/index.html $(1)/www/watercounter.html
	$(INSTALL_DIR) $(1)/www/cgi-bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/www/cgi-bin/wcounter.sh $(1)/www/cgi-bin/wcounter.sh
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/www/cgi-bin/wcontrol.sh $(1)/www/cgi-bin/wcontrol.sh
	$(INSTALL_DIR) $(1)/www/img
	# Removed to save the space, please copy those yourself:
	# $(INSTALL_BIN) $(PKG_BUILD_DIR)/www/img/background.jpg $(1)/www/img/background.jpg
	# $(INSTALL_BIN) $(PKG_BUILD_DIR)/www/img/counter-bottom-cold.png $(1)/www/img/counter-bottom-cold.png
	# $(INSTALL_BIN) $(PKG_BUILD_DIR)/www/img/counter-bottom-hot.png $(1)/www/img/counter-bottom-hot.png
	# $(INSTALL_BIN) $(PKG_BUILD_DIR)/www/img/counter-cold.png $(1)/www/img/counter-cold.png
	# $(INSTALL_BIN) $(PKG_BUILD_DIR)/www/img/counter-hot.png $(1)/www/img/counter-hot.png
	# $(INSTALL_BIN) $(PKG_BUILD_DIR)/www/img/counter-top-cold.png $(1)/www/img/counter-top-cold.png
	# $(INSTALL_BIN) $(PKG_BUILD_DIR)/www/img/counter-top-hot.png $(1)/www/img/counter-top-hot.png
	# $(INSTALL_BIN) $(PKG_BUILD_DIR)/www/img/pipe_bottom.png $(1)/www/img/pipe_bottom.png
	# $(INSTALL_BIN) $(PKG_BUILD_DIR)/www/img/pipe_top.png $(1)/www/img/pipe_top.png
	# $(INSTALL_BIN) $(PKG_BUILD_DIR)/www/img/stream1.png $(1)/www/img/stream1.png
	# $(INSTALL_BIN) $(PKG_BUILD_DIR)/www/img/stream2.png $(1)/www/img/stream2.png
	# $(INSTALL_BIN) $(PKG_BUILD_DIR)/www/img/stream3.png $(1)/www/img/stream3.png
	# $(INSTALL_BIN) $(PKG_BUILD_DIR)/www/img/stream4.png $(1)/www/img/stream4.png
	# $(INSTALL_BIN) $(PKG_BUILD_DIR)/www/img/stream5.png $(1)/www/img/stream5.png
	# $(INSTALL_BIN) $(PKG_BUILD_DIR)/www/img/valve.png $(1)/www/img/valve.png
	# $(INSTALL_BIN) $(PKG_BUILD_DIR)/www/img/water_on_glass_x3.png $(1)/www/img/water_on_glass_x3.png
endef

$(eval $(call BuildPackage,watercounter))
