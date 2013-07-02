# Makefile for libgsl
 
######################################################
###                   libgsl.a                      ##
######################################################
 
LOCAL_PATH := $(my-dir)
include $(CLEAR_VARS)

libgsl_la_SOURCES = \
  version.c

LOCAL_SRC_FILES:= $(libgsl_la_SOURCES)
 
#LOCAL_SHARED_LIBRARIES := libjpeg libpng libtiff
#LOCAL_STATIC_LIBRARIES := libsimd
 
LOCAL_C_INCLUDES := $(LOCAL_PATH) \
	$(LOCAL_PATH)/include
 
LOCAL_CFLAGS := -DAVOID_TABLES  -O3 -fstrict-aliasing -fprefetch-loop-arrays  -DANDROID \
        -DANDROID_TILE_BASED_DECODE -DENABLE_ANDROID_NULL_CONVERT -D__Ansi__

LOCAL_LDLIBS := -lz
LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_STATIC_LIBRARY)
LOCAL_MODULE_TAGS := debug

LOCAL_MODULE := libgsl

include $(BUILD_STATIC_LIBRARY)