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
LOCAL_STATIC_LIBRARIES := libpng libtiff #libsimd
LOCAL_LDLIBS := -lz

LOCAL_C_INCLUDES := $(LOCAL_PATH) \
	$(LOCAL_PATH)/../libtiff \
	$(LOCAL_PATH)/../libjpeg-turbo \
	$(LOCAL_PATH)/../libpng \
	external/zlib

LOCAL_CFLAGS := -DAVOID_TABLES -O3 \
	-DANDROID_TILE_BASED_DECODE -DENABLE_ANDROID_NULL_CONVERT -D__Ansi__

LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_STATIC_LIBRARY)
LOCAL_MODULE_TAGS := debug

LOCAL_MODULE := libpano13

include $(BUILD_SHARED_LIBRARY)
