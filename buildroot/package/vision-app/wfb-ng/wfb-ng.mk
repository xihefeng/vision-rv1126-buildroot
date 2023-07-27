################################################################################
#
# 
#
################################################################################

WFB_NG_SITE = $(TOPDIR)/../vision-app/wfb-ng
WFB_NG_VERSION = release
WFB_NG_SITE_METHOD = local
WFB_NG_DEPENDENCIES = host-pkgconf libsodium libpcap

WFB_NG_CONF_OPTS += "-DBUILD_PC=OFF"

# Build TX
ifeq ($(BR2_PACKAGE_WFB_NG_TX),y)
WFB_NG_CONF_OPTS += "-DBUILD_TX=ON"
else
WFB_NG_CONF_OPTS += "-DBUILD_TX=OFF"
endif

# Build RX
ifeq ($(BR2_PACKAGE_WFB_NG_RX),y)
WFB_NG_CONF_OPTS += "-DBUILD_RX=ON"
else
WFB_NG_CONF_OPTS += "-DBUILD_RX=OFF"
endif

# Build Keygen
ifeq ($(BR2_PACKAGE_WFB_NG_KEYGEN),y)
WFB_NG_CONF_OPTS += "-DBUILD_KEYGEN=ON"
else
WFB_NG_CONF_OPTS += "-DBUILD_KEYGEN=OFF"
endif

# Copy keys
ifeq ($(BR2_PACKAGE_WFB_NG_KEYS),y)
WFB_NG_CONF_OPTS += "-DCOPY_KEYS=ON"
else
WFB_NG_CONF_OPTS += "-DCOPY_KEYS=OFF"
endif

WFB_NG_INSTALL_STAGING = NO
WFB_NG_INSTALL_TARGET = YES

$(eval $(cmake-package))