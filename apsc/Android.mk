# Makefile for autopano tools

######################################################
###                   autopano                      ##
######################################################

LOCAL_PATH := $(my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	LoweDetector.c RANSAC.c GaussianConvolution.c \
	ScaleSpace.c KeypointXML.c MatchKeys.c KDTree.c BondBall.c \
	AreaFilter.c ImageMatchModel.c Transform.c DisplayImage.c ImageMap.c \
	HashTable.c ArrayList.c Random.c SimpleMatrix.c Utils.c \
	AutoPano.c

LOCAL_SHARED_LIBRARIES := libtiffdecoder libpano13

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

LOCAL_STATIC_LIBRARIES := libxml2
ifeq ($(ANDROID_BUILD_TOP),)
LOCAL_STATIC_LIBRARIES += libiconv
else
LOCAL_SHARED_LIBRARIES += libicuuc
endif

LOCAL_CFLAGS := -DAVOID_TABLES  -O3 -fexceptions -fstrict-aliasing -fprefetch-loop-arrays  -D__Ansi__ -DHAS_PANO13

LOCAL_MODULE_TAGS := debug

LOCAL_MODULE := autopano

include $(BUILD_EXECUTABLE)


