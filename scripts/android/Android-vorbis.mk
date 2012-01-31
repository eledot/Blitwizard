LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := vorbis

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include $(LOCAL_PATH)/../ogg/include/ $(LOCAL_PATH)/lib/

LOCAL_SRC_FILES := \
	$(subst $(LOCAL_PATH)/,, \
	$(wildcard $(LOCAL_PATH)/lib/*.c))

LOCAL_SHARED_LIBRARIES := ogg

LOCAL_LDLIBS := -ldl 

include $(BUILD_SHARED_LIBRARY)
