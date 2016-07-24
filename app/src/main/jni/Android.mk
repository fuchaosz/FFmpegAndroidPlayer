LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_SRC_FILES := videoplayer.c
LOCAL_LDLIBS := -llog -lm
LOCAL_MODULE := VideoPlayer
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
include $(BUILD_SHARED_LIBRARY)