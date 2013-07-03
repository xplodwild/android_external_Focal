# Makefile for boost
# It's prebuilt, because it's super boring to build on Android


##
# date_time
##
LOCAL_PATH := $(my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := libboost_date_time-gcc-mt-1_53
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_SRC_FILES := lib/libboost_date_time-gcc-mt-1_53.a
LOCAL_C_INCLUDES := $(LOCAL_PATH)

include $(BUILD_PREBUILT)

##
# filesystem
##
include $(CLEAR_VARS)

LOCAL_MODULE := libboost_filesystem-gcc-mt-1_53
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_SRC_FILES := lib/libboost_filesystem-gcc-mt-1_53.a
LOCAL_C_INCLUDES := $(LOCAL_PATH)

include $(BUILD_PREBUILT)

##
# iostreams
##
include $(CLEAR_VARS)

LOCAL_MODULE := libboost_iostreams-gcc-mt-1_53
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_SRC_FILES := lib/libboost_iostreams-gcc-mt-1_53.a
LOCAL_C_INCLUDES := $(LOCAL_PATH)

include $(BUILD_PREBUILT)

##
# program_options
##
include $(CLEAR_VARS)

LOCAL_MODULE := libboost_program_options-gcc-mt-1_53
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_SRC_FILES := lib/libboost_program_options-gcc-mt-1_53.a
LOCAL_C_INCLUDES := $(LOCAL_PATH)

include $(BUILD_PREBUILT)

##
# regex
##
include $(CLEAR_VARS)

LOCAL_MODULE := libboost_regex-gcc-mt-1_53
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_SRC_FILES := lib/libboost_regex-gcc-mt-1_53.a
LOCAL_C_INCLUDES := $(LOCAL_PATH)

include $(BUILD_PREBUILT)

##
# signals
##
include $(CLEAR_VARS)

LOCAL_MODULE := libboost_signals-gcc-mt-1_53
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_SRC_FILES := lib/libboost_signals-gcc-mt-1_53.a
LOCAL_C_INCLUDES := $(LOCAL_PATH)

include $(BUILD_PREBUILT)

##
# system
##
include $(CLEAR_VARS)

LOCAL_MODULE := libboost_system-gcc-mt-1_53
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_SRC_FILES := lib/libboost_system-gcc-mt-1_53.a
LOCAL_C_INCLUDES := $(LOCAL_PATH)

include $(BUILD_PREBUILT)

##
# thread
##
include $(CLEAR_VARS)

LOCAL_MODULE := libboost_thread-gcc-mt-1_53
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_SRC_FILES := lib/libboost_thread-gcc-mt-1_53.a
LOCAL_C_INCLUDES := $(LOCAL_PATH)

include $(BUILD_PREBUILT)
