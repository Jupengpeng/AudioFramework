#include "TTCacheBuffer.h"

CTTCacheBuffer::CTTCacheBuffer()
: iCachedCount(0)
, iTotalSize(0)
, iIndexStart(0)
, iIndexEnd(0)
{	
	iCritical.Create();

	memset(iPage, 0, sizeof(TTBufferPage)*PAGE_BUFFER_COUNT);
}

TTInt CTTCacheBuffer::Open()
{
    InitPage();
	return TTKErrNone;
}

CTTCacheBuffer::~CTTCacheBuffer()
{
	Close();

	TTInt i;
	iCritical.Lock();
	for(i = 0; i < PAGE_BUFFER_COUNT; i++)
	{
		if(iPage[i].iBuffer)
		{
			SAFE_FREE(iPage[i].iBuffer);
			iPage[i].iTotalSize = 0;
		}
	}
	iCritical.UnLock();

	iCritical.Destroy();
}

void CTTCacheBuffer::InitPage()
{
	TTInt i;

	TTCAutoLock Lock(&iCritical);
	iCachedCount = 0;
	for(i = 0; i < PAGE_BUFFER_COUNT; i++)
	{
		iPage[i].iPosition = 0;
		iPage[i].iFlag = 0;
		iPage[i].iSize = 0;

		if(iPage[i].iBuffer == NULL)
		{
			iPage[i].iBuffer = (TTUint8*)malloc(PAGE_BUFFER_SIZE);
			if(iPage[i].iBuffer)
			{
				iPage[i].iTotalSize = PAGE_BUFFER_SIZE;
				iCachedCount++;
			}
		}
	}
	
	iIndexStart = 0; 
	iIndexEnd = 0;
}

void CTTCacheBuffer::UnInitPage()
{
	TTInt i;

	TTCAutoLock Lock(&iCritical);
	for(i = 0; i < PAGE_BUFFER_COUNT; i++)
	{
		iPage[i].iPosition = 0;
		iPage[i].iFlag = 0;
		iPage[i].iSize = 0;
	}
}

TTInt CTTCacheBuffer::Read(TTUint8* aBuffer, TTInt aReadPos, TTInt aReadSize)
{
	TTASSERT((aBuffer != NULL));
	TTInt nSize = 0;
	TTInt nIndex = 0;

	iCritical.Lock();
	TTInt nReadSize = 0;
	TTInt nReadPos = aReadPos;
	TTInt idx  = -1;

	for(idx = iIndexStart; idx <= iIndexEnd; idx++)
	{
		nIndex = idx%PAGE_BUFFER_COUNT; 
		if(aReadPos >= iPage[nIndex].iPosition && aReadPos < iPage[nIndex].iPosition + iPage[nIndex].iSize)
		{
			nSize =  iPage[nIndex].iPosition + iPage[nIndex].iSize - aReadPos;
			if(nSize >= aReadSize)
			{
				nSize = aReadSize;
				memcpy(aBuffer, iPage[nIndex].iBuffer + aReadPos - iPage[nIndex].iPosition, nSize);
				nReadSize += nSize;
				break;
			}
			else
			{
				memcpy(aBuffer, iPage[nIndex].iBuffer + aReadPos - iPage[nIndex].iPosition, nSize);
				nReadSize += nSize;
				aBuffer += nSize;
				aReadSize -= nSize;
				aReadPos += nSize;
			}
		}
	}
	
	if(idx <= iIndexEnd && idx - 1 > iIndexStart)
	{
		nIndex = iIndexEnd%PAGE_BUFFER_COUNT;
		TTInt64 nEnd = iPage[nIndex].iPosition + iPage[nIndex].iSize;
		TTInt nLimit = iCachedCount - 2;
		if(nLimit <= 0) nLimit = 1;
		if(nEnd != iTotalSize && iIndexEnd - iIndexStart >= nLimit)
		{
			iIndexStart++;
		}
	}		
	iCritical.UnLock();

	return nReadSize;
}

TTInt CTTCacheBuffer::Write(TTUint8* aBuffer, TTInt aWritePos, TTInt aWriteSize)
{
    TTASSERT((aBuffer != NULL) && (aWriteSize > 0));

	TTInt nWriteSize = 0;
	TTInt nIndex = 0;
	TTCAutoLock Lock(&iCritical);
	nIndex = iIndexEnd%PAGE_BUFFER_COUNT;	
	if(iPage[nIndex].iSize == 0)
	{
		if(iPage[nIndex].iTotalSize == 0)
		{
			iIndexEnd++;
			nIndex = iIndexEnd%PAGE_BUFFER_COUNT;	
			iPage[nIndex].iSize = 0;

			nWriteSize += Write(aBuffer, aWritePos, aWriteSize);

			return nWriteSize;
		}

		iPage[nIndex].iPosition = aWritePos;
	}

	if(aWritePos == iPage[nIndex].iPosition + iPage[nIndex].iSize)
	{
		nWriteSize = iPage[nIndex].iTotalSize - iPage[nIndex].iSize;		
		if(nWriteSize >= aWriteSize)
		{
			nWriteSize = aWriteSize;
			memcpy(iPage[nIndex].iBuffer + iPage[nIndex].iSize, aBuffer, nWriteSize);
			iPage[nIndex].iSize += nWriteSize;
		}
		else
		{
			memcpy(iPage[nIndex].iBuffer + iPage[nIndex].iSize, aBuffer, nWriteSize);
			iPage[nIndex].iSize += nWriteSize;

			iIndexEnd++;			
			nIndex = iIndexEnd%PAGE_BUFFER_COUNT;	
			iPage[nIndex].iSize = 0;

			nWriteSize += Write(aBuffer + nWriteSize, aWritePos + nWriteSize, aWriteSize - nWriteSize);
		}		
	}
	else
	{
		iIndexEnd++;
		iIndexStart = iIndexEnd;
		nIndex = iIndexEnd%PAGE_BUFFER_COUNT;	
		iPage[nIndex].iSize = 0;

		nWriteSize += Write(aBuffer, aWritePos, aWriteSize);
	}

	return nWriteSize;
}

void CTTCacheBuffer::Close()
{
	UnInitPage();
}

TTInt CTTCacheBuffer::CachePoistion(TTInt64& aCacheBegin, TTInt64& aCacheEnd)
{
	TTInt nCachedSize = 0;
	TTInt nIndex = 0; 
	iCritical.Lock();
	nIndex = iIndexStart%PAGE_BUFFER_COUNT;	
	aCacheBegin = iPage[nIndex].iPosition;

	nIndex = iIndexEnd%PAGE_BUFFER_COUNT;
	aCacheEnd = iPage[nIndex].iPosition + iPage[nIndex].iSize;

	iCritical.UnLock();
	return nCachedSize;
}