#include "AYMediaPlayer.h"

int main()
{
	IAYMediaPlayer *pMediaPlayer;
	bool iRet = createMediaPlayerInstance(NULL,&pMediaPlayer);
	char*record ="record.pcm";
	pMediaPlayer->setParam(PID_RECORD_FILE_PATH,(void*)record);
	char*backgroud ="accompany.pcm";
	pMediaPlayer->setParam(PID_BACKGROUD_FILE_PATH,(void*)backgroud);

	int volume =100;
	pMediaPlayer->setParam(PID_AUDIO_RECORD_VOLUME,(void*)&volume);
	pMediaPlayer->setParam(PID_AUDIO_BACKGROUD_VOLUME,(void*)&volume);
	AYMediaAudioFormat recordFormat;
	recordFormat.nSamplesPerSec=44100;
	recordFormat.nChannels =1;
	recordFormat.wBitsPerSample =16;
	pMediaPlayer->setParam(PID_RECORDER_AUDIO_FORMAT,(void*)&recordFormat);


	AYMediaAudioFormat backGroudFormat;
	backGroudFormat.nSamplesPerSec=44100;
	backGroudFormat.nChannels =2;
	backGroudFormat.wBitsPerSample =16;
	pMediaPlayer->setParam(PID_BACKGROUD_AUDIO_FORMAT,(void*)&backGroudFormat);
	pMediaPlayer->run();
	system("pause");
}