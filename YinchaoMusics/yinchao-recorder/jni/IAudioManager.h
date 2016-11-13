#ifndef __IAUDIO_MAMAGER__
#define __IAUDIO_MAMAGER__



typedef struct S_Audio_Format{
	int sampleRate;
	int channels;
	int sampleBits;
}S_Audio_Format;



class IAudioManager
{

public:
	virtual ~IAudioManager(){}
	virtual int Init()=0;
	virtual int UnInit()=0;
	virtual int SetNativeWindow(void* pNativeWindow)=0;
	virtual int setAudioFormat(int sampleRate,int channels,int sampleBits)=0;
	virtual int start()=0;
	virtual int stop()=0;

};


#endif __IAUDIO_MAMAGER__