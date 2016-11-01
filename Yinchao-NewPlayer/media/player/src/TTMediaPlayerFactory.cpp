// INCLUDES
#include "TTMediaPlayer.h"

ITTMediaPlayer* CTTMediaPlayerFactory::NewL(ITTMediaPlayerObserver* aPlayerObserver)
{
    char* myChar = "123";
    
	ITTMediaPlayer* pMediaPlayer = new CTTMediaPlayer(aPlayerObserver, myChar);

	return pMediaPlayer;
}
