# Makefile for liblcms2

######################################################
###                 liblcms2.a                      ##
######################################################

LOCAL_PATH := $(my-dir)
include $(CLEAR_VARS)

liblcms2_la_SOURCES = \
	src/cmscnvrt.c src/cmserr.c src/cmsgamma.c src/cmsgmt.c src/cmsintrp.c \
	src/cmsio0.c src/cmsio1.c src/cmslut.c \
	src/cmsplugin.c src/cmssm.c src/cmsmd5.c src/cmsmtrx.c src/cmspack.c src/cmspcs.c \
	src/cmswtpnt.c src/cmsxform.c \
	src/cmssamp.c src/cmsnamed.c src/cmscam02.c src/cmsvirt.c \
	src/cmstypes.c src/cmscgats.c src/cmsps2.c src/cmsopt.c \
	src/cmshalf.c

LOCAL_SRC_FILES:= $(liblcms2_la_SOURCES)

LOCAL_C_INCLUDES := $(LOCAL_PATH) \
	$(LOCAL_PATH)/include

LOCAL_CFLAGS := -DAVOID_TABLES -O3 -fstrict-aliasing -fprefetch-loop-arrays \
	-DANDROID_TILE_BASED_DECODE -DENABLE_ANDROID_NULL_CONVERT -D__Ansi__

LOCAL_LDLIBS := -lz

LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_STATIC_LIBRARY)
LOCAL_MODULE_TAGS := debug

LOCAL_MODULE := liblcms2

include $(BUILD_STATIC_LIBRARY)
