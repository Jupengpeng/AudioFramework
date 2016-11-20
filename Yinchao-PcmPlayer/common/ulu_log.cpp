#include "ulu_log.h"


#ifdef _WINDOWS
#include "stdio.h"
#include "stdarg.h"
#include "string.h"

FILE*   gpLogDump = NULL;
void  LogPrint(const char *__fmt, ...)
{
	if (gpLogDump == NULL)
	{
#ifdef _AY_NET_SDK_
		gpLogDump = fopen("D:/ululog_buffer.log", "wb");
#else
		gpLogDump = fopen("D:/ululog_player.log", "wb");
#endif
	}

	if (NULL != gpLogDump)
	{
		char		szvoLog[1024] = { 0 };
		char		szvoLogTemp[1024] = { 0 };
		va_list args;
		va_start(args, __fmt);
		_vsnprintf(szvoLogTemp, 1024, __fmt, args);
		_snprintf(szvoLog, 1024, "%s\n", szvoLogTemp);
		va_end(args);
		if (NULL != gpLogDump)
		{
			fwrite(szvoLog, 1, strlen(szvoLog), gpLogDump);
			fflush(gpLogDump);
		}
	}

}

#endif