#include "STOSConfig.h"
#include "STSampleBufferManager.h"
#include "STBufferConfig.h"
#include "STLog.h"

STSampleBufferManager::STSampleBufferManager(STInt aTotalSize)
	:iTotalSize(aTotalSize)
{
	iTotalMemPtr = (STUint8*)malloc(aTotalSize);
}

STSampleBufferManager::~STSampleBufferManager()
{
	iFreeBufferArray.ResetAndDestroy();
	iFreeBufferArray.Close();
	SAFE_FREE(iTotalMemPtr);
}

void STSampleBufferManager::Assign(STInt aStreamCount, STInt aFirstBufferSize, STInt aSampleBufferSize)
{
	iStreamCount = aStreamCount;
	STASSERT(iTotalSize > aStreamCount * aFirstBufferSize);
	iFreeBufferArray.Reset();

	STInt nMemOffset = 0;
	for (STInt i = 0; i < aStreamCount; i++)
	{
		iFreeBufferArray.Append(new STSampleBuffer(aFirstBufferSize, iTotalMemPtr + nMemOffset, ESTTrue));
		nMemOffset += aFirstBufferSize;
	}

	while (iTotalSize - nMemOffset >= aSampleBufferSize)
	{
		STSampleBuffer* pBuffer = new STSampleBuffer(aSampleBufferSize, iTotalMemPtr + nMemOffset, ESTFalse);
		nMemOffset += aSampleBufferSize;
		iFreeBufferArray.Insert(pBuffer, 0);
	}
}

STSampleBuffer* STSampleBufferManager::GetFreeBuffer()
{
	STSampleBuffer* pBuffer = NULL;
	STInt nLastFreeIdx = iFreeBufferArray.Count() - iStreamCount - 1;
	if (nLastFreeIdx >= 0)
	{
		pBuffer = iFreeBufferArray[nLastFreeIdx];
		iFreeBufferArray.Remove(nLastFreeIdx);
	}
	return pBuffer;
}

STSampleBuffer* STSampleBufferManager::GetFirstBuffer()
{
	STSampleBuffer* pBuffer = NULL;
	for (STInt i = iFreeBufferArray.Count() - 1; i >= 0; i--)
	{
		if (iFreeBufferArray[i]->IsFirstBuffer())
		{
			pBuffer = iFreeBufferArray[i];
			iFreeBufferArray.Remove(i);
			break;
		}
	}	

	return pBuffer;
}

void STSampleBufferManager::RecycleFreeBuffer(STSampleBuffer* aBuffer)
{
	STASSERT(aBuffer != NULL);
	aBuffer->Reset();
	if(aBuffer->IsFirstBuffer())
	{   
		iFreeBufferArray.Append(aBuffer);
	}
	else
	{
		iFreeBufferArray.Insert(aBuffer, 0);
	}
}
