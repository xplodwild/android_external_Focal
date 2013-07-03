# Makefile for enblend suite

######################################################
###                  enblend                        ##
######################################################
 
LOCAL_PATH := $(my-dir)
include $(CLEAR_VARS)

enblend_SOURCES = \
	src/enblend.cc \
	src/gpu.cc \
	src/error_message.cc \
	src/filenameparse.cc \
	src/filespec.cc \
	src/self_test.cc \
	src/tiff_message.cc \
	src/layer_selection/info.cc \
	src/layer_selection/layer_selection.cc \
	src/layer_selection/selector.cc

LOCAL_CPP_EXTENSION := .cc

LOCAL_SDK_VERSION := 14
LOCAL_NDK_STL_VARIANT := gnustl_static

LOCAL_SRC_FILES:= $(enblend_SOURCES)

LOCAL_SHARED_LIBRARIES := libvigraimpex libtiffdecoder
LOCAL_STATIC_LIBRARIES := liblcms2 libgsl liblayersel

LOCAL_C_INCLUDES := $(LOCAL_PATH) \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/src/layer_selection/ \
	$(LOCAL_PATH)/../gsl \
	$(LOCAL_PATH)/../boost-1_53 \
	$(LOCAL_PATH)/../libtiff \
	$(LOCAL_PATH)/../lcms2/include \
	$(LOCAL_PATH)/../vigra/include
 
LOCAL_CFLAGS := -DAVOID_TABLES -O3 -fexceptions -fstrict-aliasing -fprefetch-loop-arrays \
	-D__Ansi__ -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64

LOCAL_LDLIBS := -lz
LOCAL_MODULE_TAGS := debug

LOCAL_MODULE := enblend

include $(BUILD_EXECUTABLE)

######################################################
##                   enfuse                         ##
######################################################
 
include $(CLEAR_VARS)

enfuse_SOURCES = \
	src/enfuse.cc \
	src/error_message.cc \
	src/filenameparse.cc \
	src/filespec.cc \
	src/self_test.cc \
	src/tiff_message.cc \
	src/layer_selection/info.cc \
	src/layer_selection/layer_selection.cc \
	src/layer_selection/selector.cc

LOCAL_CPP_EXTENSION := cc

LOCAL_SDK_VERSION := 14
LOCAL_NDK_STL_VARIANT := gnustl_static

LOCAL_SRC_FILES:= $(enfuse_SOURCES)

LOCAL_SHARED_LIBRARIES := libvigraimpex libtiffdecoder
LOCAL_STATIC_LIBRARIES := liblcms2 libgsl liblayersel

LOCAL_C_INCLUDES := $(LOCAL_PATH) \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/src/layer_selection/ \
	$(LOCAL_PATH)/../gsl \
	$(LOCAL_PATH)/../boost-1_53 \
	$(LOCAL_PATH)/../libtiff \
	$(LOCAL_PATH)/../lcms2/include \
	$(LOCAL_PATH)/../vigra/include
 
LOCAL_CFLAGS := -DAVOID_TABLES -O3 -fexceptions -fstrict-aliasing -fprefetch-loop-arrays \
	-D__Ansi__ -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64

LOCAL_MODULE_TAGS := debug

LOCAL_MODULE := enfuse

include $(BUILD_EXECUTABLE)

