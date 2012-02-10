LOCAL_PATH := $(call my-dir)

APP_ABI := armeabi armeabi-v7a

include $(CLEAR_VARS)

LOCAL_MODULE := png

LOCAL_C_INCLUDES := $(LOCAL_PATH)/

LOCAL_SRC_FILES := \
	$(subst $(LOCAL_PATH)/,, \
	$(wildcard $(LOCAL_PATH)/*.c))

LOCAL_STATIC_LIBRARIES := zlib

LOCAL_LDLIBS := -ldl 

include $(BUILD_STATIC_LIBRARY)
