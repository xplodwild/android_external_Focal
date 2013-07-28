LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_CFLAGS := 

LOCAL_MODULE := libpng_static

LOCAL_NDK_STL_VARIANT := gnustl_static

LOCAL_SRC_FILES :=\
	png.c \
	pngerror.c \
	pngget.c \
	pngmem.c \
	pngpread.c \
	pngread.c \
	pngrio.c \
	pngrtran.c \
	pngrutil.c \
	pngset.c \
	pngtrans.c \
	pngwio.c \
	pngwrite.c \
	pngwtran.c \
	pngwutil.c 

LOCAL_LDLIBS := -lz
LOCAL_SHARED_LIBRARIES := libz
LOCAL_ARM_MODE := arm

LOCAL_C_INCLUDES := external/zlib

-include external/Focal/gnustl.mk

include $(BUILD_STATIC_LIBRARY)
