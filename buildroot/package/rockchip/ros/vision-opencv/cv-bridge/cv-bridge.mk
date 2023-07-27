#default to KINETIC
CV_BRIDGE_VERSION = 1.12.8
ifeq ($(BR2_PACKAGE_ROS_INDIGO),y)
CV_BRIDGE_VERSION = 1.11.16
endif

CV_BRIDGE_SITE = $(call github,ros-perception,vision_opencv,$(CV_BRIDGE_VERSION))
CV_BRIDGE_SUBDIR = cv_bridge

CV_BRIDGE_DEPENDENCIES += boost rosconsole sensor-msgs host-python-numpy opencv3

ifeq ($(BR2_PACKAGE_OPENCV),y)
# The samples are using opencv instead of opencv3
CV_BRIDGE_DEPENDENCIES += opencv
else ifeq ($(BR2_PACKAGE_OPENCV3),y)
CV_BRIDGE_DEPENDENCIES += opencv3
else ifeq ($(BR2_PACKAGE_OPENCV4),y)
CV_BRIDGE_DEPENDENCIES += opencv4
endif

$(eval $(catkin-package))
