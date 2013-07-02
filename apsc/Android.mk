# Makefile for autopano tools

######################################################
###                    libsift                      ##
######################################################

LOCAL_PATH := $(my-dir)
include $(CLEAR_VARS)

libsift_SOURCES = \
	LoweDetector.c RANSAC.c GaussianConvolution.c \
	ScaleSpace.c KeypointXML.c MatchKeys.c KDTree.c BondBall.c \
	AreaFilter.c ImageMatchModel.c Transform.c DisplayImage.c ImageMap.c \
	HashTable.c ArrayList.c Random.c SimpleMatrix.c Utils.c

LOCAL_SDK_VERSION := 14
LOCAL_NDK_STL_VARIANT := gnustl_static

LOCAL_SRC_FILES:= $(libsift_SOURCES)

LOCAL_C_INCLUDES := $(LOCAL_PATH) \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/../libtiff \
	$(LOCAL_PATH)/../libpano13

ifeq ($(ANDROID_BUILD_TOP),)
LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../libxml2/include \
	$(LOCAL_PATH)/../libiconv/include
else
LOCAL_C_INCLUDES += \
	external/libxml2/include \
	external/icu4c/common
endif

LOCAL_CFLAGS := -DAVOID_TABLES -O3 -fexceptions -fstrict-aliasing -fprefetch-loop-arrays \
	-D__Ansi__ -D_GNU_SOURCE -DHAS_PANO13

LOCAL_STATIC_LIBRARIES := libxml2 libiconv

LOCAL_LDLIBS := -lxml
LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_STATIC_LIBRARY)
LOCAL_MODULE_TAGS := debug

LOCAL_MODULE := libsift

include $(BUILD_STATIC_LIBRARY)

######################################################
###                   autopano                      ##
######################################################
 
include $(CLEAR_VARS)

enblend_SOURCES = AutoPano.c

LOCAL_SDK_VERSION := 14

LOCAL_SRC_FILES:= $(enblend_SOURCES)

LOCAL_SHARED_LIBRARIES := libtiffdecoder libpano13
LOCAL_STATIC_LIBRARIES := libsift

LOCAL_C_INCLUDES := $(LOCAL_PATH) \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/../libtiff \
	$(LOCAL_PATH)/../libpano13

ifeq ($(ANDROID_BUILD_TOP),)
LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../libxml2/include \
	$(LOCAL_PATH)/../libiconv/include
else
LOCAL_C_INCLUDES += \
	external/libxml2/include \
	external/icu4c/common
endif

LOCAL_CFLAGS := -DAVOID_TABLES  -O3 -fexceptions -fstrict-aliasing -fprefetch-loop-arrays  -D__Ansi__ -DHAS_PANO13

LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_STATIC_LIBRARY)
LOCAL_MODULE_TAGS := debug

LOCAL_MODULE := autopano

include $(BUILD_EXECUTABLE)


