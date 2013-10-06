
LOCAL_PATH := $(my-dir)

######################################################
###                    cpfind                       ##
######################################################

include $(CLEAR_VARS)

LOCAL_SRC_FILES := cpfind/main.cpp \
	cpfind/PanoDetector.cpp \
	cpfind/PanoDetectorLogic.cpp \
	cpfind/TestCode.cpp \
	cpfind/Utils.cpp \
	localfeatures/RansacFiltering.cpp \
	localfeatures/Homography.cpp \
	localfeatures/Image.cpp \
	localfeatures/CircularKeyPointDescriptor.cpp \
	localfeatures/KeyPointDetector.cpp \
	localfeatures/KeyPointIO.cpp \
	localfeatures/MathStuff.cpp \
	../zthread/src/AtomicCount.cxx \
	../zthread/src/Condition.cxx \
	../zthread/src/ConcurrentExecutor.cxx \
	../zthread/src/CountingSemaphore.cxx \
	../zthread/src/FastMutex.cxx \
	../zthread/src/FastRecursiveMutex.cxx \
	../zthread/src/Mutex.cxx \
	../zthread/src/RecursiveMutexImpl.cxx \
	../zthread/src/RecursiveMutex.cxx \
	../zthread/src/Monitor.cxx \
	../zthread/src/PoolExecutor.cxx \
	../zthread/src/PriorityCondition.cxx \
	../zthread/src/PriorityInheritanceMutex.cxx \
	../zthread/src/PriorityMutex.cxx \
	../zthread/src/PrioritySemaphore.cxx \
	../zthread/src/Semaphore.cxx \
	../zthread/src/SynchronousExecutor.cxx \
	../zthread/src/Thread.cxx \
	../zthread/src/ThreadedExecutor.cxx \
	../zthread/src/ThreadImpl.cxx \
	../zthread/src/ThreadLocalImpl.cxx \
	../zthread/src/ThreadQueue.cxx \
	../zthread/src/Time.cxx \
	../zthread/src/ThreadOps.cxx \
	../../celeste/CelesteGlobals.cpp \
	../../celeste/Celeste.cpp \
	../../celeste/ContrastFilter.cpp \
	../../celeste/Gabor.cpp \
	../../celeste/GaborFilter.cpp \
	../../celeste/GaborJet.cpp \
	../../celeste/ImageFile.cpp \
	../../celeste/LogPolar.cpp \
	../../celeste/PGMImage.cpp \
	../../celeste/svm.cpp \
	../../celeste/Utilities.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH) \
	$(LOCAL_PATH)/../../vigra/include \
	$(LOCAL_PATH)/../../boost-1_53/ \
	$(LOCAL_PATH)/../../exiv2/include \
	$(LOCAL_PATH)/../../libpano13 \
	$(LOCAL_PATH)/../../libtiff \
	$(LOCAL_PATH)/.. \
	$(LOCAL_PATH)/../zthread/include \
	$(LOCAL_PATH)/../../

LOCAL_STATIC_LIBRARIES := libhugin libboost_thread-gcc-mt-1_53 libboost_system-gcc-mt-1_53
LOCAL_SHARED_LIBRARIES := libvigraimpex libpano13 libexiv2

LOCAL_CFLAGS := -DAVOID_TABLES  -O3 -fexceptions -fstrict-aliasing -fprefetch-loop-arrays \
	-frtti -D__Ansi__ -Wno-return-type -Wno-non-virtual-dtor

LOCAL_NDK_STL_VARIANT := gnustl_static

LOCAL_ARM_MODE := arm

LOCAL_MODULE := cpfind

-include external/Focal/gnustl.mk

include $(BUILD_EXECUTABLE)

