#include "STSampleBuffer.h"
#include "STMacrodef.h"

STSampleBuffer::STSampleBuffer(STInt aBufferSize, STUint8* aDataPtr, STBool aIsFirstBuffer)
	: iDataPtr(aDataPtr)
    , iSize(aBufferSize)
	, iPosition(0)
    , iByteOffset(0)
	, iIsFirstBuffer(aIsFirstBuffer)
	, iStreamIndex(0)
{
	
}

STSampleBuffer::~STSampleBuffer()
{
	
}

STInt STSampleBuffer::GetByteOffset()
{
	return iByteOffset;
}

STInt STSampleBuffer::GetPosition()
{
	return iPosition;
}

STBool STSampleBuffer::IsFirstBuffer()
{
	return iIsFirstBuffer;
}

void STSampleBuffer::Reset()
{
	iByteOffset = 0;
	iPosition  = 0;
}

STUint8* STSampleBuffer::Ptr()
{
	return iDataPtr;
}

STInt STSampleBuffer::Size()
{
	return iSize;
}

STInt STSampleBuffer::ValidSize()
{
	return (iSize - iPosition);
}

void STSampleBuffer::SetPosition(STInt aPositon)
{
	iPosition = aPositon;
}

void STSampleBuffer::SetByteOffset(STInt aByteOffset)
{
	iByteOffset = aByteOffset;
}

void STSampleBuffer::SetStreamIndex(STInt aIndex)
{
	STASSERT(aIndex >= 0);
	iStreamIndex = aIndex;
}
	
STInt STSampleBuffer::GetStreamIndex()
{
	return iStreamIndex;
}