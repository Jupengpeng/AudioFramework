#include "TTOsalConfig.h"
#include "TTSleep.h"

#if (defined __TT_OS_WINDOWS__)//WINDOWS
#include <time.h>
#include <Windows.h>

void TTSleep(unsigned int mSec)
{
	Sleep (mSec);
}

#elif (defined __TT_OS_SYMBIAN__)//SYMBIAN
#include <unistd.h>

void TTSleep(usigned int mSec);
{
	User::After (mSec * 1000);
}

#elif (defined __TT_OS_ANDROID__ || defined (__TT_OS_IOS__))//ANDROID
#include <unistd.h>

void TTSleep(unsigned int mSec)
{
	usleep (mSec*1000);
}
#endif

//end of file
