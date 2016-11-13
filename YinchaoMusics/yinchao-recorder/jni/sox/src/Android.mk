LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libYCMediaRecoder
LOCAL_ARM_MODE := arm
LOCAL_C_INCLUDES := $(LOCAL_PATH) \
$(LOCAL_PATH)/../libogg/include/ \
$(LOCAL_PATH)/../vorbis/include/ \
$(LOCAL_PATH)/../flac/include/ \
$(LOCAL_PATH)/../lame-3.98.4/include/ \
$(LOCAL_PATH)/../libmad-0.15.1b/ \
$(LOCAL_PATH)/../libpng-1.5.2/include/ \
$(LOCAL_PATH)/../libsndfile-1.0.24/src/ \
$(LOCAL_PATH)/../wavpack-4.60.1/ \
$(LOCAL_PATH)/../ffmpeg/ \
$(LOCAL_PATH)/../ladspa/ \
$(LOCAL_PATH)/../libgsm/ \
$(LOCAL_PATH)/../../ \
$(LOCAL_PATH)/../lpc10/ \
$(LOCAL_PATH)/../../android_external_alsa-lib/include/ \

LOCAL_CFLAGS           := -Wall -g


LOCAL_SRC_FILES :=  \
							sox.c    \
							adpcms.c \
							aiff.c \
							cvsd.c \
	            g711.c \
	            g721.c \
	            g723_24.c \
	            g723_40.c \
	            g72x.c vox.c \
 			 	      raw.c \
 			 	      formats.c \
 			 	      formats_i.c \
 			 	      skelform.c \
							xmalloc.c \
							getopt.c \
							getopt1.c \
							util.c \
							libsox.c \
							libsox_i.c \
							sox-fmt.c \
        			bend.c \
        			biquad.c \
        			biquads.c \
        			chorus.c \
        			compand.c \
        			crop.c \
							compandt.c \
							contrast.c \
							dcshift.c \
							delay.c \
							dft_filter.c \
							dither.c \
							divide.c \
							earwax.c \
							echo.c \
							echos.c \
							effects.c \
							effects_i.c \
							effects_i_dsp.c \
							fade.c fft4g.c \
							filter.c \
							fir.c \
							firfit.c \
							flanger.c \
							gain.c \
							input.c \
							ladspa.c \
							loudness.c \
							mcompand.c \
							mixer.c \
							noiseprof.c \
							noisered.c \
							output.c \
							overdrive.c \
							pad.c \
							pan.c \
							phaser.c \
							rate.c \
							remix.c \
							repeat.c \
							reverb.c \
							reverse.c \
							silence.c \
							sinc.c \
							skeleff.c \
							speed.c \
							speexdsp.c \
							splice.c \
							stat.c \
							stats.c \
							stretch.c \
							swap.c \
							synth.c \
							tempo.c \
							tremolo.c \
							trim.c \
							vad.c \
							vol.c \
        			raw-fmt.c \
        			s1-fmt.c \
        			s2-fmt.c \
        			s3-fmt.c \
        			s4-fmt.c \
        			u1-fmt.c \
        			u2-fmt.c \
        			u3-fmt.c \
        			u4-fmt.c \
        			al-fmt.c \
        			la-fmt.c \
        			ul-fmt.c \
        			lu-fmt.c \
        			8svx.c \
        			aiff-fmt.c \
        			aifc-fmt.c \
        			au.c \
        			avr.c \
        			cdr.c \
        			cvsd-fmt.c \
        			dvms-fmt.c \
        			dat.c \
        			hcom.c \
        			htk.c \
        			maud.c \
        			prc.c \
        			sf.c \
        			smp.c \
        			sounder.c \
        			soundtool.c \
        			sphere.c \
        			tx16w.c \
        			voc.c \
        			vox-fmt.c \
        			ima-fmt.c \
        			adpcm.c \
        			ima_rw.c \
        			wve.c \
        			xa.c \
        			nulfile.c \
        			f4-fmt.c \
        			f8-fmt.c \
        			gsrt.c \
        			../../TTMediaRecoder_jni.cpp \
        			../../AndroidAudioManager.cpp \
        			../../YCBaseEffect.cpp 

#LOCAL_SHARED_LIBRARIES := liblpc10 libgsm libfmemopen libogg libvorbis libvorbisenc libvorbisfile libFLAC libmp3lame libmad libpng libsndfile libwavpack
LOCAL_LDLIBS := -ldl -llog  -L$(DIRECTORY_TO_OBJ)


include $(BUILD_SHARED_LIBRARY)






