#include "STHttpCacheFile.h"
#include "STLog.h"

STHttpCacheFile::STHttpCacheFile()
: iFileHandle(NULL)
, iCachedSize(0)
, iTotalSize(0)
{	
	iCritical.Create();
}

STInt STHttpCacheFile::Open(const STChar* aUrl)
{
	iFileHandle = fopen(aUrl, "rb+");
	STInt nLen = 0;
	if ((iFileHandle != NULL) && (STKErrNone == fseek(iFileHandle, 0, SEEK_END)) && ((nLen = ftell(iFileHandle)) > 0))
	{	 
		iCachedSize = nLen;
		return STKErrNone;
	}
	
    return STKErrAccessDenied;
}

STInt STHttpCacheFile::Create(const STChar* aUrl)
{
	iFileHandle = fopen(aUrl, "wb+");
	if (iFileHandle == NULL)
	{
		return STKErrPathNotFound;
	}

	iCachedSize = 0;
	return STKErrNone;

}

STHttpCacheFile::~STHttpCacheFile()
{
	Close();
	iCritical.Destroy();
}

STInt STHttpCacheFile::Read(void* aBuffer, STInt aReadPos, STInt aReadSize)
{
	STASSERT((aBuffer != NULL) && (iFileHandle != 0));

	iCritical.Lock();
	STInt nReadSize = 0;
	if (((aReadPos + aReadSize <= iCachedSize) && (aReadPos >= 0)) && (0 == fseek(iFileHandle, aReadPos, SEEK_SET)))
	{
		nReadSize = fread(aBuffer, 1, aReadSize, iFileHandle);
	}
	iCritical.UnLock();
	return nReadSize;
}

STInt STHttpCacheFile::Write(void* aBuffer, STInt aWriteSize)
{
	STInt nWriteSize = 0;
	iCritical.Lock();
	if (0 == fseek(iFileHandle, iCachedSize, SEEK_SET))
	{
		nWriteSize = fwrite(aBuffer, 1, aWriteSize, iFileHandle);
		if (nWriteSize > 0)
		{
			iCachedSize += nWriteSize;
		}
	}
	iCritical.UnLock();

	return nWriteSize;
}

STInt STHttpCacheFile::CachedSize()
{
	STInt nCache= 0;
	iCritical.Lock();
	nCache = iCachedSize;
	iCritical.UnLock();
	return nCache;
}


STInt STHttpCacheFile::CachedPercent()
{
	STInt nCachedPercent = 0;
	iCritical.Lock();
	if(iTotalSize > 0)
	{
		STLOGE("STHttpCacheFile::CachedSize:iCachedSize=%d,iTotalSize = %d",iCachedSize,iTotalSize);
		nCachedPercent = ((STDouble)iCachedSize/iTotalSize)*100;
	}

	iCritical.UnLock();

	return nCachedPercent;
}

void STHttpCacheFile::SetTotalSize(STInt aTotalSize)
{
	iCritical.Lock();
	iTotalSize = aTotalSize;
	iCritical.UnLock();
}

STInt STHttpCacheFile::TotalSize()
{
	iCritical.Lock();
	STInt nTotalSize = iTotalSize;
	iCritical.UnLock();

	return nTotalSize;
}

void STHttpCacheFile::Close()
{
	if (iFileHandle != NULL)
	{
		fclose(iFileHandle);
		iFileHandle = NULL;
	}
}
