# Makefile for vigra

######################################################
###                libvigraimpex                    ##
######################################################

LOCAL_PATH := $(my-dir)

include $(CLEAR_VARS)

libvigraimpex_SOURCES_DIST = src/impex/bmp.cxx \
	src/impex/byteorder.cxx \
	src/impex/codecmanager.cxx \
	src/impex/exr.cxx \
	src/impex/gif.cxx \
	src/impex/hdr.cxx \
	src/impex/hdf5impex.cxx \
	src/impex/hdf5_rf_impex.cxx \
	src/impex/iccjpeg.c \
	src/impex/imageinfo.cxx \
	src/impex/jpeg.cxx \
	src/impex/png.cxx \
	src/impex/pnm.cxx \
	src/impex/rgbe.c \
	src/impex/sifImport.cxx \
	src/impex/sun.cxx \
	src/impex/tiff.cxx \
	src/impex/viff.cxx \
	src/impex/void_vector.cxx

LOCAL_CPP_EXTENSION := .cxx

LOCAL_SDK_VERSION := 14
LOCAL_NDK_STL_VARIANT := gnustl_static

LOCAL_SRC_FILES:= $(libvigraimpex_SOURCES_DIST)

LOCAL_SHARED_LIBRARIES := libjpeg
LOCAL_STATIC_LIBRARIES := libpng libtiff

LOCAL_C_INCLUDES := $(LOCAL_PATH) \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/../libtiff

ifeq ($(ANDROID_BUILD_TOP),)
LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../libjpeg-turbo \
	$(LOCAL_PATH)/../libpng
else
LOCAL_C_INCLUDES += \
	external/jpeg \
	external/libpng
endif

LOCAL_CFLAGS := -DAVOID_TABLES  -O3 -fexceptions -fstrict-aliasing -fprefetch-loop-arrays \
	-D__Ansi__

LOCAL_LDLIBS := -lz
LOCAL_MODULE_TAGS := debug

LOCAL_MODULE := libvigraimpex

include $(BUILD_SHARED_LIBRARY)
