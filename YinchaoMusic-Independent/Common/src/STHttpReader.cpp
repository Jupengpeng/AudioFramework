#include <string.h>
#include <stdio.h>
#include <errno.h>
#ifdef __ST_OS_ANDROID__
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#endif


#include "STUrlUtils.h"
#include "STHttpReader.h"
#include "STHttpClient.h"
#include "STHttpCacheFile.h"
#include "STLog.h"
#include "STTypedef.h"
#include "STSysTime.h"

static const STChar* KDownloadThreadName = "DownloadThread";
static const STInt	 KBUFFER_SIZE = 4 * KILO;

STHttpReader::STHttpReader()
:iHttpClient(NULL)
,iHttpCacheFile(NULL)
,iReadStatus(ETTReadStatusNotReady)
,iDecodeThreadHandle(STThread::Self())
,iUrl(NULL)
,iCachePath(NULL)
,iAborted(ESTFalse)
{
	iSemaphore.Create();
}

STHttpReader::~STHttpReader()
{
	Close();
	SAFE_FREE(iCachePath);
	iSemaphore.Destroy();
}

void STHttpReader::SetCachePath(const STChar* aPath)
{
	SAFE_FREE(iCachePath);
	iCachePath = (STChar*) malloc((strlen(aPath) + 1) * sizeof(STChar));
	strcpy(iCachePath, aPath);
}

STInt STHttpReader::Open(const STChar* aUrl)
{
	STInt nErr = STKErrNotSupported;
	SAFE_FREE(iUrl);
	iUrl = (STChar*) malloc((strlen(aUrl) + 1) * sizeof(STChar));
	strcpy(iUrl, aUrl);

	STASSERT(iHttpClient == NULL);
	STASSERT(iHttpCacheFile == NULL);

	iReadStatus = ETTReadStatusNotReady;

	iHttpCacheFile = new STHttpCacheFile();
	nErr = iHttpCacheFile->Open(iCachePath);
	if (STKErrNone != nErr)
	{
		nErr = iHttpCacheFile->Create(iCachePath);
	}

	if ((STKErrNone	!= nErr) || (STKErrNone != (nErr = iThreadHandle.Create(DownloadThreadProc, this, 0))))
	{
		SAFE_DELETE(iHttpCacheFile);
	}

	return nErr;
}

void STHttpReader::Abort()
{
	iSemaphore.LockContext();
	if (iHttpClient != NULL)
	{
		iHttpClient->Abort();
	}
	iAborted = ESTTrue;
	iSemaphore.UnlockContext();
}

STInt STHttpReader::Close()
{
	iThreadHandle.Close();
	SAFE_DELETE(iHttpCacheFile);
	SAFE_FREE(iCachePath);
	SAFE_FREE(iUrl);
	return STKErrNone;
}

STInt STHttpReader::Read(STUint8* aReadBuffer, STInt aReadPos, STInt aReadSize)
{
	STASSERT(iHttpCacheFile != NULL);

	STInt nErr = STKErrUnderflow;

	while(ESTTrue)
	{
		iSemaphore.LockContext();
		if (iReadStatus == ETTReadStatusFailed || iReadStatus == ETTReadStatusCompleted)
		{
			iSemaphore.UnlockContext();
			nErr = iHttpCacheFile->Read(aReadBuffer, aReadPos, aReadSize);
			break;
		}
		else if (iReadStatus == ETTReadStatusNotReady || iReadStatus == ETTReadStatusReading)
		{
			if (aReadPos + aReadSize > iHttpCacheFile->CachedSize())
			{
				iSemaphore.Wait();
				iSemaphore.UnlockContext();
			}
			else
			{
				iSemaphore.UnlockContext();
				nErr = iHttpCacheFile->Read(aReadBuffer, aReadPos, aReadSize);
				break;
			}
		}
	}

	return nErr;
}

STInt STHttpReader::Size() const
{
	STASSERT(iHttpCacheFile != NULL);
	return iHttpCacheFile->TotalSize();
}

STInt STHttpReader::GetDownloadPercent()
{
	STASSERT(iHttpCacheFile != NULL);
	//iHttpCacheFile->iTotalSize
	return iHttpCacheFile->CachedPercent();
}

void* STHttpReader::DownloadThreadProc(void* aPtr)
{
	STHttpReader* pReaderProxy = reinterpret_cast<STHttpReader*>(aPtr);
	pReaderProxy->DownloadThreadProcL(NULL);
	return NULL;
}

void STHttpReader::DownloadThreadProcL(void* aPtr)
{
	iSemaphore.LockContext();
	if (iAborted)
	{
		iSemaphore.UnlockContext();
		return;
	}

	iHttpClient = new STHttpClient();
	iSemaphore.UnlockContext();

	STInt nOffset = iHttpCacheFile->CachedSize();
	STInt nConnectErr = iHttpClient->Connect(iUrl, nOffset);
	STLOGE("nConnectErr:%d", nConnectErr);
	STLOGE("iHttpClientLength:%d", iHttpClient->ContentLength());
	if (nConnectErr == STKErrOverflow)
	{
		iHttpCacheFile->SetTotalSize(iHttpCacheFile->CachedSize());
		iSemaphore.LockContext();
		iReadStatus = ETTReadStatusCompleted;
		iSemaphore.Signal();
		iSemaphore.UnlockContext();
	}
	else if (nConnectErr != STKErrNone)
	{
		iSemaphore.LockContext();
		iReadStatus = ETTReadStatusFailed;
		iSemaphore.Signal();
		iSemaphore.UnlockContext();
	}
	else
	{
		iHttpCacheFile->SetTotalSize(nOffset + iHttpClient->ContentLength());
		iSemaphore.LockContext();
		iReadStatus = ETTReadStatusReading;
		iSemaphore.UnlockContext();

		STChar* pBuffer = new STChar[KBUFFER_SIZE];
		STBool bRunning = ESTTrue;
		while(bRunning)
		{
			STInt nReadSize = iHttpClient->Read(pBuffer, KBUFFER_SIZE);
			if (nReadSize < 0)
			{
				iHttpClient->Disconnect();
				iSemaphore.LockContext();
				iReadStatus = ETTReadStatusFailed;
				iSemaphore.UnlockContext();
				bRunning = ESTFalse;
			}
			else if (nReadSize > 0)
			{
				STInt nWriteSize = iHttpCacheFile->Write(pBuffer, nReadSize);
				if (nWriteSize != nReadSize)
				{
					iSemaphore.LockContext();
					iReadStatus = ETTReadStatusFailed;
					iSemaphore.UnlockContext();
					bRunning = ESTFalse;
				}

			}
			else if (iHttpCacheFile->TotalSize() <= iHttpCacheFile->CachedSize())
			{
				iHttpClient->Disconnect();
				iSemaphore.LockContext();
				iReadStatus = ETTReadStatusCompleted;
				iSemaphore.UnlockContext();
				bRunning = ESTFalse;
			}

			iSemaphore.LockContext();
			iSemaphore.Signal();
			iSemaphore.UnlockContext();
		}
		delete[] pBuffer;
	}

	iSemaphore.LockContext();
	SAFE_DELETE(iHttpClient);
	iSemaphore.UnlockContext();
}

ISTDataReaderItf::DataReaderId STHttpReader::Id()
{
	return ISTDataReaderItf::EDataReaderIdHttp;
}
