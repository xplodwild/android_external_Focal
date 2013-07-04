# Makefile for libpano13

######################################################
###              libpano13.so                       ##
######################################################

LOCAL_PATH := $(my-dir)
include $(CLEAR_VARS)

libpano13_SOURCES_DIST = javastub.c \
	adjust.c \
	bmp.c \
	ColourBrightness.c \
	correct.c \
	dump.c \
	fftn.c \
	file.c \
	filter.c \
	fourier.c \
	hdrfile.c \
	jpeg.c \
	jpegicc.c \
	lmdif.c \
	math.c \
	metadata.c \
	morpher.c \
	multilayer.c \
	optimize.c \
	pan.c \
	parser.c \
	perspect.c \
	png.c \
	ppm.c \
	PTcommon.c \
	PTDialogs.c \
	ptfeather.c \
	ptstitch.c \
	queryfeature.c \
	remap.c \
	resample.c \
	rgbe.c \
	seamer.c \
	sys_ansi.c \
	sys_compat_unix.c \
	sys_common.c \
	tiff.c \
	Triangulate.c \
	ZComb.c \
	PaniniGeneral.c

LOCAL_SRC_FILES:= $(libpano13_SOURCES_DIST)

LOCAL_SHARED_LIBRARIES := libjpeg libz
LOCAL_STATIC_LIBRARIES := libpng libtiff_static #libsimd
LOCAL_LDLIBS := -lz

LOCAL_C_INCLUDES := $(LOCAL_PATH) \
	$(LOCAL_PATH)/../libtiff \
	external/zlib

ifeq ($(ANDROID_BUILD_TOP),)
LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../libjpeg-turbo \
	$(LOCAL_PATH)/../libpng
else
LOCAL_C_INCLUDES += \
	external/jpeg \
	external/libpng
endif

LOCAL_CFLAGS := -DAVOID_TABLES -O3 \
	-DANDROID_TILE_BASED_DECODE -DENABLE_ANDROID_NULL_CONVERT -D__Ansi__

LOCAL_MODULE_TAGS := debug

LOCAL_MODULE := libpano13

include $(BUILD_SHARED_LIBRARY)

######################################################
###                  PTblender                      ##
######################################################

include $(CLEAR_VARS)

LOCAL_SRC_FILES := tools/PTblender.c

LOCAL_C_INCLUDES := $(LOCAL_PATH) \
	$(LOCAL_PATH)/../vigra/include \
	$(LOCAL_PATH)/../boost-1_53/ \
	$(LOCAL_PATH)/../exiv2/include \
	$(LOCAL_PATH)/../libpano13 \
	$(LOCAL_PATH)/../lensfun/include/lensfun \
	$(LOCAL_PATH)/../libtiff

LOCAL_SHARED_LIBRARIES := libvigraimpex libpano13
LOCAL_STATIC_LIBRARIES := libhugin

LOCAL_MODULE := PTblender

#include $(BUILD_EXECUTABLE)
