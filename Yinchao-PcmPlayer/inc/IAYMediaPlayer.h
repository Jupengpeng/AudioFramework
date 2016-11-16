#ifndef __IAYMediaPlayer_H__
#define __IAYMediaPlayer_H__

#ifdef _WINDOWS
#ifdef AYMEDIAPLAYER_SDK_EXPORTS
#define AYMEDIAPLAYER_SDK_API extern "C" __declspec(dllexport)
#else	// AYMEDIAPLAYER_SDK_EXPORTS
#define AYMEDIAPLAYER_SDK_API extern "C" __declspec(dllimport)
#endif	// AYMEDIAPLAYER_SDK_EXPORTS
#else	// _WINDOWS
#define AYMEDIAPLAYER_SDK_API extern "C" 
#endif	// _WINDOWS

typedef struct tagMediaPlayerInitParam 
{
	void *							piCallback;	
	void *							pCallbackUserData;
} MediaPlayerInitParam;

typedef struct AYMediaAudioFormat{
	int nSamplesPerSec;
	int nChannels;
	int wBitsPerSample;
}AYMediaAudioFormat;

//paremeter Id
#define PID_JAVA_VM							0X0001
#define PID_RECORD_FILE_PATH				0X0002
#define PID_BACKGROUD_FILE_PATH				0X0003
#define PID_AUDIO_VOLUME					0X0004
#define PID_RECORDER_AUDIO_FORMAT			0X0005
#define PID_BACKGROUD_AUDIO_FORMAT			0X0006

/****************************************************************************************
IAYMediaPlayer : AY Media Player Function Interface
*****************************************************************************************/
class IAYMediaPlayer
{
public:
	IAYMediaPlayer() {}
	virtual ~IAYMediaPlayer() {}

public:
	virtual int 				run()=0;

	virtual int					pause()=0;

	virtual int					stop()=0;

	virtual int					resume()=0;

	// puPosition is the desired position, but it will change after seekTo return since I frame or other reasons
	virtual int					seekTo(unsigned int * puPosition)=0;

	virtual int					getCurrentPosition(unsigned int * puPosition)=0;

	virtual int					setParam(unsigned int uParamId, void * pParam)=0;

	virtual int					getParam(unsigned int uParamId, void * pParam)=0;
};

/****************************************************************************************
AY Media Player Factory Function 
*****************************************************************************************/
AYMEDIAPLAYER_SDK_API bool createMediaPlayerInstance(MediaPlayerInitParam * pInitParam, IAYMediaPlayer ** ppiMediaPlayer);
AYMEDIAPLAYER_SDK_API void destroyMediaPlayerInstance(IAYMediaPlayer * &piMediaPlayer);

#endif	// __IAYMediaPlayer_H__
