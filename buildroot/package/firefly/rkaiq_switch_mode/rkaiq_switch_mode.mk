RKAIQ_SWITCH_MODE_VERSION = 1.0.1
RKAIQ_SWITCH_MODE_LICENSE = GPL-2.0
RKAIQ_SWITCH_MODE_VERSION = master
RKAIQ_SWITCH_MODE_SITE_METHOD = local
RKAIQ_SWITCH_MODE_SITE = $(TOPDIR)/../app/rkaiq_switch_mode
RKAIQ_SWITCH_MODE_INSTALL_STAGING = YES 


define RKAIQ_SWITCH_MODE_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)

endef

define RKAIQ_SWITCH_MODE_INSTALL_TARGET_CMDS
	
        $(INSTALL) -D -m 0755 $(@D)/rkaiq_switch_mode $(TARGET_DIR)/usr/bin/
        $(INSTALL) -D -m 0755 $(@D)/check_brightness.sh $(TARGET_DIR)/usr/bin/
        $(INSTALL) -D -m 0755 $(@D)/S49_* $(TARGET_DIR)/etc/init.d/
        $(INSTALL) -D -m 0755 $(@D)/.ffswitchmode_config $(TARGET_DIR)/etc/init.d/
endef

$(eval $(generic-package))
