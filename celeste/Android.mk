# Makefile for hugin's celeste

######################################################
###                  celeste                        ##
######################################################
 
LOCAL_PATH := $(my-dir)
include $(CLEAR_VARS)

celeste_SOURCES = \
	CelesteGlobals.cpp \
	Celeste.cpp \
	ContrastFilter.cpp \
	Gabor.cpp \
	GaborFilter.cpp \
	GaborJet.cpp \
	ImageFile.cpp \
	LogPolar.cpp \
	PGMImage.cpp \
	svm.cpp \
	Utilities.cpp \
	Main.cpp

LOCAL_SRC_FILES:= $(celeste_SOURCES)

LOCAL_SHARED_LIBRARIES := libhugin libvigraimpex
LOCAL_STATIC_LIBRARIES := 

LOCAL_C_INCLUDES := $(LOCAL_PATH) \
	$(LOCAL_PATH)/../vigra/include \
	$(LOCAL_PATH)/../hugin \
	$(LOCAL_PATH)/../libpano13/ \
	$(LOCAL_PATH)/../libtiff/ \
	$(LOCAL_PATH)/../exiv2/include

LOCAL_CFLAGS := -O3 -fexceptions -fstrict-aliasing -fprefetch-loop-arrays \
	-D__Ansi__ -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64

LOCAL_LDLIBS := -lz
LOCAL_MODULE_TAGS := debug

LOCAL_MODULE := celeste

include $(BUILD_EXECUTABLE)

