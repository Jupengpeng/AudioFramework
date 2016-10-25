#include "TTTypedef.h"
#include "TTMacrodef.h"
#include <stdlib.h>
#include "TTLog.h"

static TTChar* gCacheFilePath = NULL;

void gSetCacheFilePath(const TTChar* aCacheFilePath)
{
	LOGI("CacheFilePath:%s", aCacheFilePath);
	gCacheFilePath = (TTChar*)aCacheFilePath;
}

const TTChar* gGetCacheFilePath()
{
	return gCacheFilePath == NULL ? "C:\\cache.tmp" : gCacheFilePath;
}

const bool gGetCacheFileEnble()
{
	bool bCanWrite = true;
	FILE* Handle = fopen(gGetCacheFilePath(), "ab+");
	if (Handle == NULL){
		bCanWrite = false;
	} else {
		fclose(Handle);
	}

	return bCanWrite;
}
