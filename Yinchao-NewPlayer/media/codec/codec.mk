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

LOCAL_MODULE    := MP3Dec

LOCAL_SRC_FILES := $(MY_SRC_PATH_MP3DEC_N)/ttMemAlign.c \
					$(MY_SRC_PATH_MP3DEC_N)/ttMP3DecApis.c \
					$(MY_SRC_PATH_MP3DEC_N)/ttMP3DecBit.c \
					$(MY_SRC_PATH_MP3DEC_N)/ttMP3DecBuf.c \
					$(MY_SRC_PATH_MP3DEC_N)/ttMP3Decdct32.c \
					$(MY_SRC_PATH_MP3DEC_N)/ttMP3DecFrame.c \
					$(MY_SRC_PATH_MP3DEC_N)/ttMP3DecHuffman.c \
					$(MY_SRC_PATH_MP3DEC_N)/ttMP3DecLayer3.c \
					$(MY_SRC_PATH_MP3DEC_N)/ttMP3DecLayer12.c \
					$(MY_SRC_PATH_MP3DEC_N)/ttMP3DecPolyphase.c \
					$(MY_SRC_PATH_MP3DEC_N)/ttMP3DecRQTable.c
				
					
LOCAL_C_INCLUDES := $(LOCAL_PATH) \
					$(MY_INC_PATH_MP3DEC_N) 
		
LOCAL_CFLAGS	:= -D__GCC32__ -D__X86__ -D__MIPS__ -DHAVE_CONFIG_H -DOPT_GENERIC -DREAL_IS_FLOAT -DNOXFERMEM -ffast-math -O2  -nostdlib -enable-int-quality 

LOCAL_SHARED_LIBRARIES := mediaplayer osal

include $(BUILD_SHARED_LIBRARY)
# Resample module
include $(CLEAR_VARS)
MY_INC_PATH_AFLIB:=  $(LOCAL_PATH)/../audiodecoder/src/aflib
MY_SRC_PATH_AFLIB:= ../audiodecoder/src/aflib

LOCAL_MODULE    := resample

LOCAL_SRC_FILES :=  $(MY_SRC_PATH_AFLIB)/aflibConverter.cpp

LOCAL_C_INCLUDES :=	$(MY_INC_PATH_AFLIB)

LOCAL_CFLAGS := -D_RESAMPLE_SMALL_FILTER_

include $(BUILD_SHARED_LIBRARY)
