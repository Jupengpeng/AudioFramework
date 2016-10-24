#include "STAudioOutput.h"
#include "STOSConfig.h"
#include "STSampleBuffer.h"
#include "STThread.h"
#include "STMediaPlayerItf.h"
#include "STBufferConfig.h"

#pragma   comment(lib,"winmm.lib")

/*
* some good values for block size and count
*/
#define BLOCK_SIZE  KPCMBufferSize
#define BLOCK_COUNT 3
/*
* function prototypes
*/ 
static void CALLBACK waveOutProc(HWAVEOUT, UINT, DWORD, DWORD, DWORD);
static WAVEHDR* allocateBlocks(int size, int count);
static void freeBlocks(WAVEHDR* blockArray);
static void writeAudio(HWAVEOUT hWaveOut, LPSTR data, int size);


void CALLBACK STAudioOutput::waveOutProc(HWAVEOUT hWaveOut, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	STAudioOutput* self = (STAudioOutput*)dwInstance;
	if(uMsg != WOM_DONE)
		return;
	self->waveOutProcL();
}

void STAudioOutput::waveOutProcL() 
{
	EnterCriticalSection(&iCriticalSection);
	iWaveFreeBlockCount++;
	if (iRenderEnable)
	{
		Render();
	}
	
	LeaveCriticalSection(&iCriticalSection);
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
	HeapFree(GetProcessHeap(), 0, blockArray);
}

void STAudioOutput::writeAudio(HWAVEOUT hWaveOut, LPSTR data, int size)
{
	WAVEHDR* current;
	int remain;
	current = &iWaveBlocks[iWaveCurrentBlock];

	while(size > 0) 
	{
		/* 
		* first make sure the header we're going to use is unprepared
		*/
		if(current->dwFlags & WHDR_PREPARED) 
			waveOutUnprepareHeader(hWaveOut, current, sizeof(WAVEHDR));

		if(size < ((int)(BLOCK_SIZE - current->dwUser)))
		{
			memcpy(current->lpData + current->dwUser, data, size);
			current->dwUser += size;
			break;
		}

		remain = BLOCK_SIZE - current->dwUser;
		memcpy(current->lpData + current->dwUser, data, remain);
		size -= remain;
		data += remain;
		current->dwBufferLength = BLOCK_SIZE;

		waveOutPrepareHeader(hWaveOut, current, sizeof(WAVEHDR));
		waveOutWrite(hWaveOut, current, sizeof(WAVEHDR));

		//EnterCriticalSection(&waveCriticalSection);
		iWaveFreeBlockCount--;
		//LeaveCriticalSection(&waveCriticalSection);

		/*
		* wait for a block to become free
		*/
		// 		while(!waveFreeBlockCount)
		// 			Sleep(10);
		/*
		* point to the next block
		*/
		iWaveCurrentBlock++;
		iWaveCurrentBlock %= BLOCK_COUNT;
		current = &iWaveBlocks[iWaveCurrentBlock];
		current->dwUser = 0;
	}
}

STAudioOutput::STAudioOutput()	
{
	iRenderEnable = ESTTrue;
	iIsFlushed = ESTFalse;
	InitializeCriticalSection(&iCriticalSection);
	iCurPCM = new STInt16[KMaxWaveFreqSampleNum * 2];
}

STAudioOutput::~STAudioOutput()
{
	SAFE_DELETE(iCurPCM);
	iCurPCM = NULL;
	DeleteCriticalSection(&iCriticalSection);	
}

void STAudioOutput::Pause()
{
	waveOutPause(iHandleWaveOut);
	EnterCriticalSection(&iCriticalSection);
	iRenderEnable = ESTFalse;
	LeaveCriticalSection(&iCriticalSection);
}

void STAudioOutput::Resume()
{
	EnterCriticalSection(&iCriticalSection);
	iRenderEnable = ESTTrue;
	LeaveCriticalSection(&iCriticalSection);

	if (iIsFlushed)
	{
		STSampleBuffer* pBuffer = GetFilledBuffer();
		STASSERT(pBuffer != NULL);
		STASSERT(pBuffer->ValidSize() == KPCMBufferSize * 3);
		iIsFlushed = ESTFalse;
		EnterCriticalSection(&iCriticalSection);
		writeAudio(iHandleWaveOut, (LPSTR)pBuffer->Ptr(), pBuffer->ValidSize()/3);
		writeAudio(iHandleWaveOut, (LPSTR)pBuffer->Ptr()+ pBuffer->ValidSize()/3, pBuffer->ValidSize()/3);
		writeAudio(iHandleWaveOut, (LPSTR)pBuffer->Ptr()+ pBuffer->ValidSize()/3 * 2, pBuffer->ValidSize()/3);
		LeaveCriticalSection(&iCriticalSection);
		STBaseAudioOutput::RecycleBuffer(pBuffer);
	}
	

	waveOutRestart(iHandleWaveOut);
}

STInt STAudioOutput::Open(STInt aSampleRate, STInt aChannels, STBool aRecordSupported)
{
	STBaseAudioOutput::Open(aSampleRate, aChannels, aRecordSupported);

	WAVEFORMATEX wfx; /* look this up in your documentation */
	iWaveBlocks     = allocateBlocks(BLOCK_SIZE, BLOCK_COUNT);
	iWaveFreeBlockCount = BLOCK_COUNT;
	iWaveCurrentBlock  = 0;

	/*
	* set up the WAVEFORMATEX structure.
	*/
	wfx.nSamplesPerSec = aSampleRate; /* sample rate */
	wfx.wBitsPerSample = 16;   /* sample size */
	wfx.nChannels    = aChannels;   /* channels  */
	wfx.cbSize     = 0;   /* size of _extra_ info */
	wfx.wFormatTag   = WAVE_FORMAT_PCM;
	wfx.nBlockAlign   = (wfx.wBitsPerSample * wfx.nChannels) >> 3;
	wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;
	/*
	* try to open the default wave device. WAVE_MAPPER is
	* a constant defined in mmsystem.h, it always points to the
	* default wave device on the system (some people have 2 or
	* more sound cards).
	*/
	if(waveOutOpen(
		&iHandleWaveOut, 
		WAVE_MAPPER, 
		&wfx, 
		(DWORD_PTR)waveOutProc, 
		(DWORD_PTR)this, 
		CALLBACK_FUNCTION
		) != MMSYSERR_NOERROR) {
			 			printf("unable to open wave mapper device\n");
			// 			ExitProcess(1);

			return STKErrAccessDenied;
	}

	return STKErrNone;
}

void STAudioOutput::Close()
{	
	waveOutClose(iHandleWaveOut);
	freeBlocks(iWaveBlocks);
}

void STAudioOutput::Start()
{
	STSampleBuffer* pBuffer = GetFilledBuffer();
	STASSERT(pBuffer != NULL);
	STASSERT(pBuffer->ValidSize() == KPCMBufferSize * 3);

	EnterCriticalSection(&iCriticalSection);
	iRenderEnable = ESTTrue;
	LeaveCriticalSection(&iCriticalSection);

	writeAudio(iHandleWaveOut, (LPSTR)pBuffer->Ptr(), pBuffer->ValidSize()/3);
	writeAudio(iHandleWaveOut, (LPSTR)pBuffer->Ptr()+ pBuffer->ValidSize()/3, pBuffer->ValidSize()/3);
	writeAudio(iHandleWaveOut, (LPSTR)pBuffer->Ptr()+ pBuffer->ValidSize()/3 * 2, pBuffer->ValidSize()/3);
	RecycleBuffer(pBuffer);

	iIsFlushed = ESTFalse;
}

void STAudioOutput::Stop()
{
	EnterCriticalSection(&iCriticalSection);
	iRenderEnable = ESTFalse;	
	LeaveCriticalSection(&iCriticalSection);
	waveOutReset(iHandleWaveOut);
}

void STAudioOutput::Render()
{
	while (iWaveFreeBlockCount > 0)
	{
		STASSERT(iCurSampleBuffer == NULL);
		iCurSampleBuffer = GetFilledBuffer();
		if (iCurSampleBuffer != NULL)
		{
			STASSERT(iCurSampleBuffer->ValidSize() == KPCMBufferSize);
			writeAudio(iHandleWaveOut, (LPSTR)(iCurSampleBuffer->Ptr() + iCurSampleBuffer->GetPosition()), iCurSampleBuffer->ValidSize());	
			RecycleBuffer(iCurSampleBuffer);
			STBaseAudioOutput::SyncPosition(iCurSampleBuffer->GetByteOffset());
			iCritical.Lock();
			memcpy(iCurPCM, iCurSampleBuffer->Ptr(), KMaxWaveFreqSampleNum * 2 * sizeof(STInt16));
			iCritical.UnLock();
			iCurSampleBuffer = NULL;
		}
		else
		{
			break;
		}
	}

	iPluginThreadHandle->Resume();
}

void STAudioOutput::Flush()
{
	STBaseAudioOutput::Flush();
	waveOutReset(iHandleWaveOut);
	iIsFlushed = ESTTrue;
	EnterCriticalSection(&iCriticalSection);
	iWaveFreeBlockCount = BLOCK_COUNT;
	LeaveCriticalSection(&iCriticalSection);
}

STBool STAudioOutput::IsUnderflow()
{
	return ESTTrue;
}

STInt STAudioOutput::GetCurWave(STInt16* aWave, STInt aSamples, STInt& aChannels)
{
	iCritical.Lock();
	aChannels = iChannels;
	memcpy(aWave, iCurPCM, aSamples * iChannels * sizeof(STInt16));
	iCritical.UnLock();
	return STKErrNone;
}

void STAudioOutput::SetAccompanimentVolume(STInt aVolume)
{

}

void STAudioOutput::SetRecorderVolume(STInt aVolume)
{

}