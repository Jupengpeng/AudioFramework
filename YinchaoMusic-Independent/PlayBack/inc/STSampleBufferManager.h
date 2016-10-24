#ifndef __ST_SAMPLE_BUFFER_MANAGER_H__
#define __ST_SAMPLE_BUFFER_MANAGER_H__
#include "STTypedef.h"
#include "STMacrodef.h"
#include "STSampleBuffer.h"
#include "STArray.h"
class STSampleBufferManager 
{
public:
	STSampleBufferManager(STInt aTotalSize);
	~STSampleBufferManager();

	/**
	* \fn								void RecycleFreeBuffer(STSampleBuffer* aBuffer)
	* \brief							回收空闲Buffer
	* \param[in] aBuffer				空闲Buffer指针
	*/
	void								RecycleFreeBuffer(STSampleBuffer* aBuffer);

	void 								Assign(STInt aStreamCount, STInt aFirstBufferSize, STInt aSampleBufferSize);

	STSampleBuffer*						GetFreeBuffer();
	STSampleBuffer*						GetFirstBuffer();

private:
	STUint8*							iTotalMemPtr;
	STPointerArray<STSampleBuffer>		iFreeBufferArray;
	STInt								iTotalSize;
	STInt								iStreamCount;
};

#endif
