/**
 * File : TTPackagePathFetcher.cpp
 * Created on : 2012-3-21
 * Author : hu.cao
 * Copyright : Copyright (c) 2010 Shuidushi Software Ltd. All rights reserved.
 * Description : TTPackagePathFetcher  µœ÷Œƒº˛
 */
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
	return gCacheFilePath == NULL ? "/sdcard/cache.tmp" : gCacheFilePath;
}

const bool gGetCacheFileEnble()
{
	bool bCanWrite = true;
	if(gCacheFilePath == NULL)
		return false;
	FILE* Handle = fopen(gCacheFilePath, "ab+");
	if (Handle == NULL){
		bCanWrite = false;
	} else {
		fclose(Handle);
	}

	return bCanWrite;
}
