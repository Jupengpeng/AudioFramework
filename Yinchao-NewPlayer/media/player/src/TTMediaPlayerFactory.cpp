// INCLUDES
#include "TTMediaPlayer.h"

ITTMediaPlayer* CTTMediaPlayerFactory::NewL(ITTMediaPlayerObserver* aPlayerObserver)
{
	ITTMediaPlayer* pMediaPlayer = new CTTMediaPlayer(aPlayerObserver, NULL);

	return pMediaPlayer;
}
