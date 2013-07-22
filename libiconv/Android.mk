LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS) 

LOCAL_MODULE := libiconv

LOCAL_CFLAGS := \
	-Wno-multichar \
	-DLIBDIR="c" \
	-DBUILDING_LIBICONV \
	-DIN_LIBRARY

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH) \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/srclib

LOCAL_SRC_FILES := \
	lib/iconv.c \
	libcharset/lib/localcharset.c \
	libcharset/lib/relocatable.c

LOCAL_ARM_MODE := arm
LOCAL_LDLIBS := -llog

include $(BUILD_STATIC_LIBRARY)
