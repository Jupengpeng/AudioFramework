LOCAL_PATH := $(call my-dir)






INC_COMMON_PATH := $(LOCAL_PATH)/../../../Common/inc
SRC_COMMON_PATH := ../../../Common/src

INC_PLAYBACK_PATH := $(LOCAL_PATH)/../../../PlayBack/inc
SRC_PLAYBACK_PATH := ../../../PlayBack/src

INC_FAAD_PTAH :=  $(LOCAL_PATH)/../../../Codecs/AacDecoder/inc
SRC_FAAD_PTAH :=  ../../../Codecs/AacDecoder/src

INC_RESAMPLE_PATH := $(LOCAL_PATH)/../../../Resample/inc
SRC_RESAMPLE_PATH := ../../../Resample/src

INC_MP3_ENC_PTAH :=  $(LOCAL_PATH)/../../../Codecs/Mp3Encoder/inc
SRC_MP3_ENC_PTAH :=  ../../../Codecs/Mp3Encoder/src


INC_AAC_ENC_PTAH :=  $(LOCAL_PATH)/../../../Codecs/AacEncoder/libAACenc/include
SRC_AAC_ENC_PTAH :=  ../../../Codecs/AacEncoder/libAACenc/src

INC_AAC_SYS_PTAH :=  $(LOCAL_PATH)/../../../Codecs/AacEncoder/libSYS/include
SRC_AAC_SYS_PTAH :=  ../../../Codecs/AacEncoder/libSYS/src

INC_AAC_FDK_PTAH :=  $(LOCAL_PATH)/../../../Codecs/AacEncoder/libFDK/include
SRC_AAC_FDK_PTAH :=  ../../../Codecs/AacEncoder/libFDK/src

INC_AAC_MPEG_TP_ENC_PTAH :=  $(LOCAL_PATH)/../../../Codecs/AacEncoder/libMpegTPEnc/include
SRC_AAC_MPEG_TP_ENC_PTAH :=  ../../../Codecs/AacEncoder/libMpegTPEnc/src

INC_AAC_SBR_ENC_PTAH :=  $(LOCAL_PATH)/../../../Codecs/AacEncoder/libSBRenc/include
SRC_AAC_SBR_ENC_PTAH :=  ../../../Codecs/AacEncoder/libSBRenc/src

#add by bin.chen libspeex build
INC_LIB_SPEEX_PTAH := $(LOCAL_PATH)/../../../Codecs/Libspeex/include
SRC_LIB_SPEEX_PTAH :=  ../../../Codecs/Libspeex/libspeex

#add by bin.chen STReverb build
INC_LIB_STREVERB_PTAH := $(LOCAL_PATH)/../../../Codecs/STReverb
SRC_LIB_STREVERB_PTAH :=  ../../../Codecs/STReverb

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


#add by bin.chen
libspeex_sources := $(wildcard $(SRC_LIB_SPEEX_PTAH)/*.c)
libspeex_sources := $(libspeex_sources:$(SRC_LIB_SPEEX_PTAH)/%=%)

mp3enc_sources :=$(wildcard $(SRC_MP3_ENC_PTAH)/*.c)
mp3enc_sources := $(mp3enc_sources:$(SRC_MP3_ENC_PTAH)/%=%)



#playback
include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_MODULE := mediaplayer
#ndk_linux_r8
#LOCAL_LDLIBS := -llog -landroid -lOpenSLES -lz -L$(LOCAL_PATH) -L$(NDK)/sources/cxx-stl/gnu-libstdc++/libs/armeabi-v7a -lfreeverb3 -lfftw3f -lsndfile -lgnustl_static
LOCAL_LDLIBS := -llog -landroid -lOpenSLES -lz
LOCAL_STATIC_LIBRARIES := libmp3lame libAacDec libresample libAacEnc libspeex libfreeverb3 libfftw3 libsamplerate2 

#LOCAL_LDLIBS := -llog -landroid -lOpenSLES -lz -L$(LOCAL_PATH) -L$(NDK)/sources/cxx-stl/gnu-libstdc++/libs/armeabi-v7a

LOCAL_C_INCLUDES +=$(INC_COMMON_PATH) $(INC_PLAYBACK_PATH) $(INC_FAAD_PTAH) $(INC_RESAMPLE_PATH) $(INC_AAC_ENC_PTAH) $(INC_MP3_ENC_PTAH) $(INC_AAC_SYS_PTAH) $(INC_LIB_SPEEX_PTAH) $(INC_LIB_STREVERB_PTAH)
LOCAL_CFLAGS += -DCPU_ARM -D__SUPPORT_RECORD__ -DANDROID_NDK -D_RELEASE -DSTDC_HEADERS
LOCAL_SRC_FILES := 	STMediaPlayer_jni.cpp \
                   STReverb.cpp \
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

LOCAL_SRC_FILES :=  $(SRC_AAC_ENC_PTAH)/transform.cpp \
										$(SRC_AAC_ENC_PTAH)/tonality.cpp \
										$(SRC_AAC_ENC_PTAH)/spreading.cpp \
										$(SRC_AAC_ENC_PTAH)/sf_estim.cpp \
										$(SRC_AAC_ENC_PTAH)/quantize.cpp \
										$(SRC_AAC_ENC_PTAH)/qc_main.cpp \
										$(SRC_AAC_ENC_PTAH)/psy_main.cpp \
										$(SRC_AAC_ENC_PTAH)/psy_configuration.cpp \
										$(SRC_AAC_ENC_PTAH)/pre_echo_control.cpp \
										$(SRC_AAC_ENC_PTAH)/pnsparam.cpp \
										$(SRC_AAC_ENC_PTAH)/noisedet.cpp \
										$(SRC_AAC_ENC_PTAH)/ms_stereo.cpp \
										$(SRC_AAC_ENC_PTAH)/metadata_main.cpp \
										$(SRC_AAC_ENC_PTAH)/metadata_compressor.cpp \
										$(SRC_AAC_ENC_PTAH)/line_pe.cpp \
										$(SRC_AAC_ENC_PTAH)/intensity.cpp \
										$(SRC_AAC_ENC_PTAH)/grp_data.cpp \
										$(SRC_AAC_ENC_PTAH)/dyn_bits.cpp \
										$(SRC_AAC_ENC_PTAH)/chaosmeasure.cpp \
										$(SRC_AAC_ENC_PTAH)/channel_map.cpp \
										$(SRC_AAC_ENC_PTAH)/block_switch.cpp \
										$(SRC_AAC_ENC_PTAH)/bitenc.cpp \
										$(SRC_AAC_ENC_PTAH)/bit_cnt.cpp \
										$(SRC_AAC_ENC_PTAH)/bandwidth.cpp \
										$(SRC_AAC_ENC_PTAH)/band_nrg.cpp \
										$(SRC_AAC_ENC_PTAH)/adj_thr.cpp \
										$(SRC_AAC_ENC_PTAH)/aacenc_tns.cpp \
										$(SRC_AAC_ENC_PTAH)/aacEnc_rom.cpp \
										$(SRC_AAC_ENC_PTAH)/aacEnc_ram.cpp \
										$(SRC_AAC_ENC_PTAH)/aacenc_pns.cpp \
										$(SRC_AAC_ENC_PTAH)/aacenc_lib.cpp \
										$(SRC_AAC_ENC_PTAH)/aacenc_hcr.cpp \
										$(SRC_AAC_ENC_PTAH)/aacenc.cpp \
										$(SRC_AAC_SYS_PTAH)/cmdl_parser.cpp \
										$(SRC_AAC_SYS_PTAH)/conv_string.cpp \
										$(SRC_AAC_SYS_PTAH)/genericStds.cpp \
										$(SRC_AAC_SYS_PTAH)/wav_file.cpp \
										$(SRC_AAC_FDK_PTAH)/dct.cpp \
										$(SRC_AAC_FDK_PTAH)/autocorr2nd.cpp \
										$(SRC_AAC_FDK_PTAH)/FDK_bitbuffer.cpp \
										$(SRC_AAC_FDK_PTAH)/FDK_core.cpp \
										$(SRC_AAC_FDK_PTAH)/FDK_crc.cpp \
										$(SRC_AAC_FDK_PTAH)/FDK_hybrid.cpp \
										$(SRC_AAC_FDK_PTAH)/FDK_tools_rom.cpp \
										$(SRC_AAC_FDK_PTAH)/FDK_trigFcts.cpp \
										$(SRC_AAC_FDK_PTAH)/fft.cpp \
										$(SRC_AAC_FDK_PTAH)/fft_rad2.cpp \
										$(SRC_AAC_FDK_PTAH)/fixpoint_math.cpp \
										$(SRC_AAC_FDK_PTAH)/mdct.cpp \
										$(SRC_AAC_FDK_PTAH)/qmf.cpp \
										$(SRC_AAC_FDK_PTAH)/scale.cpp \
										$(SRC_AAC_MPEG_TP_ENC_PTAH)/tpenc_adif.cpp \
										$(SRC_AAC_MPEG_TP_ENC_PTAH)/tpenc_adts.cpp \
										$(SRC_AAC_MPEG_TP_ENC_PTAH)/tpenc_asc.cpp \
										$(SRC_AAC_MPEG_TP_ENC_PTAH)/tpenc_latm.cpp \
										$(SRC_AAC_MPEG_TP_ENC_PTAH)/tpenc_lib.cpp \
                    $(SRC_AAC_SBR_ENC_PTAH)/bit_sbr.cpp \
                    $(SRC_AAC_SBR_ENC_PTAH)/code_env.cpp \
                    $(SRC_AAC_SBR_ENC_PTAH)/env_bit.cpp \
                    $(SRC_AAC_SBR_ENC_PTAH)/env_est.cpp \
                    $(SRC_AAC_SBR_ENC_PTAH)/fram_gen.cpp \
                    $(SRC_AAC_SBR_ENC_PTAH)/invf_est.cpp \
                    $(SRC_AAC_SBR_ENC_PTAH)/mh_det.cpp \
                    $(SRC_AAC_SBR_ENC_PTAH)/nf_est.cpp \
                    $(SRC_AAC_SBR_ENC_PTAH)/ps_bitenc.cpp \
                    $(SRC_AAC_SBR_ENC_PTAH)/ps_encode.cpp \
                    $(SRC_AAC_SBR_ENC_PTAH)/ps_main.cpp \
                    $(SRC_AAC_SBR_ENC_PTAH)/resampler.cpp \
                    $(SRC_AAC_SBR_ENC_PTAH)/sbr_encoder.cpp \
                    $(SRC_AAC_SBR_ENC_PTAH)/sbr_misc.cpp \
                    $(SRC_AAC_SBR_ENC_PTAH)/sbr_ram.cpp \
                    $(SRC_AAC_SBR_ENC_PTAH)/sbr_rom.cpp \
                    $(SRC_AAC_SBR_ENC_PTAH)/sbrenc_freq_sca.cpp \
                    $(SRC_AAC_SBR_ENC_PTAH)/ton_corr.cpp \
                    $(SRC_AAC_SBR_ENC_PTAH)/tran_det.cpp 

                    
								


include $(BUILD_STATIC_LIBRARY)


#MP3 Encoder
include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_MODULE := libmp3lame
LOCAL_CFLAGS += -DANDROID
LOCAL_CFLAGS += -Wno-psabi
LOCAL_CFLAGS += -Wno-sequence-point -Wno-extra
LOCAL_LDLIBS := -llog -landroid -ldl -lGLESv1_CM

LOCAL_C_INCLUDES := $(INC_MP3_ENC_PTAH) \


LOCAL_SRC_FILES :=  $(SRC_MP3_ENC_PTAH)/bitstream.c \
										$(SRC_MP3_ENC_PTAH)/encoder.c \
										$(SRC_MP3_ENC_PTAH)/fft.c \
										$(SRC_MP3_ENC_PTAH)/gain_analysis.c \
										$(SRC_MP3_ENC_PTAH)/id3tag.c \
										$(SRC_MP3_ENC_PTAH)/lame.c \
										$(SRC_MP3_ENC_PTAH)/newmdct.c \
										$(SRC_MP3_ENC_PTAH)/presets.c \
										$(SRC_MP3_ENC_PTAH)/psymodel.c \
										$(SRC_MP3_ENC_PTAH)/quantize.c \
										$(SRC_MP3_ENC_PTAH)/quantize_pvt.c \
										$(SRC_MP3_ENC_PTAH)/reservoir.c \
										$(SRC_MP3_ENC_PTAH)/set_get.c \
										$(SRC_MP3_ENC_PTAH)/tables.c \
										$(SRC_MP3_ENC_PTAH)/takehiro.c \
										$(SRC_MP3_ENC_PTAH)/util.c \
										$(SRC_MP3_ENC_PTAH)/vbrquantize.c \
										$(SRC_MP3_ENC_PTAH)/version.c \
										$(SRC_MP3_ENC_PTAH)/mpglib_interface.c \
										$(SRC_MP3_ENC_PTAH)/VbrTag.c 

include $(BUILD_STATIC_LIBRARY)



#lib libspeex
include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_MODULE := libspeex
LOCAL_CFLAGS += -DANDROID
LOCAL_CFLAGS += -Wno-psabi
LOCAL_CFLAGS += -Wno-sequence-point -Wno-extra
LOCAL_CFLAGS += -DFIXED_POINT -DUSE_KISS_FFT -DEXPORT="" -UHAVE_CONFIG_H
LOCAL_LDLIBS := -llog -landroid

LOCAL_C_INCLUDES := $(INC_LIB_SPEEX_PTAH)


LOCAL_SRC_FILES :=  $(SRC_LIB_SPEEX_PTAH)/cb_search.c \
										$(SRC_LIB_SPEEX_PTAH)/exc_10_32_table.c \
										$(SRC_LIB_SPEEX_PTAH)/exc_8_128_table.c \
										$(SRC_LIB_SPEEX_PTAH)/filters.c \
										$(SRC_LIB_SPEEX_PTAH)/gain_table.c \
										$(SRC_LIB_SPEEX_PTAH)/hexc_table.c \
										$(SRC_LIB_SPEEX_PTAH)/high_lsp_tables.c \
										$(SRC_LIB_SPEEX_PTAH)/lsp.c \
										$(SRC_LIB_SPEEX_PTAH)/ltp.c \
										$(SRC_LIB_SPEEX_PTAH)/speex.c \
										$(SRC_LIB_SPEEX_PTAH)/stereo.c \
										$(SRC_LIB_SPEEX_PTAH)/vbr.c \
										$(SRC_LIB_SPEEX_PTAH)/vq.c \
										$(SRC_LIB_SPEEX_PTAH)/bits.c \
										$(SRC_LIB_SPEEX_PTAH)/exc_10_16_table.c \
										$(SRC_LIB_SPEEX_PTAH)/exc_20_32_table.c \
										$(SRC_LIB_SPEEX_PTAH)/exc_5_256_table.c \
										$(SRC_LIB_SPEEX_PTAH)/exc_5_64_table.c \
										$(SRC_LIB_SPEEX_PTAH)/gain_table_lbr.c \
										$(SRC_LIB_SPEEX_PTAH)/hexc_10_32_table.c \
										$(SRC_LIB_SPEEX_PTAH)/lpc.c \
										$(SRC_LIB_SPEEX_PTAH)/lsp_tables_nb.c \
										$(SRC_LIB_SPEEX_PTAH)/modes.c \
										$(SRC_LIB_SPEEX_PTAH)/modes_wb.c \
										$(SRC_LIB_SPEEX_PTAH)/nb_celp.c \
										$(SRC_LIB_SPEEX_PTAH)/quant_lsp.c \
										$(SRC_LIB_SPEEX_PTAH)/sb_celp.c \
										$(SRC_LIB_SPEEX_PTAH)/speex_callbacks.c \
										$(SRC_LIB_SPEEX_PTAH)/speex_header.c \
										$(SRC_LIB_SPEEX_PTAH)/window.c \
										$(SRC_LIB_SPEEX_PTAH)/kiss_fft.c \
										$(SRC_LIB_SPEEX_PTAH)/kiss_fftr.c \
										$(SRC_LIB_SPEEX_PTAH)/preprocess.c \
										$(SRC_LIB_SPEEX_PTAH)/jitter.c \
										$(SRC_LIB_SPEEX_PTAH)/mdf.c \
										$(SRC_LIB_SPEEX_PTAH)/fftwrap.c \
										$(SRC_LIB_SPEEX_PTAH)/filterbank.c \
										$(SRC_LIB_SPEEX_PTAH)/buffer.c \
										$(SRC_LIB_SPEEX_PTAH)/resample.c \
										$(SRC_LIB_SPEEX_PTAH)/scal.c \
			 							$(SRC_LIB_SPEEX_PTAH)/speex_jni.cpp \
			  						$(SRC_LIB_SPEEX_PTAH)/denoise.cpp

include $(BUILD_STATIC_LIBRARY)


###
#libfreeverb3
include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_MODULE := libfreeverb3
LOCAL_C_INCLUDES := $(INC_LIB_STREVERB_PTAH) 

LOCAL_CFLAGS :=-DLIBFV3_FLOAT=1 -DLIBSRATE2_FLOAT=1
LOCAL_CPPFLAGS :=-DLIBFV3_FLOAT=1 -DLIBSRATE2_FLOAT=1
LOCAL_SRC_FILES :=  $(SRC_LIB_STREVERB_PTAH)/freeverb/allpass.cpp \
 $(SRC_LIB_STREVERB_PTAH)/freeverb/biquad.cpp \
 $(SRC_LIB_STREVERB_PTAH)/freeverb/blockDelay.cpp \
 $(SRC_LIB_STREVERB_PTAH)/freeverb/comb.cpp \
 $(SRC_LIB_STREVERB_PTAH)/freeverb/compmodel.cpp \
 $(SRC_LIB_STREVERB_PTAH)/freeverb/delay.cpp \
 $(SRC_LIB_STREVERB_PTAH)/freeverb/efilter.cpp \
 $(SRC_LIB_STREVERB_PTAH)/freeverb/fir3bandsplit.cpp \
 $(SRC_LIB_STREVERB_PTAH)/freeverb/firfilter.cpp \
 $(SRC_LIB_STREVERB_PTAH)/freeverb/firwindow.cpp \
 $(SRC_LIB_STREVERB_PTAH)/freeverb/frag.cpp \
 $(SRC_LIB_STREVERB_PTAH)/freeverb/irbase.cpp \
 $(SRC_LIB_STREVERB_PTAH)/freeverb/irmodel2.cpp \
 $(SRC_LIB_STREVERB_PTAH)/freeverb/irmodel2zl.cpp \
 $(SRC_LIB_STREVERB_PTAH)/freeverb/irmodel3.cpp \
 $(SRC_LIB_STREVERB_PTAH)/freeverb/irmodel.cpp \
 $(SRC_LIB_STREVERB_PTAH)/freeverb/irmodels.cpp \
 $(SRC_LIB_STREVERB_PTAH)/freeverb/limitmodel.cpp \
 $(SRC_LIB_STREVERB_PTAH)/freeverb/nrevb.cpp \
 $(SRC_LIB_STREVERB_PTAH)/freeverb/nrev.cpp \
 $(SRC_LIB_STREVERB_PTAH)/freeverb/revbase.cpp \
 $(SRC_LIB_STREVERB_PTAH)/freeverb/revmodel.cpp \
 $(SRC_LIB_STREVERB_PTAH)/freeverb/rms.cpp \
 $(SRC_LIB_STREVERB_PTAH)/freeverb/scomp.cpp \
 $(SRC_LIB_STREVERB_PTAH)/freeverb/slimit.cpp \
 $(SRC_LIB_STREVERB_PTAH)/freeverb/slot.cpp \
 $(SRC_LIB_STREVERB_PTAH)/freeverb/src.cpp \
 $(SRC_LIB_STREVERB_PTAH)/freeverb/stenh.cpp \
 $(SRC_LIB_STREVERB_PTAH)/freeverb/strev.cpp \
 $(SRC_LIB_STREVERB_PTAH)/freeverb/utils.cpp 

LOCAL_SRC_FILES_EXT :=  $(SRC_LIB_STREVERB_PTAH)/freeverb/delay.cpp \
 $(SRC_LIB_STREVERB_PTAH)/freeverb/comb.cpp \
 $(SRC_LIB_STREVERB_PTAH)/freeverb/allpass.cpp \
 $(SRC_LIB_STREVERB_PTAH)/freeverb/src.cpp \
 $(SRC_LIB_STREVERB_PTAH)/freeverb/slot.cpp \
 $(SRC_LIB_STREVERB_PTAH)/freeverb/revbase.cpp \
 $(SRC_LIB_STREVERB_PTAH)/freeverb/nrev.cpp \
 $(SRC_LIB_STREVERB_PTAH)/freeverb/utils.cpp \
 $(SRC_LIB_STREVERB_PTAH)/freeverb/efilter.cpp \
 $(SRC_LIB_STREVERB_PTAH)/freeverb/biquad.cpp 

include $(BUILD_STATIC_LIBRARY)



#libsamplerate2
include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm
LOCAL_MODULE := libsamplerate2

LOCAL_C_INCLUDES := $(INC_LIB_STREVERB_PTAH)

LOCAL_CFLAGS :=-DLIBSRATE2_FLOAT=1
LOCAL_SRC_FILES :=  $(SRC_LIB_STREVERB_PTAH)/libsamplerate2/samplerate.c \
 $(SRC_LIB_STREVERB_PTAH)/libsamplerate2/samplerate_common.c \
 $(SRC_LIB_STREVERB_PTAH)/libsamplerate2/src_common.c \
 $(SRC_LIB_STREVERB_PTAH)/libsamplerate2/src_linear.c \
 $(SRC_LIB_STREVERB_PTAH)/libsamplerate2/src_sinc.c \
 $(SRC_LIB_STREVERB_PTAH)/libsamplerate2/src_zoh.c 

include $(BUILD_STATIC_LIBRARY)


#libfftw3
include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_C_INCLUDES := $(INC_LIB_STREVERB_PTAH)/fftw3 $(INC_LIB_STREVERB_PTAH)/ $(INC_LIB_STREVERB_PTAH)/fftw3/kernel \
$(INC_LIB_STREVERB_PTAH)/fftw3/dft \
$(INC_LIB_STREVERB_PTAH)/fftw3/rdft \
$(INC_LIB_STREVERB_PTAH)/fftw3/reodft \
$(INC_LIB_STREVERB_PTAH)/fftw3/dft/codelets \
$(INC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets
LOCAL_MODULE := libfftw3

LOCAL_SRC_FILES := $(SRC_LIB_STREVERB_PTAH)/fftw3/api/apiplan.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/configure.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/execute.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/execute-dft.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/execute-dft-c2r.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/execute-dft-r2c.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/execute-r2r.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/execute-split-dft.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/execute-split-dft-c2r.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/execute-split-dft-r2c.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/export-wisdom.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/export-wisdom-to-file.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/export-wisdom-to-string.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/extract-reim.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/f77api.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/flops.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/forget-wisdom.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/import-system-wisdom.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/import-wisdom.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/import-wisdom-from-file.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/import-wisdom-from-string.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/malloc.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/mapflags.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/map-r2r-kind.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/mkprinter-file.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/mktensor-iodims.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/mktensor-rowmajor.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/plan-dft-1d.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/plan-dft-2d.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/plan-dft-3d.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/plan-dft.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/plan-dft-c2r-1d.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/plan-dft-c2r-2d.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/plan-dft-c2r-3d.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/plan-dft-c2r.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/plan-dft-r2c-1d.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/plan-dft-r2c-2d.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/plan-dft-r2c-3d.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/plan-dft-r2c.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/plan-guru-dft.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/plan-guru-dft-c2r.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/plan-guru-dft-r2c.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/plan-guru-r2r.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/plan-guru-split-dft.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/plan-guru-split-dft-c2r.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/plan-guru-split-dft-r2c.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/plan-many-dft.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/plan-many-dft-c2r.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/plan-many-dft-r2c.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/plan-many-r2r.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/plan-r2r-1d.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/plan-r2r-2d.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/plan-r2r-3d.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/plan-r2r.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/print-plan.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/rdft2-pad.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/the-planner.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/api/version.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/kernel/align.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/kernel/alloc.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/kernel/assert.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/kernel/awake.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/kernel/buffered.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/kernel/cpy1d.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/kernel/cpy2d.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/kernel/cpy2d-pair.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/kernel/ct.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/kernel/debug.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/kernel/hash.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/kernel/iabs.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/kernel/kalloc.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/kernel/md5-1.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/kernel/md5.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/kernel/minmax.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/kernel/ops.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/kernel/pickdim.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/kernel/plan.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/kernel/planner.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/kernel/primes.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/kernel/print.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/kernel/problem.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/kernel/rader.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/kernel/scan.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/kernel/solver.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/kernel/solvtab.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/kernel/stride.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/kernel/tensor1.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/kernel/tensor2.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/kernel/tensor4.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/kernel/tensor5.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/kernel/tensor7.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/kernel/tensor8.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/kernel/tensor9.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/kernel/tensor.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/kernel/tile2d.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/kernel/timer.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/kernel/transpose.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/kernel/trig.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/kernel/twiddle.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/bluestein.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/buffered.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/conf.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/ct.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/ctsq.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/dftw-direct.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/dftw-genericbuf.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/dftw-generic.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/direct.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/generic.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/indirect.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/indirect-transpose.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/kdft.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/kdft-dif.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/kdft-difsq.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/kdft-dit.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/nop.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/plan.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/problem.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/rader.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/rank-geq2.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/solve.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/vrank-geq1.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/zero.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/n.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/t.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/standard/codlist.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/standard/n1_10.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/standard/n1_11.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/standard/n1_12.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/standard/n1_13.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/standard/n1_14.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/standard/n1_15.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/standard/n1_16.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/standard/n1_2.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/standard/n1_32.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/standard/n1_3.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/standard/n1_4.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/standard/n1_5.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/standard/n1_64.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/standard/n1_6.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/standard/n1_7.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/standard/n1_8.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/standard/n1_9.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/standard/q1_2.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/standard/q1_3.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/standard/q1_4.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/standard/q1_5.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/standard/q1_6.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/standard/q1_8.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/standard/t1_10.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/standard/t1_12.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/standard/t1_15.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/standard/t1_16.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/standard/t1_2.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/standard/t1_32.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/standard/t1_3.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/standard/t1_4.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/standard/t1_5.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/standard/t1_64.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/standard/t1_6.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/standard/t1_7.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/standard/t1_8.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/standard/t1_9.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/standard/t2_16.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/standard/t2_32.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/standard/t2_4.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/standard/t2_64.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/dft/codelets/standard/t2_8.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/buffered2.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/buffered.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/conf.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/dft-r2hc.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/dht-r2hc.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/dht-rader.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/direct2.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/direct.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/generic.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/hc2hc.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/hc2hc-common.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/hc2hc-directbuf.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/hc2hc-direct.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/hc2hc-generic.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/indirect.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/khc2hc.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/khc2r.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/kr2hc.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/kr2r.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/nop2.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/nop.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/plan2.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/plan.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/problem2.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/problem.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/rank0.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/rank0-rdft2.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/rank-geq2.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/rank-geq2-rdft2.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/rdft2-inplace-strides.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/rdft2-radix2.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/rdft2-strides.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/rdft2-tensor-max-index.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/rdft-dht.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/solve2.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/solve.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/vrank3-transpose.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/vrank-geq1.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/vrank-geq1-rdft2.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hfb.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2r.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/codlist.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hb_10.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hb_12.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hb_15.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hb_16.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hb_2.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hb_32.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hb_3.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hb_4.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hb_5.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hb_64.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hb_6.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hb_7.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hb_8.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hb_9.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hc2r_10.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hc2r_11.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hc2r_128.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hc2r_12.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hc2r_13.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hc2r_14.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hc2r_15.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hc2r_16.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hc2r_32.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hc2r_3.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hc2r_4.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hc2r_5.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hc2r_64.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hc2r_6.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hc2r_7.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hc2r_8.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hc2r_9.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hc2rIII_10.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hc2rIII_12.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hc2rIII_15.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hc2rIII_16.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hc2rIII_2.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hc2rIII_32.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hc2rIII_3.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hc2rIII_4.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hc2rIII_5.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hc2rIII_64.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hc2rIII_6.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hc2rIII_7.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hc2rIII_8.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/hc2r/hc2rIII_9.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/codlist.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/hf_10.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/hf_12.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/hf_15.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/hf_16.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/hf2_16.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/hf2_32.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/hf2_4.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/hf2_64.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/hf2_8.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/hf_2.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/hf_32.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/hf_3.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/hf_4.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/hf_5.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/hf_64.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/hf_6.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/hf_7.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/hf_8.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/hf_9.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/r2hc_10.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/r2hc_11.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/r2hc_128.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/r2hc_12.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/r2hc_13.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/r2hc_14.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/r2hc_15.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/r2hc_16.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/r2hc_2.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/r2hc_32.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/r2hc_3.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/r2hc_4.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/r2hc_5.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/r2hc_64.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/r2hc_6.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/r2hc_7.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/r2hc_8.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/r2hc_9.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/r2hcII_10.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/r2hcII_12.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/r2hcII_15.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/r2hcII_16.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/r2hcII_2.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/r2hcII_32.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/r2hcII_3.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/r2hcII_4.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/r2hcII_5.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/r2hcII_64.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/r2hcII_6.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/r2hcII_7.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/r2hcII_8.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2hc/r2hcII_9.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2r/codlist.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2r/e01_8.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/rdft/codelets/r2r/e10_8.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/reodft/conf.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/reodft/redft00e-r2hc.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/reodft/redft00e-r2hc-pad.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/reodft/reodft00e-splitradix.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/reodft/reodft010e-r2hc.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/reodft/reodft11e-r2hc.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/reodft/reodft11e-r2hc-odd.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/reodft/reodft11e-radix2.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/reodft/rodft00e-r2hc.c \
 $(SRC_LIB_STREVERB_PTAH)/fftw3/reodft/rodft00e-r2hc-pad.c 

include $(BUILD_STATIC_LIBRARY)