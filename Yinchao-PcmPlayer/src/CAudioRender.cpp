#include <stdio.h>
#include "IAudioRender.h"
#include "AndriodAudioRender.h"
#include "CIOSAudioRender.h"

bool CreateIOSAudioRender(IAudioRender *&pAudioRender)
{
#ifdef __APPLE__
    if (pAudioRender != NULL)
    {
        return false;
    }
    else
    {
        pAudioRender = new CIOSAudioRender();
    }
    return true;
#else
    return false;
#endif
    
}

bool CreateAndroidAudioTrackRender(IAudioRender *&pAudioRender)
{
#ifdef _LINUX_ANDROID
	if (pAudioRender != NULL)
	{
		return false;
	}
	else
	{
		pAudioRender = new AndroidAudioRender();
	}
	return true;
#else
	return false;
#endif
}
bool DeleteAudioRender(IAudioRender *&pAudioRender)
{
    if (pAudioRender == NULL) 
	{
		return false;
	}
	else
	{
		delete pAudioRender;
		pAudioRender = NULL;
		return true;
	}
}