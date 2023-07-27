################################################################################
#
# msp to udp
#
################################################################################

MSP_SERVICE_SITE = $(TOPDIR)/../vision-app/msp
MSP_SERVICE_VERSION = release
MSP_SERVICE_SITE_METHOD = local
MSP_SERVICE_DEPENDENCIES = host-pkgconf

MSP_SERVICE_INSTALL_STAGING = NO
MSP_SERVICE_INSTALL_TARGET = YES

$(eval $(cmake-package))