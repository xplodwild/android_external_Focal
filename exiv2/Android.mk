# Makefile for libexiv2

######################################################
###               libexiv2.so                       ##
######################################################

LOCAL_PATH := $(my-dir)
include $(CLEAR_VARS)

CCSRC := \
	src/basicio.cpp \
	src/bmpimage.cpp \
	src/canonmn.cpp \
	src/convert.cpp \
	src/cr2image.cpp \
	src/crwimage.cpp \
	src/datasets.cpp \
	src/easyaccess.cpp \
	src/epsimage.cpp \
	src/error.cpp \
	src/exif.cpp \
	src/futils.cpp \
	src/fujimn.cpp \
	src/gifimage.cpp \
	src/image.cpp \
	src/iptc.cpp \
	src/jp2image.cpp \
	src/jpgimage.cpp \
	src/makernote.cpp \
	src/metadatum.cpp \
	src/minoltamn.cpp \
	src/mrwimage.cpp \
	src/nikonmn.cpp \
	src/olympusmn.cpp \
	src/orfimage.cpp \
	src/panasonicmn.cpp \
	src/pgfimage.cpp

CCSRC += \
	src/pngimage.cpp \
	src/pngchunk.cpp

CCSRC += \
	src/preview.cpp \
	src/properties.cpp \
	src/psdimage.cpp \
	src/rafimage.cpp \
	src/rw2image.cpp \
	src/samsungmn.cpp \
	src/sigmamn.cpp \
	src/pentaxmn.cpp \
	src/sonymn.cpp \
	src/tags.cpp \
	src/tgaimage.cpp \
	src/tiffcomposite.cpp \
	src/tiffimage.cpp \
	src/tiffvisitor.cpp \
	src/types.cpp \
	src/value.cpp \
	src/version.cpp \
	src/xmp.cpp \
	src/xmpsidecar.cpp

LOCAL_SRC_FILES:= $(CCSRC)

LOCAL_NDK_STL_VARIANT := gnustl_static

LOCAL_SHARED_LIBRARIES :=
LOCAL_STATIC_LIBRARIES := 
LOCAL_LDLIBS := -lz -lc

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH) \
	$(LOCAL_PATH)/../libtiff \
	external/zlib

LOCAL_CFLAGS := -DAVOID_TABLES -O3 \
	-DANDROID_TILE_BASED_DECODE -DENABLE_ANDROID_NULL_CONVERT -D__Ansi__ \
	-fexceptions -frtti

LOCAL_MODULE_TAGS := debug

LOCAL_MODULE := libexiv2

LOCAL_ARM_MODE := arm

-include external/Nemesis/gnustl.mk

include $(BUILD_SHARED_LIBRARY)
