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

MY_INC_PATH_OSAL:= $(LOCAL_PATH)/inc
MY_SRC_PATH_OSAL:= src

# Os adaptive layer

include $(CLEAR_VARS)

LOCAL_MODULE    := libosal

LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES :=  $(MY_SRC_PATH_OSAL)/TTBufferReader.cpp \
					$(MY_SRC_PATH_OSAL)/TTBufferReaderProxy.cpp \
					$(MY_SRC_PATH_OSAL)/TTCacheBuffer.cpp \
					$(MY_SRC_PATH_OSAL)/TTCondition.cpp \
					$(MY_SRC_PATH_OSAL)/TTFileReader.cpp \
					$(MY_SRC_PATH_OSAL)/TTHttpReader.cpp \
					$(MY_SRC_PATH_OSAL)/TTHttpCacheFile.cpp \
					$(MY_SRC_PATH_OSAL)/TTDNSCache.cpp \
					$(MY_SRC_PATH_OSAL)/TTHttpClient.cpp \
					$(MY_SRC_PATH_OSAL)/TTHttpReaderProxy.cpp \
					$(MY_SRC_PATH_OSAL)/TTBaseDataReader.cpp \
					$(MY_SRC_PATH_OSAL)/Android/TTPackagePathFetcher.cpp \
					$(MY_SRC_PATH_OSAL)/TTCritical.cpp \
					$(MY_SRC_PATH_OSAL)/TThread.cpp \
					$(MY_SRC_PATH_OSAL)/TTSleep.cpp \
					$(MY_SRC_PATH_OSAL)/TTEventThread.cpp \
					$(MY_SRC_PATH_OSAL)/TTSemaphore.cpp \
					$(MY_SRC_PATH_OSAL)/TTSysTime.cpp \
					$(MY_SRC_PATH_OSAL)/TTDllLoader.cpp \
					$(MY_SRC_PATH_OSAL)/TTJniEnvUtil.cpp 	\
					$(MY_SRC_PATH_OSAL)/TTNetWorkConfig.cpp \
					$(MY_SRC_PATH_OSAL)/TTIOClient.cpp \
					$(MY_SRC_PATH_OSAL)/TTBitReader.cpp \
					$(MY_SRC_PATH_OSAL)/TTAvUtils.cpp \
					$(MY_SRC_PATH_OSAL)/TTUrlParser.cpp 
					

LOCAL_C_INCLUDES :=	$(MY_INC_PATH_OSAL) \
					$(MY_INC_PATH_OSAL)/Android

LOCAL_CFLAGS	+= -ffunction-sections -fdata-sections
LOCAL_LDFLAGS := -Wl,--gc-sections
LOCAL_LDLIBS	:= -llog 

LOCAL_EXPORT_C_INCLUDES := $(MY_INC_PATH_OSAL) \
						   $(MY_INC_PATH_OSAL)/Android

LOCAL_EXPORT_LDLIBS := -llog

include $(BUILD_SHARED_LIBRARY)

