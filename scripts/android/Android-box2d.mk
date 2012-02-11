LOCAL_PATH := $(call my-dir)

APP_ABI := armeabi armeabi-v7a

include $(CLEAR_VARS)

LOCAL_MODULE := box2d

LOCAL_C_INCLUDES := $(LOCAL_PATH)/

LOCAL_SRC_FILES := \
	$(subst $(LOCAL_PATH)/,, \
	$(wildcard $(LOCAL_PATH)/Box2D/Common/*.cpp) \
	$(wildcard $(LOCAL_PATH)/Box2D/Dynamics/*.cpp) \
	$(wildcard $(LOCAL_PATH)/Box2D/Dynamics/Contacts/*.cpp) \
    $(wildcard $(LOCAL_PATH)/Box2D/Dynamics/Joints/*.cpp) \
	$(wildcard $(LOCAL_PATH)/Box2D/Collision/*.cpp) \
    $(wildcard $(LOCAL_PATH)/Box2D/Collision/Shapes/*.cpp) \
	$(wildcard $(LOCAL_PATH)/Box2D/Rope/*.cpp))

LOCAL_LDLIBS := -ldl

include $(BUILD_STATIC_LIBRARY)
