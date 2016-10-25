#include "TTOsalConfig.h"
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#ifndef __TT_OS_WINDOWS__
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#endif
#include "TTUrlParser.h"
#include "TTBufferReaderProxy.h"
#include "TTHttpClient.h"
#include "TTCacheBuffer.h"
#include "TTNetWorkConfig.h"
#include "TTLog.h"
#include "TTTypedef.h"
#include "TTSysTime.h"
//#include "TTCommandId.h"
#include "TTSleep.h"

static const TTChar* KBufDownloadThreadName = "BufDownloadThread";
static const TTInt   BUF_PRE_FETCH_SIZE = 10*KILO;

extern TTBool  gUseProxy;

CTTBufferReaderProxy::CTTBufferReaderProxy()
:iUrl(NULL)
,iHttpClient(NULL)
,iCacheBuffer(NULL)
,iReadStatus(ETTReadStatusNotReady)
,iBufferStatus(ETTBufferingInValid)
,iPrefetchSize(BUF_PRE_FETCH_SIZE)
,iStreamBufferingObserver(NULL)
,iCurrentReadDesiredPos(0)
,iCurrentReadDesiredSize(0)
,iBandWidthTimeStart(0)
,iBandWidthSize(0)
,iBandWidthData(0)
,iStartBuffering(1)
,iFastDownLoad(1)
,iNetWorkChanged(false)
,iNetUseProxy(gUseProxy)
,iProxySize(0)
,iStatusCode(0)
,iHostIP(0)
,iOffSet(0)
,iNewOffSet(0)
,iCacheUrl(NULL)
,iRevCount(0)
,iAudioBitrate(0)
,iVideoBitrate(0)
{
	memset(iRevTime, 0, sizeof(TTInt)*MAX_RECEIVED_COUNT);
	memset(iRevSize, 0, sizeof(TTInt)*MAX_RECEIVED_COUNT);	
	iCritical.Create();	
	iSemaphore.Create();
}

CTTBufferReaderProxy::~CTTBufferReaderProxy()
{
	Close();
	iSemaphore.Destroy();
	iCritical.Destroy();
}

TTChar* CTTBufferReaderProxy::Url()
{
	return iUrl;
}

TTInt CTTBufferReaderProxy::Open(const TTChar* aUrl)
{
	TTASSERT(TTNetWorkConfig::getInstance() != NULL);
	TTASSERT(TTNetWorkConfig::getInstance()->getActiveNetWorkType() != EActiveNetWorkNone);
	TTASSERT(iHttpClient == NULL);
    iCurrentReadDesiredPos = 0;
	iCurrentReadDesiredSize = 0;
	iStartBuffering = 1;
	iCancel = ETTFalse;
	iOffSet = 0;
	iNewOffSet = 0;
	iProxySize = 0;
	iStatusCode = 0;
	iHostIP = 0;
	iBufferStatus = ETTBufferingInValid;
	iSemaphore.Reset();

	iAudioBitrate = 0;
	iVideoBitrate = 0;
	iBandWidthData = 0;
	LOGI("CTTBufferReaderProxy::Open %s, gUseProxy %d", aUrl, gUseProxy);
	
	SAFE_FREE(iUrl);
	iUrl = (TTChar*) malloc(strlen(aUrl) + 1);
	strcpy(iUrl, aUrl);

	TTInt nOffset = 0;
	TTInt nErr = TTKErrNone;

    TTInt tryCnt = 0;
    TTInt nConnectErr;
	iHttpClient = new CTTHttpClient();
    
 reconnect:
	nConnectErr = gUseProxy ?
			iHttpClient->ConnectViaProxy(iStreamBufferingObserver, iUrl) : 
			iHttpClient->Connect(iStreamBufferingObserver, iUrl);

	iHostIP = iHttpClient->HostIP();
	iStatusCode = iHttpClient->StatusCode();

    if (nConnectErr != TTKErrNone && !iCancel && tryCnt <= 3) {
        iHttpClient->Disconnect();
		tryCnt++;
        iSemaphore.Wait(KWaitIntervalMs*5);
		if(!iCancel) {
			goto reconnect;
		}
    }

	if (nConnectErr != TTKErrNone) 
	{
		SAFE_DELETE(iHttpClient);
		SAFE_FREE(iUrl);
		return nConnectErr;
	}

	iCacheBuffer = new CTTCacheBuffer();

	iCacheBuffer->Open();

	iCacheBuffer->SetTotalSize(iHttpClient->ContentLength());

	iReadStatus = ETTReadStatusReading;

	LOGE("CTTBufferReaderProxy::Open and begin to create thread.");
	
	if ((TTKErrNone	!= nErr) || (TTKErrNone != (nErr = iThreadHandle.Create(KBufDownloadThreadName, DownloadThreadProc, this, 0))))
	{
		iReadStatus = ETTReadStatusNotReady;
	}
	else
	{
		//if (iStreamBufferingObserver != NULL) {
		//	iStreamBufferingObserver->BufferingStart(TTKErrNeedWait, iHttpClient->StatusCode(), iHttpClient->HostIP());
		//}

		//iCritical.Lock();
		//iBufferStatus = ETTBufferingStart;
		//iCritical.UnLock();

		iPrefetchSize = BUF_PRE_FETCH_SIZE;
		ProcessBufferingIssue(0, iPrefetchSize);
	}

	if (TTKErrNone != nErr) 
	{
		SAFE_DELETE(iHttpClient);
		SAFE_DELETE(iCacheBuffer);
		SAFE_FREE(iUrl);
	}

	LOGI("CTTBufferReaderProxy::Open return: %d", nErr);
	return nErr;	
}

TTInt CTTBufferReaderProxy::Close()
{
	iCritical.Lock();
	iReadStatus = ETTReadStatusToStopRead;
	iCritical.UnLock();
	LOGI("CTTBufferReaderProxy Close. close %s", KBufDownloadThreadName);

	iThreadHandle.Close();

	LOGI("+++iHttpClient deleted");
	SAFE_DELETE(iHttpClient);
	LOGI("---iHttpClient deleted");

	LOGI("+++iCacheBuffer deleted");
	SAFE_DELETE(iCacheBuffer);
	LOGI("---iCacheBuffer deleted");
	SAFE_FREE(iUrl);
	SAFE_FREE(iCacheUrl);

	return TTKErrNone;
}

void CTTBufferReaderProxy::Cancel()
{
	iSemaphore.Signal();
	if(!iCancel) {
		iCancel = ETTTrue;
		if (iHttpClient != NULL)
		{
			iHttpClient->Interrupt();
		}
	}
}

TTBool CTTBufferReaderProxy::ProcessBufferingIssue(TTInt aDesireBufferedPos, TTInt aDesireBufferedSize)
{
	TTInt nWaitCount = 0;
	TTBool bIsBuffering = IsDesiredDataBuffering(aDesireBufferedPos, aDesireBufferedSize);

	while (bIsBuffering && nWaitCount++ < KWaitTimes && !iCancel)
	{
		iSemaphore.Wait(KWaitIntervalMs);
		bIsBuffering = IsDesiredDataBuffering(aDesireBufferedPos, aDesireBufferedSize);
	}

	return bIsBuffering;
}

TTBool CTTBufferReaderProxy::IsDesiredNewRequire(TTInt aDesiredPos, TTInt aDesiredSize, TTInt aLevel)
{
	TTBool bIsNewOffset = false;
	TTInt64 nBufStart = 0;
	TTInt64 nBufEnd = 0;
	TTInt64 nCachePos = 0;
	TTInt nOffset = BUF_PRE_FETCH_SIZE;
	TTInt nBufferCount = iCacheBuffer->TotalCount();
	TTInt nErr = iCacheBuffer->CachePoistion(nBufStart, nBufEnd);

	if(aLevel == 0 || aLevel == 1) {
		nOffset = (9*aLevel + 1)*KBUFFER_SIZE;

		nCachePos = nBufEnd + nOffset;
		if (nCachePos > iCacheBuffer->TotalSize()) {
			nCachePos = iCacheBuffer->TotalSize();
		}

		iCritical.Lock();
		if(aDesiredPos < nBufStart || aDesiredPos > nCachePos) {
			iNewOffSet = aDesiredPos;
			bIsNewOffset = true;
		}
		iCritical.UnLock();
	} else {
		nOffset = (iAudioBitrate + iVideoBitrate)*3;

		if(nOffset < BUF_PRE_FETCH_SIZE)
			nOffset = BUF_PRE_FETCH_SIZE;

		TTInt64 nCacheLeft = (nBufferCount - 2)*PAGE_BUFFER_SIZE - (nBufEnd - nBufStart);
		if(nCacheLeft > 0) {
			if(nOffset > nCacheLeft)
				nOffset = nCacheLeft;		
		}
		
		nCachePos = nBufEnd + nOffset;
		if (nCachePos > iCacheBuffer->TotalSize()) {
			nCachePos = iCacheBuffer->TotalSize();
		}

		iCritical.Lock();
		if((aDesiredPos < nBufStart && (!iNewOffSet)) || aDesiredPos > nCachePos || nCacheLeft <= 0) {
			iNewOffSet = aDesiredPos;
			bIsNewOffset = true;
		}
		iCritical.UnLock();
	}

	return bIsNewOffset;
}

TTBool CTTBufferReaderProxy::IsDesiredDataBuffering(TTInt aDesiredPos, TTInt aDesiredSize)
{
	TTInt nDesiredPos = aDesiredPos + aDesiredSize;
	if (nDesiredPos > iCacheBuffer->TotalSize())
	{
		nDesiredPos = iCacheBuffer->TotalSize();
	}

	TTBool bIsBuffer = true; 
	TTInt64 nBufStart = 0;
	TTInt64 nBufEnd = 0;

	TTInt nErr = iCacheBuffer->CachePoistion(nBufStart, nBufEnd);

	iCritical.Lock();
	if(aDesiredPos >= nBufStart && nDesiredPos <= nBufEnd)
		bIsBuffer = false;
	bIsBuffer = ((iReadStatus == ETTReadStatusReading) && bIsBuffer);
	iCritical.UnLock();

	return bIsBuffer;
}

TTBool CTTBufferReaderProxy::IsBuffering()
{
    TTInt nPrefetchSize = BUF_PRE_FETCH_SIZE*20;

	TTInt nBitrate = (iAudioBitrate + iVideoBitrate);
	TTInt nCount = iCacheBuffer->TotalCount();

    iCritical.Lock();
	if(iStartBuffering)
		nBitrate >>= 1;

	if(nBitrate > nPrefetchSize)
		nPrefetchSize = nBitrate;

	if(nPrefetchSize >= (nCount - 2)*PAGE_BUFFER_SIZE)
	{
		nPrefetchSize = (nCount - 2)*PAGE_BUFFER_SIZE;
	}

    TTInt nDesiredPos = iCurrentReadDesiredPos + nPrefetchSize;
	if (nDesiredPos > iCacheBuffer->TotalSize())
	{
		nDesiredPos = iCacheBuffer->TotalSize();
	}

	TTInt64 nBufStart = 0;
	TTInt64 nBufEnd = 0;
	TTBool bIsBuffer = true;

	TTInt nErr = iCacheBuffer->CachePoistion(nBufStart, nBufEnd);

	if(iCurrentReadDesiredPos >= nBufStart && nDesiredPos <= nBufEnd)
		bIsBuffer = false;

	bIsBuffer = ((iReadStatus == ETTReadStatusReading) && bIsBuffer);
    
    iCritical.UnLock();

    return bIsBuffer;
}

void CTTBufferReaderProxy::CheckBufferingDone()
{
    iCritical.Lock();
	TTBool bIsBufferStart = (iBufferStatus == ETTBufferingStart);
	iCritical.UnLock();
    
	if (bIsBufferStart && !IsBuffering())
	{
		if(iStreamBufferingObserver)
			iStreamBufferingObserver->BufferingDone();
        
		iCritical.Lock();
        iBufferStatus = ETTBufferingDone;
		if(iStartBuffering)
			iStartBuffering = 0;
        iCritical.UnLock();
	}
}

void  CTTBufferReaderProxy::CheckOnLineBuffering()
{
	iCritical.Lock();
	TTBufferingStatus eBufferStatus = iBufferStatus;
	TTReadStatus	  eReadStatus = iReadStatus;
	iCritical.UnLock();

	if (eBufferStatus != ETTBufferingStart && eReadStatus == ETTReadStatusReading)
	{
		if (iStreamBufferingObserver != NULL)
		{
			TTInt nErr = TTKErrNotReady;
			if(iHttpClient->HttpStatus() != 2) {
				nErr = TTKErrCouldNotConnect;
			}
			iStreamBufferingObserver->BufferingStart(nErr, iHttpClient->StatusCode(), iHttpClient->HostIP());
		}

		iCritical.Lock();
		iBufferStatus = ETTBufferingStart;
		iCritical.UnLock();
	}
}

void  CTTBufferReaderProxy::SetBitrate(TTInt aMediaType, TTInt aBitrate)
{
	if(aMediaType == 1) {
		iAudioBitrate = aBitrate;
	}else if(aMediaType == 2) {
		iVideoBitrate = aBitrate;
	}	
}


void  CTTBufferReaderProxy::SetNetWorkProxy(TTBool aNetWorkProxy)
{
	iCritical.Lock();	
	LOGI("CTTBufferReaderProxy aNetWorkProxy %d, iNetUseProxy %d, iNetWorkChanged %d, gUseProxy %d", aNetWorkProxy, iNetUseProxy, iNetWorkChanged, gUseProxy);

	if(aNetWorkProxy != iNetUseProxy) {
		iNetWorkChanged = true;
		iNetUseProxy = aNetWorkProxy;
	}
	iCritical.UnLock();
}

TTInt CTTBufferReaderProxy::Read(TTUint8* aReadBuffer, TTInt aReadPos, TTInt aReadSize)
{
	TTASSERT(iCacheBuffer != NULL);
	TTInt nReadSize = iCacheBuffer->Read(aReadBuffer, aReadPos, aReadSize);
    
	if(nReadSize != aReadSize) {
		iCritical.Lock();
		//iCurrentReadDesiredPos = aReadPos;
		//iCurrentReadDesiredSize = aReadSize;
		TTBufferingStatus eBufferStatus = iBufferStatus;
		TTReadStatus	  eReadStatus = iReadStatus;
		iCritical.UnLock();

		if (eBufferStatus != ETTBufferingStart && eReadStatus == ETTReadStatusReading)
		{
			if (iStreamBufferingObserver != NULL)
			{
				TTInt nErr = TTKErrNotReady;
				if(iHttpClient->HttpStatus() != 2) {
					nErr = TTKErrCouldNotConnect;
				}
				iStreamBufferingObserver->BufferingStart(nErr, iHttpClient->StatusCode(), iHttpClient->HostIP());
			}

			iCritical.Lock();
			iBufferStatus = ETTBufferingStart;
			iCurrentReadDesiredPos = aReadPos;
			iCurrentReadDesiredSize = aReadSize;
			iCritical.UnLock();

			IsDesiredNewRequire(aReadPos, aReadSize, 2);
		}		

		//TTBool bIsBuffering = ProcessBufferingIssue(aReadPos, aReadSize);
		//if(!bIsBuffering) {
		//	nReadSize = iCacheBuffer->Read(aReadBuffer, aReadPos, aReadSize);
		//}
	}

	if(nReadSize != aReadSize) {
		iCritical.Lock();
		if(iReadStatus != ETTReadStatusReading || iCancel == ETTTrue)
			nReadSize = TTKErrAccessDenied;
		iCritical.UnLock();		
	}

	return nReadSize;
}

TTInt CTTBufferReaderProxy::ReadWait(TTUint8* aReadBuffer, TTInt aReadPos, TTInt aReadSize)
{
	TTASSERT(iCacheBuffer != NULL);
	TTInt nReadSize = iCacheBuffer->Read(aReadBuffer, aReadPos, aReadSize);
    
	if(nReadSize != aReadSize) {
		iCritical.Lock();
		iCurrentReadDesiredPos = aReadPos;
		TTBufferingStatus eBufferStatus = iBufferStatus;
		TTReadStatus	  eReadStatus = iReadStatus;
		iCritical.UnLock();

		if (eReadStatus == ETTReadStatusReading)
		{
			IsDesiredNewRequire(aReadPos, aReadSize, 1);
		}		

		TTBool bIsBuffering = ProcessBufferingIssue(aReadPos, aReadSize);
		if(!bIsBuffering) {
			nReadSize = iCacheBuffer->Read(aReadBuffer, aReadPos, aReadSize);
		}
	}

	if(nReadSize != aReadSize) {
		iCritical.Lock();
		if(iReadStatus != ETTReadStatusReading || iCancel == ETTTrue)
			nReadSize = TTKErrAccessDenied;
		iCritical.UnLock();		
	}

	return nReadSize;
}

TTInt CTTBufferReaderProxy::Size() const
{
	TTInt nSize = 0;
	if(iCacheBuffer) {
		nSize = iCacheBuffer->TotalSize();
	}

	return nSize;	
}

TTInt CTTBufferReaderProxy::ReConnectServer(TTInt aOffset)
{
    TTInt nConnectErr = TTKErrNone;
	TTInt nConnectErrorCnt = 0;

    do 
    {
        iHttpClient->Disconnect();
		nConnectErr = gUseProxy ? 
			iHttpClient->ConnectViaProxy(NULL, iUrl, aOffset) : 
			iHttpClient->Connect(NULL, iUrl, aOffset);

		iStatusCode = iHttpClient->StatusCode();
		iHostIP = iHttpClient->HostIP();

		if (nConnectErr == TTKErrNone || iCancel) 
		{
			break;
		}
        nConnectErrorCnt++;
		iSemaphore.Wait(KWaitIntervalMs*5);
    } while(nConnectErrorCnt < MAX_RECONNECT_COUNT);
    
    return nConnectErr;
}

TTInt CTTBufferReaderProxy::BufferedSize()
{
	TTInt64 nBufStart = 0;
	TTInt64 nBufEnd = 0;

	if(iCacheBuffer) {
		iCacheBuffer->CachePoistion(nBufStart, nBufEnd);
	}

	return nBufEnd;
}

TTUint CTTBufferReaderProxy::ProxySize()
{
	return iProxySize;
}

void* CTTBufferReaderProxy::DownloadThreadProc(void* aPtr)
{
	CTTBufferReaderProxy* pReaderProxy = reinterpret_cast<CTTBufferReaderProxy*>(aPtr);
	pReaderProxy->DownloadThreadProcL(NULL);
	return NULL;
}

void CTTBufferReaderProxy::DownloadThreadProcL(void* aPtr)
{
	TTChar* pBuffer = new TTChar[KBUFFER_SIZE];
	if(iStreamBufferingObserver)
		iStreamBufferingObserver->PrefetchStart(iHttpClient->HostIP());

	iPrefetchSize = KILO*KILO;

#ifdef __TT_OS_ANDROID__
	nice(-1);
#endif

	TTInt nCount  = 0;
	TTInt nOffset = iOffSet;
	TTInt nNewOffset = iOffSet;
	TTInt  nSlowDown = 0;
	TTInt  nReconnectNum = 0;
	TTInt  nZeroBuffer = 0;
	TTInt  nFastDownLoad = 1;
	TTInt64	nNowTime = GetTimeOfDay();

	iBandWidthTimeStart = nNowTime;
	iBandWidthSize = 0;
	iBandWidthData = 0;

	TTInt  nEOS = 0;
	TTBool bNetWorkChanged;
	iDataReceiveStatus = ETTDataReceiveStatusPrefetchReceiving;
    LOGI("[timecost]: TT_Begin_Receive_Data...");
     
	while(!iThreadHandle.Terminating())
	{
		nOffset = iOffSet;
	
		iCritical.Lock();
		if (iReadStatus == ETTReadStatusToStopRead)
		{
			iReadStatus = ETTReadStatusNotReady;
			iCritical.UnLock();
			break;
		}
		nNewOffset = iNewOffSet;
		bNetWorkChanged = iNetWorkChanged;
		nFastDownLoad = iFastDownLoad;
		iCritical.UnLock();	

		if (iCancel)	
		{
			iCritical.Lock();
			iReadStatus = ETTReadStatusNotReady;
			iCritical.UnLock();
			break;
		}

		nNowTime = GetTimeOfDay();
		if(nNowTime >= iBandWidthTimeStart + 1000) {
			iBandWidthData = (iBandWidthData + iBandWidthSize*1000/(nNowTime - iBandWidthTimeStart)) >> 1;
			iBandWidthTimeStart = nNowTime;
			iBandWidthSize = 0;			
		}

		if(nNewOffset || bNetWorkChanged) 
		{
			LOGI("CTTBufferReaderProxy nNewOffset %d, iNetWorkChanged %d, gUseProxy %d", nNewOffset, iNetWorkChanged, gUseProxy);
			if(nNewOffset)
				nOffset = nNewOffset;
			TTInt nErr = ReConnectServer(nOffset);
			if (nErr == TTKErrNone)
			{
				iCritical.Lock();
				if(nNewOffset == iNewOffSet)
					iNewOffSet = 0;
				if(bNetWorkChanged)
					iNetWorkChanged = false;
				iCritical.UnLock();				
			}
			else
			{
				iCritical.Lock();
				TTBufferingStatus eBufferStatus = iBufferStatus;
				iCritical.UnLock();
				
				if(eBufferStatus == ETTBufferingStart)
					nReconnectNum++;
				if(nReconnectNum > 20) {
					TTInt errCode = TTKErrDisconnected;
					if (nErr == TTKErrServerTerminated)
						errCode = TTKErrServerTerminated;

					iCritical.Lock();
					iCancel = true;
					iReadStatus = ETTReadStatusNotReady;
					iCritical.UnLock();

					if(iStreamBufferingObserver) {
						char *pParam3 = NULL;
						pParam3 = inet_ntoa(*(struct in_addr*)&iHostIP);
						iStreamBufferingObserver->DownLoadException(errCode, iStatusCode, pParam3);
					}
					break;
				}
				continue;
			}

			nEOS = 0;
		}

		if(nEOS) {		
			iSemaphore.Wait(KWaitIntervalMs);
			continue;
		}

		if(nSlowDown) {
			iSemaphore.Wait(KWaitIntervalMs/2);
		}
		
		TTInt64 nBufStart = 0;
		TTInt64 nBufEnd = 0;
		TTInt  nDownLoadSize = KBUFFER_SIZE;
		TTInt nErr = iCacheBuffer->CachePoistion(nBufStart, nBufEnd);
		TTInt nBufferCount = iCacheBuffer->TotalCount();
		
		nSlowDown = 0;
		if(nBufEnd - nBufStart > (nBufferCount - 2)*PAGE_BUFFER_SIZE || nFastDownLoad == 0)
		{
			nDownLoadSize = 1;
			nSlowDown = 1;
		}

		if(nBufEnd - nBufStart > (nBufferCount - 1)*PAGE_BUFFER_SIZE -KBUFFER_SIZE) 
		{
			iSemaphore.Wait(KWaitIntervalMs);
			continue;
		}
		
		TTInt64 nStart = GetTimeOfDay();
		TTInt nReadSize = iHttpClient->Read(pBuffer, nDownLoadSize);
		TTInt64 nEnd = GetTimeOfDay();

		if(nReadSize > 0) {
			iBandWidthSize += nReadSize;
		}

		iCritical.Lock();
		if(!nSlowDown) {
			TTInt nRevCount = iRevCount%MAX_RECEIVED_COUNT;
			iRevTime[nRevCount] = nEnd - nStart;
			iRevSize[nRevCount] = nReadSize < 0 ? 0 : nReadSize;
			iRevCount++;
		}
		iCritical.UnLock();

        if (nReadSize == 0)
        {
			iOffSet = nOffset;
			nZeroBuffer++;
			iSemaphore.Wait(5);
			if(nZeroBuffer >= 1000) {
				nReadSize = -1;
			} else {
				continue;
			}
		}

		if (nReadSize < 0)
		{
			iRevCount = 0;

			if (ReConnectServer(nOffset) == TTKErrNone)
			{
				iOffSet = nOffset;
				continue;
			}
			//LOGI("-----------HttpReaderProxy Read Error: %d, nReconnectNum %d", nReadSize, nReconnectNum);

			iCritical.Lock();
			TTBufferingStatus eBufferStatus = iBufferStatus;
			iCritical.UnLock();

			if(eBufferStatus == ETTBufferingStart)
				nReconnectNum++;
			if(nReconnectNum > 20) {
				TTInt errCode = TTKErrDisconnected;
				if (nReadSize == TTKErrServerTerminated)
					errCode = TTKErrServerTerminated;

				iCritical.Lock();
				iCancel = true;
				iReadStatus = ETTReadStatusNotReady;
				iCritical.UnLock();

				if(iStreamBufferingObserver) {
					char *pParam3 = NULL;
					pParam3 = inet_ntoa(*(struct in_addr*)&iHostIP);
					iStreamBufferingObserver->DownLoadException(errCode, iStatusCode, pParam3);
				}

				break;
			}
			continue;
		}

		if(gUseProxy)
			iProxySize += nReadSize;

		//LOGI("nOffset %d, nReadSize %d", nOffset, nReadSize);

		nReconnectNum = 0;
		nZeroBuffer = 0;

        TTInt nWriteSize = iCacheBuffer->Write((TTUint8 *)pBuffer, nOffset, nReadSize);
		
		nCount++;
      
        if (iDataReceiveStatus == ETTDataReceiveStatusPrefetchReceiving && !IsDesiredDataBuffering(0, iPrefetchSize))
        {
            //send PrefetchCompleted msg
			if(iStreamBufferingObserver)
				iStreamBufferingObserver->PrefetchCompleted();

            iDataReceiveStatus = ETTDataReceiveStatusPrefetchReceived;
        }

		if (nWriteSize != nReadSize)
		{
			//Send exception msg
			if(iStreamBufferingObserver)
				iStreamBufferingObserver->DownLoadException(TTKErrWrite, 0, NULL);

			iCritical.Lock();
			iCancel = true;
			iReadStatus = ETTReadStatusNotReady;
			iCritical.UnLock();

			LOGE("Write HttpCacheFile Error");
			break;
		}
		else if (iCacheBuffer->TotalSize() <= nOffset + nReadSize)
		{
			LOGI("+++++++++++CTTBufferReaderProxy Read Compeleted+++++++++++");	
            SAFE_FREE(iCacheUrl);
            iCacheUrl = (TTChar*) malloc(strlen(iUrl) + 1);
            strcpy(iCacheUrl, iUrl);

			if(iStreamBufferingObserver)
				iStreamBufferingObserver->CacheCompleted(iCacheUrl);
            
            CheckBufferingDone();

			nEOS = 1;
		}

        CheckBufferingDone();

		iOffSet = nOffset + nReadSize;
	}

	CheckBufferingDone();

	delete[] pBuffer;
	iHttpClient->Disconnect();
}

void CTTBufferReaderProxy::SetStreamBufferingObserver(ITTStreamBufferingObserver* aObserver)
{
	//LOGI("CTTBufferReaderProxy::SetStreamBufferingObserver");
	iStreamBufferingObserver = aObserver;
	//LOGI("CTTBufferReaderProxy::SetStreamBufferingObserver return");
}

TTBool CTTBufferReaderProxy::ParseUrl(const TTChar* aUrl)
{
	TTChar pExtension[16];
	
	CTTUrlParser::ParseExtension(aUrl, pExtension);

	return strlen(pExtension) > 0;
}

void CTTBufferReaderProxy::SetDownSpeed(TTInt aFast)
{
	iCritical.Lock();
	iFastDownLoad = aFast;
	iCritical.UnLock();
}

TTInt CTTBufferReaderProxy::GetStatusCode()
{
	return iStatusCode;
}

TTUint CTTBufferReaderProxy::GetHostIP()
{
	return iHostIP;
}

TTUint CTTBufferReaderProxy::BandWidth()
{
	TTUint nBandWidth = 0;
	nBandWidth = iBandWidthData;
	if(nBandWidth < 1024)
		nBandWidth = 0;
	return nBandWidth;
}

TTUint CTTBufferReaderProxy::BandPercent()
{
	iCritical.Lock();
	TTBufferingStatus eBufferStatus = iBufferStatus;
	TTInt aDesireBufferedPos = iCurrentReadDesiredPos;
	TTInt nStartBuffering = iStartBuffering;
	iCritical.UnLock();

	if(eBufferStatus != ETTBufferingStart) {
		return 100;
	}

    TTInt nPrefetchSize = BUF_PRE_FETCH_SIZE*20;
	TTInt nBitrate = (iAudioBitrate + iVideoBitrate);

	if(nStartBuffering) {
		nBitrate >>= 1;
	}	

	TTInt nCount = iCacheBuffer->TotalCount();

	TTInt nPercent = 0;

	if(nBitrate > nPrefetchSize)
		nPrefetchSize = nBitrate;

 	if(nPrefetchSize >= (nCount - 2)*PAGE_BUFFER_SIZE)
	{
		nPrefetchSize = (nCount - 2)*PAGE_BUFFER_SIZE;
	}

    TTInt nDesiredPos = aDesireBufferedPos + nPrefetchSize;
	if (nDesiredPos > iCacheBuffer->TotalSize())
	{
		nPrefetchSize = iCacheBuffer->TotalSize() - aDesireBufferedPos;
	}

	TTInt64 nBufStart = 0;
	TTInt64 nBufEnd = 0;
	TTInt nErr = iCacheBuffer->CachePoistion(nBufStart, nBufEnd);

	if(nPrefetchSize)
		nPercent = (nBufEnd - aDesireBufferedPos)*100/nPrefetchSize;
	else
		nPercent = 100;

	if(nPercent < 0) {
		nPercent = 0;
	}

	if(nBufStart > aDesireBufferedPos) {
		nPercent = 0;
	}

	if(nPercent > 100) {
		nPercent = 100;
	}

    return nPercent;
}

TTInt CTTBufferReaderProxy::PrepareCache(TTInt aDesireBufferedPos, TTInt aDesireBufferedSize, TTInt aFlag)
{
	TTInt nWaitCount = 0;
	TTBool bIsBuffering = IsDesiredDataBuffering(aDesireBufferedPos, aDesireBufferedSize);

	if(iReadStatus != ETTReadStatusReading && iCancel)
		return TTKErrUnderflow;

	iCritical.Lock();	
	TTBufferingStatus eBufferStatus = iBufferStatus;
	TTReadStatus	  eReadStatus = iReadStatus;
	iCritical.UnLock();

	if(eBufferStatus == ETTBufferingStart) {
		if(iStreamBufferingObserver)
			iStreamBufferingObserver->BufferingDone();

		iCritical.Lock();
		iBufferStatus = ETTBufferingDone;
		iCritical.UnLock();
	}

	if(aFlag&TTREADER_CACHE_ASYNC)
	{
		if (eBufferStatus == ETTBufferingInValid || bIsBuffering)	{
			if (iStreamBufferingObserver != NULL) {
				iStreamBufferingObserver->BufferingStart(TTKErrNeedWait, iHttpClient->StatusCode(), iHttpClient->HostIP());
			}

			iCritical.Lock();
			iStartBuffering = 1;
			iCurrentReadDesiredPos = aDesireBufferedPos;
			iBufferStatus = ETTBufferingStart;
			iCritical.UnLock();
		}
	}

	if(!bIsBuffering) {
		if(eBufferStatus == ETTBufferingStart) {
			if(iStreamBufferingObserver)
				iStreamBufferingObserver->BufferingDone();

			iCritical.Lock();
			iBufferStatus = ETTBufferingDone;
			iCritical.UnLock();
		}
		return TTKErrNone;
	}

	IsDesiredNewRequire(aDesireBufferedPos, aDesireBufferedSize, 0);

	if(aFlag&TTREADER_CACHE_SYNC) 
	{
		while (bIsBuffering && nWaitCount++ < KWaitTimes && !iCancel)
		{
			iSemaphore.Wait(KWaitIntervalMs);
			bIsBuffering = IsDesiredDataBuffering(aDesireBufferedPos, aDesireBufferedSize);
		}

		if(iReadStatus != ETTReadStatusReading && iCancel)
			return TTKErrUnderflow;

		if(bIsBuffering) {
			return TTKErrUnderflow;
		}
	} 

	return TTKErrNone;
}