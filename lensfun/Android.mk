LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := liblensfun

LOCAL_SRC_FILES := libs/lensfun/camera.cpp libs/lensfun/database.cpp libs/lensfun/lens.cpp \
	libs/lensfun/mount.cpp libs/lensfun/cpuid.cpp \
	libs/lensfun/mod-color.cpp \
	libs/lensfun/mod-coord.cpp \
	libs/lensfun/mod-subpix.cpp libs/lensfun/modifier.cpp libs/lensfun/auxfun.cpp

LOCAL_LDLIBS := -lz
LOCAL_SHARED_LIBRARIES := libz
LOCAL_ARM_MODE := arm

LOCAL_C_INCLUDES := external/zlib \
	$(LOCAL_PATH)/include/lensfun \
	$(LOCAL_PATH)/../glib/glib \
	$(LOCAL_PATH)/../glib/android \
	$(LOCAL_PATH)/../glib/

LOCAL_NDK_STL_VARIANT := gnustl_static

-include external/Nemesis/gnustl.mk

include $(BUILD_STATIC_LIBRARY)
