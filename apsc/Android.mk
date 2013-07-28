# Makefile for autopano-sift-c tools

######################################################
###                   autopano                      ##
######################################################

LOCAL_PATH := $(my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	APSCpp/ANN/ANN.cpp APSCpp/ANN/brute.cpp \
	APSCpp/ANN/kd_tree.cpp APSCpp/ANN/kd_util.cpp APSCpp/ANN/kd_split.cpp \
	APSCpp/ANN/kd_dump.cpp APSCpp/ANN/kd_search.cpp APSCpp/ANN/kd_pr_search.cpp \
	APSCpp/ANN/kd_fix_rad_search.cpp APSCpp/ANN/bd_tree.cpp APSCpp/ANN/bd_search.cpp \
	APSCpp/ANN/bd_pr_search.cpp APSCpp/ANN/bd_fix_rad_search.cpp \
	APSCpp/ANN/perf.cpp APSCpp/ANNkd_wrap.cpp \
	APSCpp/CamLens.c APSCpp/HermiteSpline.c APSCpp/saInterp.c \
	APSCpp/saRemap.c APSCpp/APSCpp_main.c APSCpp/APSCpp.c \
	LoweDetector.c RANSAC.c GaussianConvolution.c \
	ScaleSpace.c KeypointXML.c MatchKeys.c KDTree.c BondBall.c \
	AreaFilter.c ImageMatchModel.c Transform.c DisplayImage.c ImageMap.c \
	HashTable.c ArrayList.c Random.c SimpleMatrix.c Utils.c

LOCAL_NDK_STL_VARIANT := gnustl_static

LOCAL_SHARED_LIBRARIES := libtiffdecoder libpano13

LOCAL_C_INCLUDES := $(LOCAL_PATH) \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/../libtiff \
	$(LOCAL_PATH)/../libpano13 \
	$(LOCAL_PATH)/APSCpp

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
endif

LOCAL_CFLAGS := -DAVOID_TABLES  -O3 -fexceptions -fstrict-aliasing -fprefetch-loop-arrays  -D__Ansi__ -DHAS_PANO13

LOCAL_MODULE_TAGS := debug

LOCAL_MODULE := autopano
LOCAL_ARM_MODE := arm

-include external/Focal/gnustl.mk

include $(BUILD_EXECUTABLE)

