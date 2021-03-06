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

LOCAL_DISABLE_FATAL_LINKER_WARNINGS := true

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)


OSAL_MK := $(LOCAL_PATH)/resources/osal/Android.mk
PLAYER_MK := $(LOCAL_PATH)/resources/media/player/Android.mk
CODEC_MK := $(LOCAL_PATH)/resources/media/codec/Android.mk

#OSAL_MK := $(LOCAL_PATH)/../../../Yinchao-NewPlayer/osal/Android.mk
#PLAYER_MK := $(LOCAL_PATH)/../../../Yinchao-NewPlayer/media/player/Android.mk
#CODEC_MK := $(LOCAL_PATH)/../../../Yinchao-NewPlayer/media/codec/Android.mk
#TAG_MK := $(LOCAL_PATH)/../../../Yinchao-NewPlayer/taglib/Android.mk

include $(OSAL_MK)
include $(PLAYER_MK)
include $(CODEC_MK)
#include $(TAG_MK)

