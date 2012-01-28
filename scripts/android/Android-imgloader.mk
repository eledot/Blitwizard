LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := imgloader

LOCAL_C_INCLUDES := $(LOCAL_PATH)/

LOCAL_SRC_FILES := \
	$(subst $(LOCAL_PATH)/,, \
	$(wildcard $(LOCAL_PATH)/*.c))

LOCAL_LDLIBS := -ldl

include $(BUILD_SHARED_LIBRARY)
