#ifndef _RIFF_H__
#define _RIFF_H__
#include "TTTypedef.h"
typedef struct WAVFormat_t 
{
	TTInt16 iFmtTag;
	TTInt16 iFmtChannels;
	TTInt   iSamplePerSec;
	TTInt   iAvgBytesPerSec;
	TTInt16 iBlockAlign;
	TTInt16 iBitsPerSample;
}TTWAVFormat;

typedef struct WAVFmtBlock_t
{
	TTUint32	iFmtFlag;
	TTInt		iFmtSize;
	TTWAVFormat iWavFormat;
}TTWAVFmtBlock;

typedef struct WAVRIFFHeader_t 
{
	TTUint32 iRIFFFlag;
	TTInt	 iRIFFSize;
	TTUint32 iRIFFType;
}TTWAVRIFFHeader;

typedef struct WAVChunkHeader_t
{
	TTUint32 iChunkFlag;
	TTInt   iChunkSize;
}TTWAVChunkHeader;

#endif
