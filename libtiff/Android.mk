LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libtiffdecoder

LOCAL_CFLAGS := -DANDROID_NDK

LOCAL_SRC_FILES := \
	tiffdecoder.c

LOCAL_LDLIBS := -ldl -llog

LOCAL_SHARED_LIBRARIES := libtiff liblog

include $(BUILD_SHARED_LIBRARY)

include $(LOCAL_PATH)/tiff/Android.mk
