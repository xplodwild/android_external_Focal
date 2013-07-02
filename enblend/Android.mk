# Makefile for enblend suite

######################################################
###                     enblend                     ##
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
                  src/tiff_message.cc

LOCAL_CPP_EXTENSION := cc

LOCAL_SDK_VERSION := 14
LOCAL_NDK_STL_VARIANT := gnustl_static

				  
LOCAL_SRC_FILES:= $(enblend_SOURCES)
 
LOCAL_SHARED_LIBRARIES := libvigraimpex
LOCAL_STATIC_LIBRARIES := liblcms2 libgsl liblayersel
 
LOCAL_C_INCLUDES := $(LOCAL_PATH) \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/src/layer_selection/ \
	$(LOCAL_PATH)/../gsl \
	$(LOCAL_PATH)/../boost_1_53_0 \
	$(LOCAL_PATH)/../libtiff \
	$(LOCAL_PATH)/../lcms2/include \
	$(LOCAL_PATH)/../vigra/include
 
LOCAL_CFLAGS := -DAVOID_TABLES  -O3 -fexceptions -fstrict-aliasing -fprefetch-loop-arrays  -DANDROID \
        -D__Ansi__ -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64

LOCAL_LDLIBS := 
LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_STATIC_LIBRARY)
LOCAL_MODULE_TAGS := debug

LOCAL_MODULE := enblend

include $(BUILD_EXECUTABLE)

######################################################
###                 liblayersel                     ##
######################################################
 
LOCAL_PATH := $(my-dir)
include $(CLEAR_VARS)

liblayersel_a_SOURCES = src/layer_selection/info.cc \
                        src/layer_selection/layer_selection.cc \
                        src/layer_selection/selector.cc
				  
LOCAL_SRC_FILES:= $(liblayersel_a_SOURCES)
 
LOCAL_SHARED_LIBRARIES := 
 
LOCAL_C_INCLUDES := $(LOCAL_PATH) \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/../boost_1_53_0 \
	$(LOCAL_PATH)/../vigra/include
 
LOCAL_CFLAGS := -DAVOID_TABLES  -O3 -fexceptions -fstrict-aliasing -fprefetch-loop-arrays  -DANDROID \
        -D__Ansi__ -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64

LOCAL_LDLIBS := 
LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_STATIC_LIBRARY)
LOCAL_MODULE_TAGS := debug

LOCAL_MODULE := liblayersel

include $(BUILD_STATIC_LIBRARY)

