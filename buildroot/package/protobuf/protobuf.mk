################################################################################
#
# protobuf
#
################################################################################

# When bumping this package, make sure to also verify if the
# python-protobuf package still works, as they share the same
# version/site variables.
PROTOBUF_VERSION = 3.6.0
PROTOBUF_SOURCE = protobuf-cpp-$(PROTOBUF_VERSION).tar.gz
PROTOBUF_SITE = https://github.com/google/protobuf/releases/download/v$(PROTOBUF_VERSION)
PROTOBUF_LICENSE = BSD-3-Clause
PROTOBUF_LICENSE_FILES = LICENSE

# N.B. Need to use host protoc during cross compilation.
PROTOBUF_DEPENDENCIES = host-protobuf
PROTOBUF_CONF_OPTS = --with-protoc=$(HOST_DIR)/bin/protoc

ifeq ($(BR2_TOOLCHAIN_HAS_LIBATOMIC),y)
PROTOBUF_CONF_ENV += LIBS=-latomic
endif

PROTOBUF_INSTALL_STAGING = YES

ifeq ($(BR2_PACKAGE_ZLIB),y)
PROTOBUF_DEPENDENCIES += zlib
endif

define PROTOBUF_REMOVE_EXTRA_STUFF
	# rm -f $(TARGET_DIR)/usr/lib/libprotobuf.a
	# rm -f $(TARGET_DIR)/usr/lib/libprotoc.a
	# rm -f $(TARGET_DIR)/usr/lib/libprotobuf.so*
	# rm -f $(TARGET_DIR)/usr/lib/libprotoc.so*
endef
PROTOBUF_POST_INSTALL_TARGET_HOOKS += PROTOBUF_REMOVE_EXTRA_STUFF

$(eval $(autotools-package))
$(eval $(host-autotools-package))
