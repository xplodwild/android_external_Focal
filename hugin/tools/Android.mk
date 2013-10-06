
LOCAL_PATH := $(my-dir)

######################################################
###                      ptclean                    ##
######################################################

include $(CLEAR_VARS)

LOCAL_SRC_FILES := cpclean.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH) \
	$(LOCAL_PATH)/../../vigra/include \
	$(LOCAL_PATH)/../../boost-1_53/ \
	$(LOCAL_PATH)/../../exiv2/include \
	$(LOCAL_PATH)/../../libpano13 \
	$(LOCAL_PATH)/..

LOCAL_SHARED_LIBRARIES := libpano13 libgmodule-2.0 \
	libgobject-2.0 libgthread-2.0 libglib-2.0 libvigraimpex \
	libexiv2
LOCAL_STATIC_LIBRARIES := libhugin liblensfun libboost_filesystem-gcc-mt-1_53 \
	libboost_system-gcc-mt-1_53

LOCAL_MODULE := ptclean

LOCAL_NDK_STL_VARIANT := gnustl_static

LOCAL_CFLAGS := -O3 -fexceptions -frtti

-include external/Focal/gnustl.mk

include $(BUILD_EXECUTABLE)

######################################################
###                      linefind                   ##
######################################################

include $(CLEAR_VARS)

LOCAL_SRC_FILES := linefind.cpp \
	../lines/FindLines.cpp \
	../lines/FindN8Lines.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH) \
	$(LOCAL_PATH)/../../vigra/include \
	$(LOCAL_PATH)/../../boost-1_53/ \
	$(LOCAL_PATH)/../../exiv2/include \
	$(LOCAL_PATH)/../../libpano13 \
	$(LOCAL_PATH)/../../libtiff \
	$(LOCAL_PATH)/..

LOCAL_SHARED_LIBRARIES := libpano13 libgmodule-2.0 \
	libgobject-2.0 libgthread-2.0 libglib-2.0 libvigraimpex \
	libexiv2
LOCAL_STATIC_LIBRARIES := libhugin liblensfun libboost_filesystem-gcc-mt-1_53 libboost_thread-gcc-mt-1_53 \
	libboost_system-gcc-mt-1_53

LOCAL_MODULE := linefind

LOCAL_NDK_STL_VARIANT := gnustl_static

LOCAL_CFLAGS := -O3 -fexceptions -frtti

-include external/Focal/gnustl.mk

#include $(BUILD_EXECUTABLE)

######################################################
###                autooptimiser                    ##
######################################################

include $(CLEAR_VARS)

LOCAL_SRC_FILES := autooptimiser.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH) \
	$(LOCAL_PATH)/../../vigra/include \
	$(LOCAL_PATH)/../../boost-1_53/ \
	$(LOCAL_PATH)/../../exiv2/include \
	$(LOCAL_PATH)/../../libpano13 \
	$(LOCAL_PATH)/../../libtiff \
	$(LOCAL_PATH)/..

LOCAL_SHARED_LIBRARIES := libpano13 libgmodule-2.0 \
	libgobject-2.0 libgthread-2.0 libglib-2.0 libvigraimpex \
	libexiv2
LOCAL_STATIC_LIBRARIES := libhugin liblensfun libboost_filesystem-gcc-mt-1_53 \
	libboost_system-gcc-mt-1_53

LOCAL_MODULE := autooptimiser

LOCAL_NDK_STL_VARIANT := gnustl_static

LOCAL_CFLAGS := -O3 -fexceptions -frtti -Wno-sign-compare -Wno-sign-promo -Wno-non-virtual-dtor

-include external/Focal/gnustl.mk

include $(BUILD_EXECUTABLE)

######################################################
###                  pano_modify                    ##
######################################################

include $(CLEAR_VARS)

LOCAL_SRC_FILES := pano_modify.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH) \
	$(LOCAL_PATH)/../../vigra/include \
	$(LOCAL_PATH)/../../boost-1_53/ \
	$(LOCAL_PATH)/../../exiv2/include \
	$(LOCAL_PATH)/../../libpano13 \
	$(LOCAL_PATH)/..

LOCAL_SHARED_LIBRARIES := libpano13 libgmodule-2.0 \
	libgobject-2.0 libgthread-2.0 libglib-2.0 libvigraimpex \
	libexiv2
LOCAL_STATIC_LIBRARIES := libhugin liblensfun libboost_filesystem-gcc-mt-1_53 \
	libboost_system-gcc-mt-1_53

LOCAL_MODULE := pano_modify

LOCAL_NDK_STL_VARIANT := gnustl_static

LOCAL_CFLAGS := -O3 -fexceptions -frtti

-include external/Focal/gnustl.mk

include $(BUILD_EXECUTABLE)

######################################################
###                    pto_gen                      ##
######################################################

include $(CLEAR_VARS)

LOCAL_SRC_FILES := pto_gen.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH) \
	$(LOCAL_PATH)/../../vigra/include \
	$(LOCAL_PATH)/../../boost-1_53/ \
	$(LOCAL_PATH)/../../exiv2/include \
	$(LOCAL_PATH)/../../libpano13 \
	$(LOCAL_PATH)/../../libtiff \
	$(LOCAL_PATH)/../../lensfun/include/lensfun \
	$(LOCAL_PATH)/..

LOCAL_SHARED_LIBRARIES := libpano13 libgmodule-2.0 \
	libgobject-2.0 libgthread-2.0 libglib-2.0 libvigraimpex \
	libexiv2
LOCAL_STATIC_LIBRARIES := libhugin liblensfun libboost_filesystem-gcc-mt-1_53 \
	libboost_system-gcc-mt-1_53

LOCAL_MODULE := pto_gen

LOCAL_NDK_STL_VARIANT := gnustl_static

LOCAL_CFLAGS := -O3 -fexceptions -frtti

-include external/Focal/gnustl.mk

include $(BUILD_EXECUTABLE)

######################################################
###                    pto_var                      ##
######################################################

include $(CLEAR_VARS)

LOCAL_SRC_FILES := pto_var.cpp ParseExp.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH) \
	$(LOCAL_PATH)/../../vigra/include \
	$(LOCAL_PATH)/../../boost-1_53/ \
	$(LOCAL_PATH)/../../exiv2/include \
	$(LOCAL_PATH)/../../libpano13 \
	$(LOCAL_PATH)/..

LOCAL_SHARED_LIBRARIES := libpano13 libgmodule-2.0 \
	libgobject-2.0 libgthread-2.0 libglib-2.0 libvigraimpex \
	libexiv2
LOCAL_STATIC_LIBRARIES := libhugin libboost_filesystem-gcc-mt-1_53 \
	libboost_system-gcc-mt-1_53 libboost_regex-gcc-mt-1_53

LOCAL_MODULE := pto_var

LOCAL_NDK_STL_VARIANT := gnustl_static

LOCAL_CFLAGS := -O3 -fexceptions -frtti

-include external/Focal/gnustl.mk

include $(BUILD_EXECUTABLE)

######################################################
###                       nona                      ##
######################################################

include $(CLEAR_VARS)

LOCAL_SRC_FILES := nona.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH) \
	$(LOCAL_PATH)/../../vigra/include \
	$(LOCAL_PATH)/../../boost-1_53/ \
	$(LOCAL_PATH)/../../libtiff \
	$(LOCAL_PATH)/../../exiv2/include \
	$(LOCAL_PATH)/../../libpano13 \
	$(LOCAL_PATH)/../../lensfun/include/lensfun \
	$(LOCAL_PATH)/../

LOCAL_SHARED_LIBRARIES := libpano13 libgmodule-2.0 \
	libgobject-2.0 libgthread-2.0 libglib-2.0 libvigraimpex \
	libexiv2
LOCAL_STATIC_LIBRARIES := libhugin liblensfun libboost_filesystem-gcc-mt-1_53 \
	libboost_system-gcc-mt-1_53 libboost_thread-gcc-mt-1_53

LOCAL_MODULE := nona

LOCAL_NDK_STL_VARIANT := gnustl_static

LOCAL_CFLAGS := -O3 -fexceptions -frtti

-include external/Focal/gnustl.mk

include $(BUILD_EXECUTABLE)

######################################################
###               align_image_stack                 ##
######################################################

include $(CLEAR_VARS)

LOCAL_SRC_FILES := align_image_stack.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH) \
	$(LOCAL_PATH)/../../vigra/include \
	$(LOCAL_PATH)/../../boost-1_53/ \
	$(LOCAL_PATH)/../../libtiff \
	$(LOCAL_PATH)/../../exiv2/include \
	$(LOCAL_PATH)/../../libpano13 \
	$(LOCAL_PATH)/../../lensfun/include/lensfun \
	$(LOCAL_PATH)/../

LOCAL_SHARED_LIBRARIES := libpano13 libgmodule-2.0 \
	libgobject-2.0 libgthread-2.0 libglib-2.0 libvigraimpex \
	libexiv2
LOCAL_STATIC_LIBRARIES := libhugin liblensfun libboost_filesystem-gcc-mt-1_53 \
	libboost_system-gcc-mt-1_53 libboost_thread-gcc-mt-1_53

LOCAL_MODULE := align_image_stack

LOCAL_NDK_STL_VARIANT := gnustl_static

LOCAL_CFLAGS := -O3 -fexceptions -frtti

-include external/Focal/gnustl.mk

include $(BUILD_EXECUTABLE)

