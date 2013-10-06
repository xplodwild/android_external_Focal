LOCAL_PATH:=$(call my-dir)

######################################################
###                  multiblend                     ##
######################################################

LOCAL_PATH := $(my-dir)
include $(CLEAR_VARS)


LOCAL_SRC_FILES := multiblend.cpp

LOCAL_SHARED_LIBRARIES := libjpeg libz
LOCAL_STATIC_LIBRARIES := libpng_static libtiff_static

LOCAL_C_INCLUDES := $(LOCAL_PATH) \
	$(LOCAL_PATH)/../boost-1_53 \
	$(LOCAL_PATH)/../libtiff \
	$(LOCAL_PATH)/../libpng \
	external/zlib


ifeq ($(ANDROID_BUILD_TOP),)
LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../libjpeg-turbo
else
LOCAL_C_INCLUDES += \
	external/jpeg
endif

LOCAL_NDK_STL_VARIANT := gnustl_static

LOCAL_CFLAGS := -O3 -Wno-sequence-point
#LOCAL_CFLAGS += -DDEBUG

LOCAL_LDLIBS := -lz

LOCAL_ARM_MODE := arm

LOCAL_MODULE := multiblend

-include external/Focal/gnustl.mk

include $(BUILD_EXECUTABLE)
