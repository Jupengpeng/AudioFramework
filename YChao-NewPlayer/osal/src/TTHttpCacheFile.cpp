#include "TTHttpCacheFile.h"

CTTHttpCacheFile::CTTHttpCacheFile()
: iFileHandle(NULL)
, iCachedSize(0)
, iTotalSize(0)
, iIndexEnd(0)
, iCurIndex(0)
, iBufferUnit(NULL)
, iWriteBuffer(NULL)
, iWriteCount(0)
, iWriteIndex(0)
{	
	iCritical.Create();
}

TTInt CTTHttpCacheFile::Open(const TTChar* aUrl)
{
	iFileHandle = fopen(aUrl, "rb+");
	TTInt nLen = 0;
	if ((iFileHandle != NULL) && (TTKErrNone == fseek(iFileHandle, 0, SEEK_END)) && ((nLen = ftell(iFileHandle)) > 0))
	{	 
		iCachedSize = nLen;
		UnInitBufferUnit();
		return TTKErrNone;
	}
	
    return TTKErrAccessDenied;
}

TTInt CTTHttpCacheFile::Create(const TTChar* aUrl, TTInt aTotalSize)
{
	TTASSERT(aTotalSize > 0);
	iCritical.Lock();
	iFileHandle = fopen(aUrl, "wb+");
	if (iFileHandle == NULL)
	{
		iCritical.UnLock();
		return TTKErrPathNotFound;
	}

	iCachedSize = 0;
	iTotalSize = aTotalSize;

	UnInitBufferUnit();
	if(aTotalSize < BUFFER_UNIT_MAXSIZE) {
		TTInt nErr = InitBufferUnit(aTotalSize);
		if(nErr) {
			UnInitBufferUnit();
		}
	}
	iCritical.UnLock();
	return TTKErrNone;
}

CTTHttpCacheFile::~CTTHttpCacheFile()
{
	Close();
	iCritical.Destroy();
}

TTInt CTTHttpCacheFile::Read(void* aBuffer, TTInt aReadPos, TTInt aReadSize)
{
	TTASSERT((aBuffer != NULL) && (iFileHandle != 0));
	if(iBufferMode) {
		return ReadBuffer((TTUint8*)aBuffer, aReadPos, aReadSize);
	}

	iCritical.Lock();
	TTInt nReadSize = 0;
	if (((aReadPos + aReadSize <= iCachedSize) && (aReadPos >= 0)) 
		&& (0 == fseek(iFileHandle, aReadPos, SEEK_SET)))
	{
		nReadSize = fread(aBuffer, 1, aReadSize, iFileHandle);
	}
	iCritical.UnLock();
	return nReadSize;
}

TTInt CTTHttpCacheFile::Write(void* aBuffer, TTInt aWriteSize)
{
    TTASSERT((aBuffer != NULL) && (iFileHandle != 0) && (aWriteSize + iCachedSize <= iTotalSize));
	TTInt nWriteSize = 0;

	if(iBufferMode) {
		return WriteBuffer((TTUint8*)aBuffer, aWriteSize);
	}

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

TTInt CTTHttpCacheFile::ReadBuffer(TTUint8* aBuffer, TTInt aReadPos, TTInt aReadSize)
{
	TTASSERT((aBuffer != NULL));
	TTInt nSize = 0;
	TTInt nIndex = 0;

	iCritical.Lock();
	TTInt nReadSize = 0;
	TTInt nReadPos = aReadPos;
	TTInt idx  = 0;

	for(idx = 0; idx <= iIndexEnd; idx++) {
		nIndex = idx; 
		if(aReadPos >= iBufferUnit[nIndex]->iPosition && aReadPos < iBufferUnit[nIndex]->iPosition + iBufferUnit[nIndex]->iSize) {
			nSize =  iBufferUnit[nIndex]->iPosition + iBufferUnit[nIndex]->iSize - aReadPos;
			if(nSize >= aReadSize) {
				nSize = aReadSize;
				memcpy(aBuffer, iBufferUnit[nIndex]->iBuffer + aReadPos - iBufferUnit[nIndex]->iPosition, nSize);
				nReadSize += nSize;
				iCurIndex = nIndex;
				break;
			} else {
				memcpy(aBuffer, iBufferUnit[nIndex]->iBuffer + aReadPos - iBufferUnit[nIndex]->iPosition, nSize);
				nReadSize += nSize;
				aBuffer += nSize;
				aReadSize -= nSize;
				aReadPos += nSize;
			}
		}
	}
	iCritical.UnLock();

	return nReadSize;
}	

TTInt CTTHttpCacheFile::WriteBuffer(TTUint8* aBuffer, TTInt aWriteSize)
{
	TTInt nWriteSize = 0;
	TTInt nIndex = 0;
	TTCAutoLock Lock(&iCritical);
	nIndex = iIndexEnd;	
	if(iBufferUnit[nIndex]->iSize == 0){
		iBufferUnit[nIndex]->iPosition = iCachedSize;
	}

	nWriteSize = iBufferUnit[nIndex]->iTotalSize - iBufferUnit[nIndex]->iSize;		
	if(nWriteSize >= aWriteSize){
		nWriteSize = aWriteSize;
		memcpy(iBufferUnit[nIndex]->iBuffer + iBufferUnit[nIndex]->iSize, aBuffer, nWriteSize);
		iBufferUnit[nIndex]->iSize += nWriteSize;
		iCachedSize += nWriteSize;
	} else {
		memcpy(iBufferUnit[nIndex]->iBuffer + iBufferUnit[nIndex]->iSize, aBuffer, nWriteSize);
		iBufferUnit[nIndex]->iSize += nWriteSize;
		iCachedSize += nWriteSize;

		iIndexEnd++;			
		nIndex = iIndexEnd;	
		iBufferUnit[nIndex]->iSize = 0;

		nWriteSize += WriteBuffer(aBuffer + nWriteSize, aWriteSize - nWriteSize);
	}		

	return nWriteSize;
}


TTInt CTTHttpCacheFile::WriteFile(TTInt aCount)
{
	if(iBufferMode == 0 || iBufferUnit == NULL || iFileHandle == NULL)
		return TTKErrEof;

	TTInt nIndexEnd = 0;
	TTInt nWriteSize = 0;
	iCritical.Lock();
	if(iWriteIndex >= iIndexEnd && iCachedSize < iTotalSize) {
		iCritical.UnLock();
		return TTKErrNotReady;
	}

	if(aCount > 0) {
		if(iIndexEnd - iCurIndex < aCount) {
			iCritical.UnLock();
			return TTKErrNotReady;
		}
	}
	nIndexEnd = iIndexEnd;
	iCritical.UnLock();

	if(iWriteIndex > nIndexEnd) {
		if(iCachedSize == iTotalSize) {
			return TTKErrEof;
		} else {
			return TTKErrDiskFull;
		}
	}

	iCritical.Lock();
	if(iBufferUnit[iWriteIndex] == NULL && iBufferUnit[iWriteIndex]->iBuffer == NULL) {
		iCritical.UnLock();
		return TTKErrNotReady;
	}
	nWriteSize = iBufferUnit[iWriteIndex]->iSize;
	memcpy(iWriteBuffer, iBufferUnit[iWriteIndex]->iBuffer, nWriteSize);
	iCritical.UnLock();

	nWriteSize = fwrite(iWriteBuffer, 1, nWriteSize, iFileHandle);		
	if (nWriteSize > 0)
		iWriteCount += nWriteSize;

	if(iWriteIndex == nIndexEnd) {
		if(iWriteCount == iTotalSize) {
			iWriteIndex++;
			return TTKErrEof;
		} else {
			iWriteIndex++;
			return TTKErrDiskFull;
		}
	} else {
		iWriteIndex++;
	}

	return TTKErrNone;	
}

void CTTHttpCacheFile::Close()
{
	iCritical.Lock();
	if (iFileHandle != NULL)
	{
		fclose(iFileHandle);
		iFileHandle = NULL;
	}
	UnInitBufferUnit();
	iCritical.UnLock();
}

TTInt CTTHttpCacheFile::CachedSize()
{
	TTInt nCachedSize = 0;
	iCritical.Lock();
	nCachedSize = iCachedSize;
	iCritical.UnLock();
	return nCachedSize;
}

TTInt CTTHttpCacheFile::InitBufferUnit(int nSize)
{
	if(nSize >= BUFFER_UNIT_MAXSIZE)
		return TTKErrOverflow;
	
	iBufferCount = nSize/BUFFER_UNIT_SIZE + 1;

	iBufferUnit = new TTBufferUnit*[iBufferCount];

	TTInt n = 0;
	for(n = 0; n < iBufferCount; n++) {
		iBufferUnit[n] = new TTBufferUnit;

		iBufferUnit[n]->iPosition = 0;
		iBufferUnit[n]->iFlag = 0;
		iBufferUnit[n]->iSize = 0;

		iBufferUnit[n]->iBuffer = (TTUint8*)malloc(BUFFER_UNIT_SIZE);
		if(iBufferUnit[n]->iBuffer){
			iBufferUnit[n]->iTotalSize = BUFFER_UNIT_SIZE;
		} else {
			return TTKErrOverflow;
		}
	}

	iWriteBuffer = (TTUint8*)malloc(BUFFER_UNIT_SIZE);
	if(iWriteBuffer == NULL)
		return TTKErrOverflow;

	iBufferMode = 1;

	return TTKErrNone;
}

TTInt CTTHttpCacheFile::UnInitBufferUnit()
{
	TTInt n = 0;
	if(iBufferUnit != NULL) {
		for(n = 0; n < iBufferCount; n++) {
			if(iBufferUnit[n]) {
				SAFE_FREE(iBufferUnit[n]->iBuffer);
			}
			SAFE_FREE(iBufferUnit[n]);
		}
		SAFE_FREE(iBufferUnit);
	}

	SAFE_FREE(iWriteBuffer);

	iIndexEnd = 0;
	iBufferMode = 0;
	iBufferCount = 0;
	iWriteCount = 0;
	iWriteIndex = 0;
	return TTKErrNone;
}