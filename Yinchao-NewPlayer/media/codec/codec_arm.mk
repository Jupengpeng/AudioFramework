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

include $(CLEAR_VARS)

MY_INC_PATH_MP3DEC_N:= $(LOCAL_PATH)/MP3Dec/inc
MY_SRC_PATH_MP3DEC_N:= MP3Dec/src


# MP3Dec
include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
LOCAL_MODULE    := MP3Dec_v7
else
LOCAL_MODULE    := MP3Dec
endif

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
LOCAL_ARM_NEON := true
endif

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
LOCAL_SRC_FILES := 	\
					$(MY_SRC_PATH_MP3DEC_N)/ttMemAlign.c \
					$(MY_SRC_PATH_MP3DEC_N)/ttMP3DecApis.c \
					$(MY_SRC_PATH_MP3DEC_N)/ttMP3DecBit.c \
					$(MY_SRC_PATH_MP3DEC_N)/ttMP3DecBuf.c \
					$(MY_SRC_PATH_MP3DEC_N)/ttMP3Decdct32.c \
					$(MY_SRC_PATH_MP3DEC_N)/ttMP3DecFrame.c \
					$(MY_SRC_PATH_MP3DEC_N)/ttMP3DecLayer3.c \
					$(MY_SRC_PATH_MP3DEC_N)/ttMP3DecHuffman.c \
					$(MY_SRC_PATH_MP3DEC_N)/ttMP3DecLayer12.c \
					$(MY_SRC_PATH_MP3DEC_N)/ttMP3DecPolyphase.c \
					$(MY_SRC_PATH_MP3DEC_N)/ttMP3DecRQTable.c \
					$(MY_SRC_PATH_MP3DEC_N)/android_armv7/dct32_asm.S \
					$(MY_SRC_PATH_MP3DEC_N)/android_armv7/layer3_asm.S \
					$(MY_SRC_PATH_MP3DEC_N)/android_armv7/Synth_asm.S
	
endif					

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
LOCAL_CFLAGS := -DARM_OPT -DARMV6_OPT -DARMV7_OPT -DNDEBUG -mfloat-abi=soft -mfpu=neon -march=armv7-a -mtune=cortex-a8 -fsigned-char -O2 -ffast-math  -nostdlib -enable-int-quality  -fvisibility=hidden -ffunction-sections -fdata-sections
else
LOCAL_CFLAGS := -DARM_OPT -DARMV6_OPT -DNDEBUG -mfloat-abi=soft -march=armv6j -mtune=arm1136jf-s -fsigned-char -O2 -ffast-math  -nostdlib -enable-int-quality  -fvisibility=hidden -ffunction-sections -fdata-sections
endif

LOCAL_LDFLAGS := -Wl,--gc-sections

LOCAL_DISABLE_FATAL_LINKER_WARNINGS := true

# ifneq ($(LOCAL_DISABLE_FATAL_LINKER_WARNINGS),true)
#  LOCAL_LDFLAGS += -Wl,--fatal-warnings
# endif

ifneq ($(TARGET_ARCH_ABI),armeabi-v7a)
LOCAL_SRC_FILES := 	\
					$(MY_SRC_PATH_MP3DEC_N)/ttMemAlign.c \
					$(MY_SRC_PATH_MP3DEC_N)/ttMP3DecApis.c \
					$(MY_SRC_PATH_MP3DEC_N)/ttMP3DecBit.c \
					$(MY_SRC_PATH_MP3DEC_N)/ttMP3DecBuf.c \
					$(MY_SRC_PATH_MP3DEC_N)/ttMP3Decdct32.c \
					$(MY_SRC_PATH_MP3DEC_N)/ttMP3DecFrame.c \
					$(MY_SRC_PATH_MP3DEC_N)/ttMP3DecLayer3.c \
					$(MY_SRC_PATH_MP3DEC_N)/ttMP3DecHuffman.c \
					$(MY_SRC_PATH_MP3DEC_N)/ttMP3DecLayer12.c \
					$(MY_SRC_PATH_MP3DEC_N)/ttMP3DecPolyphase.c \
					$(MY_SRC_PATH_MP3DEC_N)/ttMP3DecRQTable.c \
					$(MY_SRC_PATH_MP3DEC_N)/android_armv6/dct32_asm.S \
					$(MY_SRC_PATH_MP3DEC_N)/android_armv6/layer3_asm.S \
					$(MY_SRC_PATH_MP3DEC_N)/android_armv6/Synth_asm.S
 
endif							
								
LOCAL_C_INCLUDES := $(LOCAL_PATH) \
					$(MY_INC_PATH_MP3DEC_N) 

LOCAL_SHARED_LIBRARIES := mediaplayer osal

include $(BUILD_SHARED_LIBRARY)

# Resample module
include $(CLEAR_VARS)
MY_INC_PATH_AFLIB:=  $(LOCAL_PATH)/../audiodecoder/src/aflib
MY_SRC_PATH_AFLIB:= ../audiodecoder/src/aflib

LOCAL_MODULE    := resample

LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES :=  $(MY_SRC_PATH_AFLIB)/aflibConverter.cpp

LOCAL_C_INCLUDES :=	$(MY_INC_PATH_AFLIB)

ifeq ($(TARGET_ARCH),arm)
LOCAL_CFLAGS := -D_RESAMPLE_SMALL_FILTER_ -O2 -ffunction-sections -fdata-sections
LOCAL_LDFLAGS := -Wl,--gc-sections,--no-fatal-warnings,--no-warn-shared-textrel
endif

include $(BUILD_SHARED_LIBRARY)
