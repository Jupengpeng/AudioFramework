#include "TTSysTime.h"
#include "TTIOClient.h"
#include <string.h>

extern TTBool  gUseProxy;

static const unsigned int KWaitTimeMs = 100; 
static const unsigned int KReconnectCount = 3; 


CTTIOClient::CTTIOClient(ITTStreamBufferingObserver* aObserver)
:mObserver(aObserver)
,mIOType(0)
,mHttpClient(NULL)
,mFile(NULL)
,mStatus(DisConnected)
,mUrl(NULL)
,mOffset(0)
,mCancel(0)
,mContentLength(0)
,mZeroNum(0)
,mReconnectNum(0)
,mStatusCode(0)
,mHostIP(0)
,mTotalTime(0)
,mTotalSize(0)
{
	mHttpClient = new CTTHttpClient();

	mSemaphore.Create();
	mCritical.Create();
}

CTTIOClient::~CTTIOClient()
{
	Close();
	SAFE_DELETE(mHttpClient);
	mSemaphore.Destroy();
	mCritical.Destroy();
}

TTInt  CTTIOClient::Open(const TTChar* aUrl, TTInt aOffset)
{
	TTCAutoLock Lock(&mCritical);
	if(aUrl == NULL) {
		return TTKErrArgument;
	}

	Close();

	mSemaphore.Reset();
	
	mStatus = ConnectInit;
	mUrl = (TTChar*) malloc((strlen(aUrl) + 1) * sizeof(TTChar));
	strcpy(mUrl, aUrl);

	updateSource();

	TTInt nErr = 0;
	TTInt tryCnt = 0;
	mStatusCode = 0;
	mHostIP = 0;
   
	mStatus = Connecting;
	if(mIOType == 1) {
		mFile = fopen(mUrl, "rb");

		nErr = ((mFile != NULL) && ((TTKErrNone == fseek(mFile, 0, SEEK_END)) && ((mContentLength = TTInt(ftell(mFile))) != TTKErrNotFound))) ? TTKErrNone : TTKErrAccessDenied;
	}else if(mIOType == 2) {
reconnect:
		nErr = gUseProxy ?
			mHttpClient->ConnectViaProxy(mObserver, mUrl, aOffset) : 
			mHttpClient->Connect(mObserver, mUrl, aOffset);

		mStatusCode = mHttpClient->StatusCode();
		mHostIP = mHttpClient->HostIP();

		if (nErr != TTKErrNone && !mCancel && tryCnt <= KReconnectCount) {
			mHttpClient->Disconnect();
			tryCnt++;			
			mSemaphore.Wait(KWaitTimeMs*5);
			if(!mCancel) {
				goto reconnect;
			}
		}

		if(nErr == TTKErrNone) {
			mContentLength = mHttpClient->ContentLength();
		}
	}

	if(nErr == TTKErrNone) {
		mStatus = Connected;
		return mContentLength;
	} else {		
		mStatus = ErrNotConnect;
		return nErr;
	}

	return nErr;
}

TTInt  CTTIOClient::Close()
{
	Cancel();

	TTCAutoLock Lock(&mCritical);
	SAFE_FREE(mUrl);
	if(mFile) {
		fclose(mFile);
		mFile = NULL;
	}

	mStatus = DisConnecting;
	if(mHttpClient) {
		mHttpClient->Disconnect();
	}
	mStatus = DisConnected;
	mContentLength = 0;
	mReconnectNum = 0;
	mOffset = 0;
	mIOType = 0;
	mCancel = 0;
	mZeroNum = 0;

	return 0;
}
	
TTInt  CTTIOClient::Read(TTChar* aDstBuffer, TTInt aSize, TTInt aOffset)
{
	TTCAutoLock Lock(&mCritical);
	TTInt nReadSize = 0;	
	
	if(mStatus != Connected && mStatus != DownLoading) {
		return TTKErrAccessDenied;
	}
	
	TTInt64 nStartTime = GetTimeOfDay();

	if(mIOType == 1) {
		if(aOffset >= 0) {
			if (TTKErrNone != fseek(mFile, aOffset, SEEK_SET))  {
				return TTKErrAccessDenied;
			}

			mOffset = aOffset;
		}
		nReadSize = (TTInt)fread((void*)aDstBuffer, 1, aSize, mFile);
		mOffset += nReadSize;
	} else {
		if(aOffset >= 0 || mZeroNum > 20) {
			TTInt nOffset = mOffset;
			if(aOffset >= 0) {
				nOffset = mOffset;
			}

			if (TTKErrNone != ReOpen(nOffset))  {
				return TTKErrCouldNotConnect;
			}
			mOffset = nOffset;
		}

		nReadSize = mHttpClient->Read(aDstBuffer, aSize);
		if(nReadSize >= 0) {
			mOffset += nReadSize;
		}
	}

	TTInt64 nEndTime = GetTimeOfDay();

	if(nReadSize > 0) {
		mZeroNum = 0;
		updateBandWidth(nEndTime - nStartTime, nReadSize);
	} else if(nReadSize == 0){
		mZeroNum++;
		updateBandWidth(nEndTime - nStartTime, nReadSize);
	} else {
		updateBandWidth(nEndTime - nStartTime, 0);
	}

	return nReadSize;
}

TTBool CTTIOClient::IsTransferBlock()
{
	TTCAutoLock Lock(&mCritical);
	if(mIOType != 2) {
		return ETTFalse;
	}

	return mHttpClient->IsTtransferBlock();
}

TTInt CTTIOClient::RequireContentLength()
{
	TTCAutoLock Lock(&mCritical);
	if(mIOType != 2) {
		return -1;
	}

	return mHttpClient->RequireContentLength();
}

TTInt  CTTIOClient::GetBuffer(TTChar* aDstBuffer, TTInt aSize, TTInt aOffset)
{
	TTCAutoLock Lock(&mCritical);
	TTChar* pBuffer = aDstBuffer;
	TTInt  nSize = aSize;
	TTInt  nReadSize = 0;
	TTInt  nZeroNum = 0;

	TTInt64 nStartTime = GetTimeOfDay();
	if(mIOType == 1) {
		if(aOffset >= 0) {
			if (TTKErrNone != fseek(mFile, aOffset, SEEK_SET))  {
				return TTKErrAccessDenied;
			}

			mOffset = aOffset;
		}
		nReadSize = (TTInt)fread((void*)aDstBuffer, 1, aSize, mFile);
		mOffset += nReadSize;

		return nReadSize;
	} 

	TTInt nOffset = mOffset;
	if(aOffset >= 0) {
		nOffset = mOffset;

		if (TTKErrNone != ReOpen(nOffset))  {
			return TTKErrCouldNotConnect;
		}

		mOffset = nOffset;
	}	

	do {
		TTInt nDownLoadSize = nSize - nReadSize;
		if(nDownLoadSize > 4*1024) {
			nDownLoadSize = 4*1024;
		}
		TTInt nReaded = mHttpClient->Read(pBuffer + nReadSize, nDownLoadSize);
		if(nReaded == 0) {
			nZeroNum++;
			if(nZeroNum > 100) {
				nReaded = -1;
			}
			continue;
		}  
		
		if(nReaded < 0) {
			TTInt nErr = TTKErrCouldNotConnect;
			for(TTInt n = 0; n < 10; n++) {
				nErr = ReOpen(mOffset);
				if(nErr == TTKErrNone) {
					break;
				}			
			}

			if(nErr < 0) {
				return nErr;
			}
		} else {
			nReadSize += nReaded;
			mOffset += nReaded;
			nZeroNum = 0;
		}
	}while(nReadSize < nSize || mCancel);

	TTInt64 nEndTime = GetTimeOfDay();
	updateBandWidth(nEndTime - nStartTime, nReadSize);

	return nReadSize;
}

TTInt  CTTIOClient::ReOpen(TTInt aOffset)
{
	TTCAutoLock Lock(&mCritical);
	if(mIOType != 2) {
		return 0;
	}

	TTInt nConnectErr = TTKErrNone;
	TTInt nConnectErrorCnt = 0;

	if(mReconnectNum > 30) {
		mStatus = DisConnected;

		char *pParam3 = NULL;
		pParam3 = inet_ntoa(*(struct in_addr*)&mHostIP);
		mObserver->DownLoadException(TTKErrDisconnected, mStatusCode, pParam3);
		return TTKErrAccessDenied;
	}

	TTInt64 nStartTime = GetTimeOfDay();
    do 
    {
        mHttpClient->Disconnect();
		nConnectErr = gUseProxy ? 
			mHttpClient->ConnectViaProxy(NULL, mUrl, aOffset) : 
			mHttpClient->Connect(NULL, mUrl, aOffset);

		mStatusCode = mHttpClient->StatusCode();
		mHostIP = mHttpClient->HostIP();

		if (nConnectErr == TTKErrNone || mCancel){
			break;
		}
        nConnectErrorCnt++;
		mSemaphore.Wait(KWaitTimeMs*2);
    } while(nConnectErrorCnt <= KReconnectCount);

	if(nConnectErr == TTKErrNone) {
		mReconnectNum = 0;
	} else {
		mReconnectNum++;    
	}

	TTInt64 nEndTime = GetTimeOfDay();

	updateBandWidth(nEndTime - nStartTime, 0);

    return nConnectErr;
}

void  CTTIOClient::Cancel()
{
	mSemaphore.Signal();
	if(mCancel == 0) {
		mCancel = 1;
		if(mHttpClient) {
			mHttpClient->Interrupt();
		}
	}
}

TTInt  CTTIOClient::IsEnd()
{
	return mContentLength == 0 ? mContentLength : mContentLength == mOffset; 
}

void CTTIOClient::updateSource()
{
#ifndef __TT_OS_WINDOWS__
	if(strncasecmp("http://", mUrl, 7) == 0) {
		mIOType = 2;
	} else {
		mIOType = 1;
	}
#else
	if(strnicmp("http://", mUrl, 7) == 0) {
		mIOType = 2;
	} else {
		mIOType = 1;
	}
#endif
}

void CTTIOClient::updateBandWidth(TTInt64 nTimeUs, TTInt nSize)
{
    BandwidthEntry entry;
    entry.mTimeUs = nTimeUs;
    entry.mNumBytes = nSize;
    mTotalTime += nTimeUs;
    mTotalSize += nSize;

    mBandwidthList.push_back(entry);
    if (mBandwidthList.size() > 100) {
        BandwidthEntry *entry = &*mBandwidthList.begin();
        mTotalTime -= entry->mTimeUs;
        mTotalSize -= entry->mNumBytes;
        mBandwidthList.erase(mBandwidthList.begin());
    }
}

TTInt CTTIOClient::GetBandWidth()
{
	if(mBandwidthList.size() < 2 || mTotalTime == 0) {
		return 0;
	}
	
	TTInt nBandWidth = mTotalSize*1000/mTotalTime;

	return nBandWidth;
}

TTChar* CTTIOClient::GetActualUrl()
{
	TTCAutoLock Lock(&mCritical);
	if(mIOType != 2) {
		return mUrl;
	}

	char* url = mHttpClient->GetRedirectUrl();
	if(url != NULL) {
		return url;
	}

	return mUrl;
}

TTInt CTTIOClient::GetStatusCode()
{
	return mStatusCode;
}

TTUint CTTIOClient::GetHostIP()
{
	return mHostIP;
}