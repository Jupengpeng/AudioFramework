#ifndef __IAUDIO_RENDER__
#define __IAUDIO_RENDER__
#include "cm_type.h"
#include "IAYMediaPlayer.h"
#define   AUDIO_COMMIT_NEED_RETRY     1
#define   AUDIO_RENDER_DEVICE_ERROR   0x1000

class IAudioRender
{
public:
	virtual ~IAudioRender(){}
	virtual int Init() = 0;
	virtual int UnInit() = 0;
	virtual int SetAudioFormat(AYMediaAudioFormat*pAudioFormat) = 0;
	virtual int SetNativeWindow(void*  pNativeWindow) = 0;
	virtual int Start() = 0;
	virtual int Pause() = 0;
	virtual int Stop() = 0;
	virtual int Render(unsigned char*   pData, unsigned int  ulDataSize) = 0;
	virtual int Flush() = 0;
	virtual int GetPlayingTime(uint64*  pullTime) = 0;
	virtual int GetBufferTime(uint64*  pullTime) = 0;
	virtual int SetCallBack(void*   pCallbackFunc) = 0;
	virtual int GetRemainSize() = 0;
	virtual int SetVolume(int iVolume) = 0;
	virtual int GetVolume() = 0;
	virtual int GetLatency() = 0;;
};

bool CreateIOSAudioRender(IAudioRender *&pAudioRender);
bool CreateAndroidAudioTrackRender(IAudioRender *&pAudioRender);
bool DeleteAudioRender(IAudioRender *&pAudioRender);
#endif