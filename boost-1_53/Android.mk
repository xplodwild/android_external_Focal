# Makefile for boost
# It's prebuilt, because it's super boring to build on Android

ifeq ($(ANDROID_BUILD_TOP),)
NEMESIS_PREBUILT_STATIC_LIB=$(PREBUILT_STATIC_LIBRARY)
else
NEMESIS_PREBUILT_STATIC_LIB=$(BUILD_PREBUILT)
endif

##
# date_time
##
LOCAL_PATH := $(my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := libboost_date_time-gcc-mt-1_53
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MODULE_SUFFIX := .a
LOCAL_SRC_FILES := lib/libboost_date_time-gcc-mt-1_53.a
LOCAL_C_INCLUDES := $(LOCAL_PATH)

include $(NEMESIS_PREBUILT_STATIC_LIB)

##
# filesystem
##
include $(CLEAR_VARS)

LOCAL_MODULE := libboost_filesystem-gcc-mt-1_53
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MODULE_SUFFIX := .a
LOCAL_SRC_FILES := lib/libboost_filesystem-gcc-mt-1_53.a
LOCAL_C_INCLUDES := $(LOCAL_PATH)

include $(NEMESIS_PREBUILT_STATIC_LIB)

##
# iostreams
##
include $(CLEAR_VARS)

LOCAL_MODULE := libboost_iostreams-gcc-mt-1_53
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MODULE_SUFFIX := .a
LOCAL_SRC_FILES := lib/libboost_iostreams-gcc-mt-1_53.a
LOCAL_C_INCLUDES := $(LOCAL_PATH)

include $(NEMESIS_PREBUILT_STATIC_LIB)

##
# program_options
##
include $(CLEAR_VARS)

LOCAL_MODULE := libboost_program_options-gcc-mt-1_53
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MODULE_SUFFIX := .a
LOCAL_SRC_FILES := lib/libboost_program_options-gcc-mt-1_53.a
LOCAL_C_INCLUDES := $(LOCAL_PATH)

include $(NEMESIS_PREBUILT_STATIC_LIB)

##
# regex
##
include $(CLEAR_VARS)

LOCAL_MODULE := libboost_regex-gcc-mt-1_53
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MODULE_SUFFIX := .a
LOCAL_SRC_FILES := lib/libboost_regex-gcc-mt-1_53.a
LOCAL_C_INCLUDES := $(LOCAL_PATH)

include $(NEMESIS_PREBUILT_STATIC_LIB)

##
# signals
##
include $(CLEAR_VARS)

LOCAL_MODULE := libboost_signals-gcc-mt-1_53
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MODULE_SUFFIX := .a
LOCAL_SRC_FILES := lib/libboost_signals-gcc-mt-1_53.a
LOCAL_C_INCLUDES := $(LOCAL_PATH)

include $(NEMESIS_PREBUILT_STATIC_LIB)

##
# system
##
include $(CLEAR_VARS)

LOCAL_MODULE := libboost_system-gcc-mt-1_53
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MODULE_SUFFIX := .a
LOCAL_SRC_FILES := lib/libboost_system-gcc-mt-1_53.a
LOCAL_C_INCLUDES := $(LOCAL_PATH)

include $(NEMESIS_PREBUILT_STATIC_LIB)

##
# thread
##
include $(CLEAR_VARS)

LOCAL_MODULE := libboost_thread-gcc-mt-1_53
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MODULE_SUFFIX := .a
LOCAL_SRC_FILES := lib/libboost_thread-gcc-mt-1_53.a
LOCAL_C_INCLUDES := $(LOCAL_PATH)

include $(NEMESIS_PREBUILT_STATIC_LIB)
