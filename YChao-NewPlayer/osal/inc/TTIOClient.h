#ifndef __TT_IO_CLIENT_H__
#define __TT_IO_CLIENT_H__

#include <pthread.h>
#include "TTHttpClient.h"
#include "TTFileReader.h"
#include "TTSemaphore.h"
#include "TTCritical.h"
#include "TTStreamBufferingItf.h"
#include "TTList.h"

enum TTIOStatus
{
	ConnectInit = 0
	, Connecting = 1
	, Connected = 2
	, DownLoading = 3
	, DisConnecting = 4
	, DisConnected = 5

	
	, ErrNotConnect = 10
	, ErrDisConnect = 11
};

class CTTIOClient
{
public:
	CTTIOClient(ITTStreamBufferingObserver* aObserver);
	~CTTIOClient();

	TTInt							Open(const TTChar* aUrl, TTInt aOffset = 0);
	TTInt							Close();
	TTInt							IsEnd();
	TTInt							Read(TTChar* aDstBuffer, TTInt aSize, TTInt aOffset = -1);
	TTInt							GetBuffer(TTChar* aDstBuffer, TTInt aSize, TTInt aOffset = -1);
	TTInt							ReOpen(TTInt aOffset = 0);
	INLINE_C TTInt					ContentLength() { return mContentLength;};	
	TTIOStatus						GetStatus() {return mStatus;};
	TTInt							GetOffset() {return mOffset;};
	char*							GetURL() {return mUrl;};
	void							Cancel();
	TTInt							GetBandWidth();
	TTInt							GetStatusCode();
	TTUint							GetHostIP();

	TTBool							IsTransferBlock();

	TTInt							RequireContentLength();

	TTChar*							GetActualUrl();

private:	
	void							updateSource();
	void							updateBandWidth(TTInt64 nTimeUs, TTInt nSize);

private:
	ITTStreamBufferingObserver*	mObserver;
	TTInt			mIOType;	//0 unknown, 1, local file 2, http

	RTTSemaphore	mSemaphore;
	RTTCritical		mCritical;

	CTTHttpClient*	mHttpClient;
	FILE*			mFile;

	TTIOStatus		mStatus;
	TTChar*			mUrl;
	TTInt			mOffset;
	TTInt			mCancel;
	TTInt			mContentLength;
	TTInt			mZeroNum;
	TTInt			mReconnectNum;
	TTInt			mStatusCode;
	TTUint			mHostIP;

	struct BandwidthEntry {
        TTInt64 mTimeUs;
        TTInt   mNumBytes;
    };

	List<BandwidthEntry> mBandwidthList;

	TTInt64			mTotalTime;
	TTInt64			mTotalSize;

};

#endif
