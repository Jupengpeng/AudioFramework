#include <stdlib.h>
#include <string.h>
#include "STTypedef.h"
#include "STFileReader.h"
#include "STOSConfig.h"
#include "STMacrodef.h"

static const STInt KPreReadBufferSize = 128 * KILO;
static const STInt KReadPosInvalid = -1;

STFileReader::STFileReader()
: iFile(NULL)
, iFileSize(0)
, iCurPreReadBufferPos(KReadPosInvalid)
{
	iPreReadBuffer = (STUint8*)malloc(KPreReadBufferSize);
	STASSERT(iPreReadBuffer != NULL);
}

STFileReader::~STFileReader()
{
	Close();

	SAFE_FREE(iPreReadBuffer);
}

STInt STFileReader::Open(const STChar* aUrl)
{
	Close();

	iFile = fopen(aUrl, "rb");

	return ((iFile != NULL) && ((STKErrNone == fseek(iFile, 0, SEEK_END)) && ((iFileSize = STInt(ftell(iFile))) != STKErrNotFound))) ? STKErrNone : STKErrAccessDenied;
}

STInt STFileReader::Close()
{
	STInt nErr = STKErrNone;

	if ((iFile != NULL) && ((nErr = fclose(iFile)) == STKErrNone))	
		iFile = NULL;

	iCurPreReadBufferPos = KReadPosInvalid;
	iFileSize = 0;

	return nErr;
}

STInt STFileReader::Read(STUint8* aReadBuffer, STInt aReadPos, STInt aReadSize)
{
	if (iFileSize > aReadPos)
	{
		STInt nErr = STKErrNone;
	
		STInt nReadSize = (aReadPos + aReadSize > iFileSize) ? (iFileSize - aReadPos) : aReadSize;
		
		if (nReadSize <= KPreReadBufferSize)
		{
			nErr = CheckPreRead(aReadBuffer, aReadPos, nReadSize);
			if(nErr == STKErrUnderflow)
			{
				PreRead(aReadPos);
				nErr = CheckPreRead(aReadBuffer, aReadPos, nReadSize);		
			}
		}
		else
		{
			nErr = ReadFile(aReadBuffer, aReadPos, aReadSize);
		}

		return nErr;
	}

	return STKErrOverflow;
}

STInt STFileReader::ReadFile(STUint8* aReadBuffer, STInt aReadPos, STInt aReadSize)
{
	STInt nErr = STKErrNone;

	if ((aReadPos >= 0) && (aReadPos <= iFileSize) && (aReadSize > 0))
	{
		STInt nReadSize = (aReadPos + aReadSize > iFileSize) ? (iFileSize - aReadPos ) : aReadSize;

		if (STKErrNone == fseek(iFile, aReadPos, SEEK_SET)) 
			nErr = (STInt)fread((void*)aReadBuffer, 1, nReadSize, iFile);
		else
			nErr = STKErrAccessDenied;
	}
	else
	{
		nErr = STKErrOverflow;
	}

	return nErr;
}

STInt STFileReader::CheckPreRead(STUint8* aReadBuffer, STInt aReadPos, STInt aReadSize)
{
	STInt nErr = STKErrUnderflow;
	if (iCurPreReadBufferPos != KReadPosInvalid)
	{
		if ((iCurPreReadBufferPos <= aReadPos) && ((iCurPreReadBufferPos + KPreReadBufferSize) >= (aReadSize + aReadPos)))
		{
			STASSERT(iPreReadBuffer != NULL);
			memcpy(aReadBuffer, iPreReadBuffer + aReadPos - iCurPreReadBufferPos, aReadSize);
			nErr = aReadSize;
		}
	}

	return nErr;
}

void STFileReader::PreRead(STInt aReadPos)
{
	STASSERT(iPreReadBuffer != NULL);
	STInt nErr = ReadFile(iPreReadBuffer, aReadPos, KPreReadBufferSize);
	iCurPreReadBufferPos = (nErr > 0) ? aReadPos : KReadPosInvalid;//如果读取了文件，则更新位置
}

STInt STFileReader::Size() const
{
	return iFileSize;
}

STInt STFileReader::CheckReadInt(STInt aReadPos, STInt aIntSize, STInt& aIntOffset)
{
	if (iCurPreReadBufferPos != KReadPosInvalid)
	{
		if (!((iCurPreReadBufferPos <= aReadPos) && ((iCurPreReadBufferPos + KPreReadBufferSize) >= (aIntSize + aReadPos))))
		{
			PreRead(aReadPos);
			aIntOffset = 0;
		}
		else
		{
			aIntOffset = aReadPos - iCurPreReadBufferPos;
		}

		return STKErrNone;
	}

	return STKErrUnderflow;
}

ISTDataReaderItf::DataReaderId STFileReader::Id()
{
	return ISTDataReaderItf::EDataReaderIdLoacal;
}
//end of file
