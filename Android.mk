NEMESIS_JNI_PATH := $(call my-dir)

ifeq ($(ANDROID_BUILD_TOP),)
	include $(NEMESIS_JNI_PATH)/libtiff/Android.mk
	include $(NEMESIS_JNI_PATH)/libpano13/Android.mk
	include $(NEMESIS_JNI_PATH)/vigra/Android.mk
	include $(NEMESIS_JNI_PATH)/lcms2/Android.mk
	include $(NEMESIS_JNI_PATH)/gsl/Android.mk
	include $(NEMESIS_JNI_PATH)/enblend/Android.mk
	include $(NEMESIS_JNI_PATH)/apsc/Android.mk
	include $(NEMESIS_JNI_PATH)/libjpeg-turbo/Android.mk
	include $(NEMESIS_JNI_PATH)/libpng/Android.mk
	include $(NEMESIS_JNI_PATH)/libiconv/Android.mk
	include $(NEMESIS_JNI_PATH)/libxml2/Android.mk
endif
