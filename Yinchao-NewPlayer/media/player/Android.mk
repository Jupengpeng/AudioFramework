# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

LOCAL_PATH := $(call my-dir)

# Media jni

include $(CLEAR_VARS)

MY_INC_PATH_INFO := $(LOCAL_PATH)/../info/inc
MY_SRC_PATH_INFO := ../info/src

MY_INC_PATH_PLAYER := $(LOCAL_PATH)/inc
MY_SRC_PATH_PLAYER := src

MY_INC_PATH_COMMON := $(LOCAL_PATH)/../common/inc
MY_SRC_PATH_COMMON := ../common/src

MY_INC_PATH_AFLIB := $(LOCAL_PATH)/../audiodecoder/src/aflib

LOCAL_MODULE    := mediaplayer

LOCAL_ARM_MODE := arm
 
LOCAL_SRC_FILES := 	\
					TTMediaPlayer_jni.cpp \
					$(MY_SRC_PATH_COMMON)/TTAudioPlugin.cpp \
					$(MY_SRC_PATH_COMMON)/TTFFT.cpp \
					$(MY_SRC_PATH_PLAYER)/TTAudioDecode.cpp \
					$(MY_SRC_PATH_PLAYER)/TTAudioProcess.cpp \
					$(MY_SRC_PATH_PLAYER)/TTBaseAudioSink.cpp \
					$(MY_SRC_PATH_PLAYER)/TTSrcDemux.cpp \
					$(MY_SRC_PATH_PLAYER)/TTMediaPlayer.cpp \
					$(MY_SRC_PATH_PLAYER)/TTMediaPlayerFactory.cpp \
					$(MY_SRC_PATH_PLAYER)/PureDecodeEntity.cpp \
					$(MY_SRC_PATH_PLAYER)/Android/TTAndroidAudioSink.cpp \
					$(MY_SRC_PATH_INFO)/TTIntReader.cpp \
					$(MY_SRC_PATH_INFO)/TTID3Tag.cpp \
					$(MY_SRC_PATH_INFO)/TTAPETag.cpp \
					$(MY_SRC_PATH_INFO)/TTMediaParser.cpp \
					$(MY_SRC_PATH_INFO)/TTMP3Header.cpp \
					$(MY_SRC_PATH_INFO)/TTMP3Parser.cpp \
					$(MY_SRC_PATH_INFO)/TTHttpMP3Parser.cpp \
					$(MY_SRC_PATH_INFO)/TTMediainfoProxy.cpp \
					
LOCAL_C_INCLUDES := $(LOCAL_PATH) \
					$(MY_INC_PATH_PLAYER) \
					$(MY_INC_PATH_COMMON) \
					$(MY_INC_PATH_INFO) \
					$(MY_INC_PATH_TAG) \
					$(MY_INC_PATH_PLAYER)/Android/ \
					$(MY_INC_PATH_AFLIB)
										
LOCAL_EXPORT_C_INCLUDES := \
					$(MY_INC_PATH_INFO) \
					$(MY_INC_PATH_COMMON) \
					$(MY_INC_PATH_PLAYER)

LOCAL_LDLIBS	:= -ldl

ifeq ($(TARGET_ARCH),arm)
LOCAL_CFLAGS	+=-D_ARM_ARCH_VFP_ -DCPU_ARM -ffunction-sections -fdata-sections
LOCAL_CPPFLAGS	+=-D_ARM_ARCH_VFP_ -DCPU_ARM -ffunction-sections -fdata-sections
LOCAL_STATIC_LIBRARIES += cpufeatures
endif

LOCAL_LDFLAGS := -Wl,--gc-sections
					
LOCAL_SHARED_LIBRARIES := osal resample

include $(BUILD_SHARED_LIBRARY)

