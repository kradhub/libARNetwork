LOCAL_PATH := $(call my-dir)

# libNetwork
include $(CLEAR_VARS)

LOCAL_MODULE := LIBNETWORK-prebuilt
LOCAL_SRC_FILES := lib/libnetwork_dbg.a

include $(PREBUILT_STATIC_LIBRARY)

# WRAPPER_LIB
include $(CLEAR_VARS)

LOCAL_C_INCLUDES:= $(LOCAL_PATH)/include $(LOCAL_PATH)/../libSAL/include 
LOCAL_LDLIBS := -llog
LOCAL_MODULE := network_android
LOCAL_SRC_FILES := manager_wrapper.c  paramNewIoBuffer_wrapper.c
LOCAL_CFLAGS := -O0 -g
LOCAL_STATIC_LIBRARIES := LIBNETWORK-prebuilt
LOCAL_SHARED_LIBRARIES := LIBSAL-prebuilt
include $(BUILD_SHARED_LIBRARY)

