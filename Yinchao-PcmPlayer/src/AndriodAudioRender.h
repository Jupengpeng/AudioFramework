#ifdef _LINUX_ANDROID

#ifndef __ANDROIDAUDIORENDER
#define __ANDROIDAUDIORENDER

#include "IAudioRender.h"
#include "ulu_common.h"
#include <jni.h>

typedef struct S_Android_AudioTrack_Spec 
{
	enum StreamType 
	{
		STREAM_VOICE_CALL = 0,
		STREAM_SYSTEM = 1,
		STREAM_RING = 2,
		STREAM_MUSIC = 3,
		STREAM_ALARM = 4,
		STREAM_NOTIFICATION = 5,
	} stream_type;

	int sample_rate_in_hz;

	enum ChannelConfig 
	{
		CHANNEL_OUT_INVALID = 0x0,
		CHANNEL_OUT_DEFAULT = 0x1, /* f-l */
		CHANNEL_OUT_MONO = 0x4, /* f-l, f-r */
		CHANNEL_OUT_STEREO = 0xc, /* f-l, f-r, b-l, b-r */
		CHANNEL_OUT_QUAD = 0xcc, /* f-l, f-r, b-l, b-r */
		CHANNEL_OUT_SURROUND = 0x41c, /* f-l, f-r, f-c, b-c */
		CHANNEL_OUT_5POINT1 = 0xfc, /* f-l, f-r, b-l, b-r, f-c, low */
		CHANNEL_OUT_7POINT1 = 0x3fc, /* f-l, f-r, b-l, b-r, f-c, low, f-lc, f-rc */

		CHANNEL_OUT_FRONT_LEFT = 0x4,
		CHANNEL_OUT_FRONT_RIGHT = 0x8,
		CHANNEL_OUT_BACK_LEFT = 0x40,
		CHANNEL_OUT_BACK_RIGHT = 0x80,
		CHANNEL_OUT_FRONT_CENTER = 0x10,
		CHANNEL_OUT_LOW_FREQUENCY = 0x20,
		CHANNEL_OUT_FRONT_LEFT_OF_CENTER = 0x100,
		CHANNEL_OUT_FRONT_RIGHT_OF_CENTER = 0x200,
		CHANNEL_OUT_BACK_CENTER = 0x400,
	} channel_config;

	enum AudioFormat 
	{
		ENCODING_INVALID = 0,
		ENCODING_DEFAULT = 1,
		ENCODING_PCM_16BIT = 2, // signed, guaranteed to be supported by devices.
		ENCODING_PCM_8BIT = 3, // unsigned, not guaranteed to be supported by devices.
	} audio_format;

	int buffer_size_in_bytes;

	enum Mode 
	{
		MODE_STATIC = 0,
		MODE_STREAM = 1,
	} mode;

	// extra field
	int isamples;
} S_Android_AudioTrack_Spec;


typedef struct S_Android_AudioTrack 
{
	jobject thiz;
	S_Android_AudioTrack_Spec spec;
	jbyteArray buffer;
	int buffer_capacity;
	int min_buffer_size;
	float max_volume;
	float min_volume;
} S_Android_AudioTrack;

typedef struct AudioTrack_fields_t 
{
	jclass clazz;
	jmethodID constructor;
	jmethodID getMinBufferSize;
	jmethodID getMaxVolume;
	jmethodID getMinVolume;
	jmethodID getNativeOutputSampleRate;

	jmethodID play;
	jmethodID pause;
	jmethodID flush;
	jmethodID stop;
	jmethodID release;
	jmethodID write_byte;
	jmethodID setStereoVolume;
	jmethodID getLatency;
} AudioTrack_fields_t;


class AndroidAudioRender :public IAudioRender
{
public:
	AndroidAudioRender();
	virtual ~AndroidAudioRender();
	virtual int						Init();
	virtual int						UnInit();
	virtual int						SetAudioFormat(AYMediaAudioFormat*   pAudioFormat);
	virtual int						SetNativeWindow(void*  pNativeWindow);
	virtual int						Start();
	virtual int						Pause();
	virtual int						Stop();
	virtual int						Render(unsigned char*   pData, unsigned int  ulDataSize);
	virtual int						Flush();
	virtual int						GetPlayingTime(uint64*  pullTime);
	virtual int						GetBufferTime(uint64*  pullTime);
	virtual int						SetCallBack(void*   pCallbackFunc);
	virtual int						GetRemainSize();
	virtual int						SetVolume(int iVolume);
	virtual int						GetVolume();

	virtual int						GetLatency();
private:
	int								InitAndroidAudioSystem();
	int								AudiotrackGetMinBufferSize();
	bool							AudiotrackGetMaxVolume(float&  fMaxVolume);
	bool							AudiotrackGetMinVolume(float&  fMinVolume);	
	int								GetAndroidTrackFromAudioSpec();
	int								GetReserveBuffer(unsigned int ulDataSize);
	int								AndroidAudioTrackRelease();	
	void							AndroidAudioTrackFree();
	JNIEnv*							GetJNIEnv();

	int								m_iChannelCount;
	int								m_iSampleRate;
	int								m_iSampleBits;
	S_Android_AudioTrack_Spec		m_sAndroidAudioSpec;
	void*							m_pJVM;
	void*							m_pJEnv;
	S_Android_AudioTrack			m_sAndroidAudioTrack;
	AudioTrack_fields_t				m_sAndroidAudioFields;
	int								m_iCurVolumeInPer;
};

#endif

#endif