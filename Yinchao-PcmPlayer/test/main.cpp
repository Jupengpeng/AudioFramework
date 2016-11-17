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
	pMediaPlayer->run();
	system("pause");
}