LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_SRC_FILES := videoplayer.c
LOCAL_LDLIBS := -llog -lm
LOCAL_MODULE := libVideoPlayer
LOCAL_C_INCLUDE := $(JNI_H_INCLUDE)
include $(BUILD_SHARED_LIBRARY)