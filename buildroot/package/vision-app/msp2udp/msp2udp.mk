################################################################################
#
# msp to udp
#
################################################################################

MSP2UDP_SITE = $(TOPDIR)/../vision-app/msp2udp
MSP2UDP_VERSION = release
MSP2UDP_SITE_METHOD = local
MSP2UDP_DEPENDENCIES = host-pkgconf

MSP2UDP_INSTALL_STAGING = NO
MSP2UDP_INSTALL_TARGET = YES

$(eval $(cmake-package))