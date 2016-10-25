#include "TTOsalConfig.h"


// INCLUDES
#include "Wins/TTWinAudioSink.h"
#include "TTSleep.h"
#include "TTSysTime.h"
#include "TTLog.h"

#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>

#pragma   comment(lib,"winmm.lib")

CTTWinAudioSink* pSink = NULL;
FILE* DumpFile = NULL;
extern TTInt gAudioEffectLowDelay;

/*
* some good values for block size and count
*/
#define BLOCK_SIZE  (KHwAudioBuffer/2)
#define BLOCK_COUNT 3
/*
* function prototypes
*/ 
static void CALLBACK waveOutProc(HWAVEOUT, UINT, DWORD, DWORD, DWORD);
static WAVEHDR* allocateBlocks(int size, int count);
static void freeBlocks(WAVEHDR* blockArray);
static TTInt writeAudio(HWAVEOUT hWaveOut, LPSTR data, int size);
/*
* module level variables
*/
static CRITICAL_SECTION waveCriticalSection;
static CRITICAL_SECTION waveOutCritical;
static WAVEHDR*     waveBlocks = NULL;
static volatile int   waveFreeBlockCount;
static int       waveCurrentBlock;
HWAVEOUT hWaveOut = NULL; /* device handle */
static int		 RenderNum = 0;
static int		 block_size = 0;

static int		 Start = 0;
static int		 End = 0;

static void CALLBACK waveOutProc(
								 HWAVEOUT hWaveOut, 
								 UINT uMsg, 
								 DWORD dwInstance, 
								 DWORD dwParam1,  
								 DWORD dwParam2   
								 )
{
	/*
	* pointer to free block counter
	*/
	int* freeBlockCounter = (int*)dwInstance;
	/*
	* ignore calls that occur due to openining and closing the
	* device.
	*/
	if(uMsg != WOM_DONE)
		return;
	EnterCriticalSection(&waveCriticalSection);
	(*freeBlockCounter)++;
	//TTPRINTF("EmptyData \n");
	if (pSink != NULL)
	{
		pSink->AudioDone((void *)dwParam1);
	}	

	LeaveCriticalSection(&waveCriticalSection);
}



WAVEHDR* allocateBlocks(int size, int count)
{
	char* buffer;
	int i;
	WAVEHDR* blocks;
	DWORD totalBufferSize = (size + sizeof(WAVEHDR)) * count;

	/*
	* allocate memory for the entire set in one go
	*/
	if((buffer = (char*)HeapAlloc(
		GetProcessHeap(), 
		HEAP_ZERO_MEMORY, 
		totalBufferSize
		)) == NULL) {
			fprintf(stderr, "Memory allocation error\n");
			ExitProcess(1);
	}
	/*
	* and set up the pointers to each bit
	*/
	blocks = (WAVEHDR*)buffer;
	buffer += sizeof(WAVEHDR) * count;
	for(i = 0; i < count; i++) {
		blocks[i].dwBufferLength = size;
		blocks[i].lpData = buffer;
		buffer += size;
	}

	return blocks;
}

void freeBlocks(WAVEHDR* blockArray)
{
	/* 
	* and this is why allocateBlocks works the way it does
	*/ 
	HeapFree(GetProcessHeap(), 0, blockArray);
}


TTInt writeAudio(HWAVEOUT hWaveOut, LPSTR data, int size)
{
	WAVEHDR* current;
	current = &waveBlocks[waveCurrentBlock];

	TTInt send = 0;

	while(size > 0) 
	{
		/* 
		* first make sure the header we're going to use is unprepared
		*/
		EnterCriticalSection(&waveOutCritical);
		if(current->dwFlags & WHDR_PREPARED) 
			waveOutUnprepareHeader(hWaveOut, current, sizeof(WAVEHDR));
		LeaveCriticalSection(&waveOutCritical);

		if(size < ((int)(block_size - current->dwUser)))
		{
			memcpy(current->lpData + current->dwUser, data, size);
#ifdef __DUMP_PCM__
			if(DumpFile)
				fwrite(data, size, 1, DumpFile);
#endif
			current->dwUser += size;
			break;
		}

		int remain = block_size - current->dwUser;
		memcpy(current->lpData + current->dwUser, data, remain);
#ifdef __DUMP_PCM__
		if(DumpFile)
			fwrite(data, remain, 1, DumpFile);
#endif
		size -= remain;
		data += remain;
		current->dwBufferLength = block_size;
		
		EnterCriticalSection(&waveOutCritical);
		waveOutPrepareHeader(hWaveOut, current, sizeof(WAVEHDR));
		waveOutWrite(hWaveOut, current, sizeof(WAVEHDR));
		RenderNum++;
		LeaveCriticalSection(&waveOutCritical);

		send++;

		EnterCriticalSection(&waveCriticalSection);
		waveFreeBlockCount--;
		LeaveCriticalSection(&waveCriticalSection);

		/*
		* wait for a block to become free
		*/
		while(!waveFreeBlockCount)
		 	TTSleep(10);
		/*
		* point to the next block
		*/
		waveCurrentBlock++;
		waveCurrentBlock %= BLOCK_COUNT;
		current = &waveBlocks[waveCurrentBlock];
		current->dwUser = 0;
	}

	if(send > 1)
	{
		int mm  = 0;
		mm++;
	}

	return send;
}

CTTWinAudioSink::CTTWinAudioSink(CTTSrcDemux* SrcMux, TTInt nCount)
: TTCBaseAudioSink(SrcMux, nCount)
{
	InitializeCriticalSection(&waveCriticalSection);
	InitializeCriticalSection(&waveOutCritical);

	mCritRender.Create();

	mLVolume = 65535 / 2;
	mRVolume = 65535 / 2;
}

CTTWinAudioSink::~CTTWinAudioSink()
{
	DeleteCriticalSection(&waveCriticalSection);
	DeleteCriticalSection(&waveOutCritical);

	mCritRender.Destroy();
}

TTInt CTTWinAudioSink::newAudioTrack()
{
	closeAudioTrack();

	WAVEFORMATEX wfx; /* look this up in your documentation */
	/*
	* set up the WAVEFORMATEX structure.
	*/
	wfx.nSamplesPerSec = mAudioFormat.SampleRate; /* sample rate */
	wfx.wBitsPerSample = mAudioFormat.SampleBits;   /* sample size */
	wfx.nChannels    = mAudioFormat.Channels;   /* channels  */
	wfx.cbSize     = 0;   /* size of _extra_ info */
	wfx.wFormatTag   = WAVE_FORMAT_PCM;
	wfx.nBlockAlign   = (wfx.wBitsPerSample * wfx.nChannels) >> 3;
	wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

	if(wfx.nSamplesPerSec > 24000) {
		block_size = wfx.nAvgBytesPerSec / 4;
	} else {
		block_size = wfx.nAvgBytesPerSec / 2;
	}
	block_size = (block_size + wfx.nBlockAlign - 1)/wfx.nBlockAlign*wfx.nBlockAlign;

	waveBlocks     = allocateBlocks(block_size, BLOCK_COUNT);
	waveFreeBlockCount = BLOCK_COUNT;
	waveCurrentBlock  = 0;

	mAvgBytesPerSec = wfx.nAvgBytesPerSec;
	/*
	* try to open the default wave device. WAVE_MAPPER is
	* a constant defined in mmsystem.h, it always points to the
	* default wave device on the system (some people have 2 or
	* more sound cards).
	*/
	EnterCriticalSection(&waveOutCritical);
	if(waveOutOpen(
		&hWaveOut, 
		WAVE_MAPPER, 
		&wfx, 
		(DWORD_PTR)waveOutProc, 
		(DWORD_PTR)&waveFreeBlockCount, 
		CALLBACK_FUNCTION
		) != MMSYSERR_NOERROR) {
			 			printf("unable to open wave mapper device\n");
			// 			ExitProcess(1);

			return TTKErrAccessDenied;
	}
	LeaveCriticalSection(&waveOutCritical);

	pSink = this;

	EnterCriticalSection(&waveOutCritical);
	waveOutSetVolume(hWaveOut,(mLVolume<<16 | mRVolume));
	LeaveCriticalSection(&waveOutCritical);

	mFrameDuration = block_size*1000;
	mFrameDuration = mFrameDuration/mAvgBytesPerSec;
	mAudioOffSetTime = mFrameDuration*(BLOCK_COUNT - 1);

	if(mProcessCount == 1) {
		mSinkBuf = (TTPBYTE)malloc(block_size);
		mSinkBufLen = block_size;
	}
	
#ifdef __DUMP_PCM__
	DumpFile = fopen("D:\\Test\\Dump.pcm", "wb");
#endif

	return TTKErrNone;
}

TTInt CTTWinAudioSink::stop()
{
	TTCAutoLock Lock(&mCritical);
	TTCBaseAudioSink::stop();
	EnterCriticalSection(&waveOutCritical);
	if(hWaveOut)
		waveOutReset(hWaveOut);
	LeaveCriticalSection(&waveOutCritical);

	return TTKErrNone;
}

TTInt CTTWinAudioSink::closeAudioTrack()
{
	EnterCriticalSection(&waveOutCritical);
	if(hWaveOut) {
		waveOutReset(hWaveOut);
		waveOutClose(hWaveOut);
		freeBlocks(waveBlocks);
	}
	hWaveOut = NULL;
	LeaveCriticalSection(&waveOutCritical);

	pSink = NULL;
	SAFE_FREE(mSinkBuf);


#ifdef __DUMP_PCM__
	if(DumpFile)
		fclose(DumpFile);
#endif

	return TTKErrNone;
}


TTInt CTTWinAudioSink::flush()
{
	TTCBaseAudioSink::flush();
	
	while(waveFreeBlockCount != BLOCK_COUNT)
		 	TTSleep(5);

	return TTKErrNone;
}

TTInt CTTWinAudioSink::syncPosition(TTUint64 aPosition, TTInt aOption)
{
	mCritRender.Lock();
	WAVEHDR* current;
	current = &waveBlocks[waveCurrentBlock];
	current->dwUser = 0;
	mSinkBuffer.nSize = 0;
	mCritRender.UnLock();

	return TTCBaseAudioSink::syncPosition(aPosition, aOption);
}


TTInt CTTWinAudioSink::render()
{
	TTInt nErr = TTKErrNone;

	//End = GetTimeOfDay();
	//LOGI("Using time: End - Start: %d, mRenderNum %d", (TTInt)(End - Start), mRenderNum);
	//Start = End;
	EnterCriticalSection(&waveCriticalSection);

	//TTPRINTF("Renderdata: %d\n", waveFreeBlockCount);
	if((waveFreeBlockCount > 0) )
	{
		LeaveCriticalSection(&waveCriticalSection);
		if(mAudioProcess == NULL)
			return TTKErrNotFound;

		if(mSinkBuffer.nSize > 0 && mSinkBuffer.pBuffer != NULL) {
			nErr = WriteData(&mSinkBuffer);
			startOne(-1);
			return nErr;
		}

		if(mProcessCount > 1) {
			mSinkBuffer.pBuffer = NULL; 
			mSinkBuffer.nSize = 0; 
		}else {
			mSinkBuffer.pBuffer = mSinkBuf;
			mSinkBuffer.nSize = mSinkBufLen;
		}
		mSinkBuffer.llTime = mCurPos;
		mSinkBuffer.nFlag = 0;
		nErr = mAudioProcess->getOutputBuffer(&mSinkBuffer);
		if (nErr == TTKErrNone)
		{
			if(mSinkBuffer.nFlag & TT_FLAG_BUFFER_EOS) {
				setEOS();
				nErr = TTKErrEof;
			} else if(mSinkBuffer.nFlag & TT_FLAG_BUFFER_NEW_FORMAT){
				audioFormatChanged();
			} else {

				if(mSinkBuffer.nFlag & TT_FLAG_BUFFER_TIMESTAMP_RESET)	{
					if(mObserver) {
						mObserver->pObserver(mObserver->pUserData, ENotifyTimeReset, (TTInt)mSinkBuffer.llTime, 0, NULL);
					}
				}

				if(mRenderNum == 0) {
					audioFormatChanged();
				}

				mCritTime.Lock();
				mCurPos = mSinkBuffer.llTime + mSinkBuffer.nSize*500/(mAudioFormat.Channels*mAudioFormat.SampleRate);
				mRenderPCM += mSinkBuffer.nSize;
				mCritTime.UnLock();

				if(ETTAudioFadeNone != getFadeStatus())
					fadeOutInHandle();

				WriteData(&mSinkBuffer);
			}
		} else if(nErr == TTKErrEof) {
			setEOS();
		}
		else if(nErr == TTKErrOverflow) {
			SAFE_FREE(mSinkBuf);
			mSinkBufLen = mSinkBuffer.nSize*3/2;			
			mSinkBuf = (TTPBYTE)malloc(mSinkBufLen);
		}  else if(nErr == TTKErrFormatChanged) {
			audioFormatChanged();
		} 
		if(nErr != TTKErrEof)
			startOne(-1);
	}
	else
	{
		LeaveCriticalSection(&waveCriticalSection);
	}

	return nErr;
}

TTInt CTTWinAudioSink::WriteData(TTBuffer* aBuffer)
{
	TTASSERT(aBuffer != NULL);

	TTCAutoLock Lock(&mCritRender);
	WAVEHDR* current;
	current = &waveBlocks[waveCurrentBlock];
	LPSTR data = (LPSTR)(aBuffer->pBuffer);
	int size = aBuffer->nSize;

	/* 
	* first make sure the header we're going to use is unprepared
	*/
	EnterCriticalSection(&waveOutCritical);
	if(current->dwFlags & WHDR_PREPARED) 
		waveOutUnprepareHeader(hWaveOut, current, sizeof(WAVEHDR));
	LeaveCriticalSection(&waveOutCritical);

	if(size < ((int)(block_size - current->dwUser)))
	{
		memcpy(current->lpData + current->dwUser, data, size);
#ifdef __DUMP_PCM__
		if(DumpFile)
			fwrite(data, size, 1, DumpFile);
#endif
		current->dwUser += size;
		aBuffer->pBuffer += size;
		aBuffer->nSize -= size;
		return TTKErrNone;
	}

	int remain = block_size - current->dwUser;
	memcpy(current->lpData + current->dwUser, data, remain);
#ifdef __DUMP_PCM__
	if(DumpFile)
		fwrite(data, remain, 1, DumpFile);
#endif
	size -= remain;
	data += remain;
	aBuffer->pBuffer += remain;
	aBuffer->nSize -= remain;

	current->dwBufferLength = block_size;

	EnterCriticalSection(&waveOutCritical);
	waveOutPrepareHeader(hWaveOut, current, sizeof(WAVEHDR));
	waveOutWrite(hWaveOut, current, sizeof(WAVEHDR));
	RenderNum++;
	LeaveCriticalSection(&waveOutCritical);

	EnterCriticalSection(&waveCriticalSection);
	waveFreeBlockCount--;
	LeaveCriticalSection(&waveCriticalSection);

	mCritTime.Lock();
	if(mRenderNum == 0)
	{
		mAudioSystemTime = 0;
		if(mSeeking) {
			if(mObserver) {
				mObserver->pObserver(mObserver->pUserData, ENotifySeekComplete, TTKErrNone, 0, NULL);
			}

			mSeeking = false;
		}
	}

	mRenderNum++;
	TTInt64 nSkipDuration = current->dwUser*1000/mAvgBytesPerSec;

	mAudioBufStartTime = mSinkBuffer.llTime - nSkipDuration;
	if(mAudioBufStartTime < 0) mAudioBufStartTime = 0;
	mAudioSysStartTime = GetTimeOfDay();

	if(mAudioBufStartTime == 0) {
		mAudioSystemTime = GetTimeOfDay();
		mAudioBufferTime = mAudioBufStartTime;
	}

	mCritTime.UnLock();

	waveCurrentBlock++;
	waveCurrentBlock %= BLOCK_COUNT;
	current = &waveBlocks[waveCurrentBlock];
	current->dwUser = 0;

	return TTKErrNone;
}


TTInt CTTWinAudioSink::setVolume(TTInt aLVolume, TTInt aRVolume)
{
	TTCAutoLock Lock(&mCritical); 

	aLVolume = aLVolume & 0xFFFF;
	aRVolume = aRVolume & 0xFFFF;
	TTCBaseAudioSink::setVolume(aLVolume, aRVolume);
	EnterCriticalSection(&waveOutCritical);
	waveOutSetVolume(hWaveOut,(aLVolume<<16 | aRVolume));
	LeaveCriticalSection(&waveOutCritical);

	return TTKErrNone;
}

TTInt CTTWinAudioSink::AudioDone(void* param)
{
	startOne(-1);

	return TTKErrNone;
}

