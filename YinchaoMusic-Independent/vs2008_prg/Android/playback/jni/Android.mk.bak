LOCAL_PATH := $(call my-dir)

INC_COMMON_PATH = $(LOCAL_PATH)/../../../Common/inc
SRC_COMMON_PATH = ../../../Common/src

INC_PLAYBACK_PATH = $(LOCAL_PATH)/../../../PlayBack/inc
SRC_PLAYBACK_PATH = ../../../PlayBack/src

INC_FAAD_PTAH =  $(LOCAL_PATH)/../../../Codecs/AacDecoder/inc
SRC_FAAD_PTAH =  ../../../Codecs/AacDecoder/src

INC_RESAMPLE_PATH = $(LOCAL_PATH)/../../../Resample/inc
SRC_RESAMPLE_PATH = ../../../Resample/src

INC_AAC_ENC_PTAH =  $(LOCAL_PATH)/../../../Codecs/AacEncoder/libAACenc/include
SRC_AAC_ENC_PTAH =  ../../../Codecs/AacEncoder/libAACenc/src

INC_AAC_SYS_PTAH =  $(LOCAL_PATH)/../../../Codecs/AacEncoder/libSYS/include
SRC_AAC_SYS_PTAH =  ../../../Codecs/AacEncoder/libSYS/src

INC_AAC_FDK_PTAH =  $(LOCAL_PATH)/../../../Codecs/AacEncoder/libFDK/include
SRC_AAC_FDK_PTAH =  ../../../Codecs/AacEncoder/libFDK/src

INC_AAC_MPEG_TP_ENC_PTAH =  $(LOCAL_PATH)/../../../Codecs/AacEncoder/libMpegTPEnc/include
SRC_AAC_MPEG_TP_ENC_PTAH =  ../../../Codecs/AacEncoder/libMpegTPEnc/src

INC_AAC_SBR_ENC_PTAH =  $(LOCAL_PATH)/../../../Codecs/AacEncoder/libSBRenc/include
SRC_AAC_SBR_ENC_PTAH =  ../../../Codecs/AacEncoder/libSBRenc/src

aacenc_sources := $(wildcard $(SRC_AAC_ENC_PTAH)/*.cpp)
aacenc_sources := $(aacenc_sources:$(SRC_AAC_ENC_PTAH)/%=%)

sys_sources := $(wildcard $(SRC_AAC_SYS_PTAH)/*.cpp)
sys_sources := $(sys_sources:$(SRC_AAC_SYS_PTAH)/%=%)

fdk_sources := $(wildcard $(SRC_AAC_FDK_PTAH)/*.cpp)
fdk_sources := $(fdk_sources:$(SRC_AAC_FDK_PTAH)/%=%)

mpegtpenc_sources := $(wildcard $(SRC_AAC_MPEG_TP_ENC_PTAH)/*.cpp)
mpegtpenc_sources := $(mpegtpenc_sources:$(SRC_AAC_MPEG_TP_ENC_PTAH)/%=%)

sbrenc_sources := $(wildcard $(SRC_AAC_SBR_ENC_PTAH)/*.cpp)
sbrenc_sources := $(sbrenc_sources:$(SRC_AAC_SBR_ENC_PTAH)/%=%)

#playback
include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_MODULE := mediaplayer
LOCAL_LDLIBS := -llog -landroid -lOpenSLES -lz
LOCAL_STATIC_LIBRARIES := libAacDec libresample libAacEnc
LOCAL_C_INCLUDES +=$(INC_COMMON_PATH) $(INC_PLAYBACK_PATH) $(INC_FAAD_PTAH) $(INC_RESAMPLE_PATH) $(INC_AAC_ENC_PTAH) $(INC_AAC_SYS_PTAH)
LOCAL_CFLAGS += -DCPU_ARM -Wno-psabi -D__SUPPORT_RECORD__
LOCAL_SRC_FILES := 	STMediaPlayer_jni.cpp \
				   STAudioOutput.cpp \
				   STOpenSLESEngine.cpp \
				   STRunTime.cpp \
				   STAudioRecorder.cpp \
				   $(SRC_PLAYBACK_PATH)/STMediaPlayer.cpp \
				   $(SRC_PLAYBACK_PATH)/STMsgQueue.cpp \
				   $(SRC_PLAYBACK_PATH)/STPluginManager.cpp \
				   $(SRC_PLAYBACK_PATH)/STSampleBuffer.cpp \
				   $(SRC_PLAYBACK_PATH)/STSampleBufferManager.cpp \
				   $(SRC_PLAYBACK_PATH)/STBasePlugin.cpp \
				   $(SRC_PLAYBACK_PATH)/STBaseAudioOutput.cpp \
				   $(SRC_PLAYBACK_PATH)/STFFT.cpp \
				   $(SRC_PLAYBACK_PATH)/STPlayControl.cpp \
				   $(SRC_PLAYBACK_PATH)/STWavPlugin.cpp \
				   $(SRC_PLAYBACK_PATH)/STAacPlugin.cpp \
				   $(SRC_PLAYBACK_PATH)/STAacEncoder.cpp \
				   $(SRC_COMMON_PATH)/STSemaphore.cpp \
				   $(SRC_COMMON_PATH)/STUrlUtils.cpp \
				   $(SRC_COMMON_PATH)/STCritical.cpp \
				   $(SRC_COMMON_PATH)/STSysTime.cpp \
				   $(SRC_COMMON_PATH)/STThread.cpp \
				   $(SRC_COMMON_PATH)/STDataReaderSelector.cpp \
				   $(SRC_COMMON_PATH)/STFileReader.cpp \
				   $(SRC_COMMON_PATH)/STHttpCacheFile.cpp \
				   $(SRC_COMMON_PATH)/STHttpReader.cpp \
				   $(SRC_COMMON_PATH)/STHttpClient.cpp \

include $(BUILD_SHARED_LIBRARY)

#aac decode
include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_MODULE := libAacDec
LOCAL_C_INCLUDES := $(INC_FAAD_PTAH) $(INC_COMMON_PATH)
LOCAL_CFLAGS += -Wno-psabi
LOCAL_SRC_FILES := $(SRC_FAAD_PTAH)/bits.c \
				   $(SRC_FAAD_PTAH)/cfft.c \
				   $(SRC_FAAD_PTAH)/common.c \
				   $(SRC_FAAD_PTAH)/decoder.c \
				   $(SRC_FAAD_PTAH)/drc.c \
				   $(SRC_FAAD_PTAH)/drm_dec.c \
				   $(SRC_FAAD_PTAH)/error.c \
				   $(SRC_FAAD_PTAH)/filtbank.c \
				   $(SRC_FAAD_PTAH)/hcr.c \
				   $(SRC_FAAD_PTAH)/huffman.c \
				   $(SRC_FAAD_PTAH)/ic_predict.c \
				   $(SRC_FAAD_PTAH)/is.c \
				   $(SRC_FAAD_PTAH)/lt_predict.c \
				   $(SRC_FAAD_PTAH)/mdct.c \
				   $(SRC_FAAD_PTAH)/mp4.c \
				   $(SRC_FAAD_PTAH)/ms.c \
				   $(SRC_FAAD_PTAH)/output.c \
				   $(SRC_FAAD_PTAH)/pns.c \
				   $(SRC_FAAD_PTAH)/ps_dec.c \
				   $(SRC_FAAD_PTAH)/ps_syntax.c \
				   $(SRC_FAAD_PTAH)/pulse.c \
				   $(SRC_FAAD_PTAH)/rvlc.c \
				   $(SRC_FAAD_PTAH)/sbr_dct.c \
				   $(SRC_FAAD_PTAH)/sbr_dec.c \
				   $(SRC_FAAD_PTAH)/sbr_e_nf.c \
				   $(SRC_FAAD_PTAH)/sbr_fbt.c \
				   $(SRC_FAAD_PTAH)/sbr_hfadj.c \
				   $(SRC_FAAD_PTAH)/sbr_hfgen.c \
				   $(SRC_FAAD_PTAH)/sbr_huff.c \
				   $(SRC_FAAD_PTAH)/sbr_qmf.c \
				   $(SRC_FAAD_PTAH)/sbr_syntax.c \
				   $(SRC_FAAD_PTAH)/sbr_tf_grid.c \
				   $(SRC_FAAD_PTAH)/specrec.c \
				   $(SRC_FAAD_PTAH)/ssr.c \
				   $(SRC_FAAD_PTAH)/ssr_fb.c \
				   $(SRC_FAAD_PTAH)/ssr_ipqf.c \
				   $(SRC_FAAD_PTAH)/syntax.c \
				   $(SRC_FAAD_PTAH)/tns.c \

include $(BUILD_STATIC_LIBRARY)

#resample
include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_MODULE := libresample
LOCAL_C_INCLUDES := $(INC_RESAMPLE_PATH)
LOCAL_CFLAGS += -Wno-psabi
LOCAL_SRC_FILES := $(SRC_RESAMPLE_PATH)/Resample_samplerate.cpp \
				   $(SRC_RESAMPLE_PATH)/Resample_src_linear.cpp \
				   $(SRC_RESAMPLE_PATH)/Resample_src_sinc.cpp \
				   $(SRC_RESAMPLE_PATH)/Resample_src_zoh.cpp \

include $(BUILD_STATIC_LIBRARY)

#AAC Encoder
include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_MODULE := libAacEnc
LOCAL_CFLAGS += -DANDROID
LOCAL_CFLAGS += -Wno-psabi
LOCAL_CFLAGS += -Wno-sequence-point -Wno-extra
LOCAL_LDLIBS := -llog -landroid

LOCAL_C_INCLUDES := $(INC_AAC_ENC_PTAH) \
				   $(INC_AAC_SYS_PTAH) \
				   $(INC_AAC_FDK_PTAH) \
				   $(INC_AAC_MPEG_TP_ENC_PTAH) \
				   $(INC_AAC_SBR_ENC_PTAH) \

LOCAL_SRC_FILES :=  $(aacenc_sources:%=$(SRC_AAC_ENC_PTAH)/%) \
				   $(sys_sources:%=$(SRC_AAC_SYS_PTAH)/%) \
				   $(fdk_sources:%=$(SRC_AAC_FDK_PTAH)/%) \
				   $(mpegtpenc_sources:%=$(SRC_AAC_MPEG_TP_ENC_PTAH)/%) \
				   $(sbrenc_sources:%=$(SRC_AAC_SBR_ENC_PTAH)/%) \

include $(BUILD_STATIC_LIBRARY)