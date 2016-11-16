#LOCAL_PATH := $(call my-dir)
LOCAL_PATH := .

MY_INCLUDE_COMMON_DIR :=  $(LOCAL_PATH)/../common
MY_SRC_BASE_DIR := $(LOCAL_PATH)/../src
MY_INC_BASE_DIR := $(LOCAL_PATH)/../inc

include $(CLEAR_VARS)

LOCAL_MODULE := ay_client_player_android
LOCAL_MODULE_FILENAME := libAYMediaPlayer
LOCAL_MODULE_PATH := $(MY_LIB_BASE_DIR)	

LOCAL_C_INCLUDES := $(MY_INCLUDE_COMMON_DIR) \
    $(MY_SRC_BASE_DIR) \
    $(MY_INC_BASE_DIR)

LOCAL_SRC_FILES := \
	$(MY_INCLUDE_COMMON_DIR)/ulu_mutex.cpp \
	$(MY_INCLUDE_COMMON_DIR)/ulu_OSFunc.cpp \
	$(MY_INCLUDE_COMMON_DIR)/ulu_thread.cpp \
	$(MY_INCLUDE_COMMON_DIR)/ulu_JniEnvUtil.cpp \
	$(MY_SRC_BASE_DIR)/CAudioRender.cpp \
	$(MY_SRC_BASE_DIR)/AYMediaPlayer.cpp \
	$(MY_SRC_BASE_DIR)/AndriodAudioRender.cpp 


LOCAL_CFLAGS :=  -D_LINUX -D_LINUX_ANDROID -D__STDC_CONSTANT_MACROS -D__AYPlayerSDK_FACTORY__ -DULU_Player -D_DUMP_LOG -D_DUMPDATA_ORIGINAL
LOCAL_LDLIBS    := -llog -landroid  -lEGL -lGLESv2
#include $(BUILD_STATIC_LIBRARY)
include $(BUILD_SHARED_LIBRARY)



