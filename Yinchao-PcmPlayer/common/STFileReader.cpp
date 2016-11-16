#include <stdlib.h>
#include <string.h>
#include "STFileReader.h"
#include "STMacrodef.h"

static const int KPreReadBufferSize = 128 * KILO;
static const int KReadPosInvalid = -1;

STFileReader::STFileReader()
	: iFile(NULL)
	, iFileSize(0)
	, iCurPreReadBufferPos(KReadPosInvalid)
{
	iPreReadBuffer = (unsigned char*)malloc(KPreReadBufferSize);
}

STFileReader::~STFileReader()
{
	Close();
	if (iPreReadBuffer!=NULL)
	{
		free(iPreReadBuffer);
		iPreReadBuffer=NULL;
	}

}

int STFileReader::Open(const char* aUrl)
{
	Close();

	iFile = fopen(aUrl, "rb");

	return ((iFile != NULL) && ((STKErrNone == fseek(iFile, 0, SEEK_END)) && ((iFileSize = int(ftell(iFile))) != STKErrNotFound))) ? STKErrNone : STKErrAccessDenied;
}

int STFileReader::Close()
{
	int nErr = STKErrNone;

	if ((iFile != NULL) && ((nErr = fclose(iFile)) == STKErrNone))	
		iFile = NULL;

	iCurPreReadBufferPos = KReadPosInvalid;
	iFileSize = 0;

	return nErr;
}

int STFileReader::Read(unsigned char* aReadBuffer, int aReadPos, int aReadSize)
{
	if (iFileSize > aReadPos)
	{
		int nErr = STKErrNone;

		int nReadSize = (aReadPos + aReadSize > iFileSize) ? (iFileSize - aReadPos) : aReadSize;

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

int STFileReader::ReadFile(unsigned char* aReadBuffer, int aReadPos, int aReadSize)
{
	int nErr = STKErrNone;

	if ((aReadPos >= 0) && (aReadPos <= iFileSize) && (aReadSize > 0))
	{
		int nReadSize = (aReadPos + aReadSize > iFileSize) ? (iFileSize - aReadPos ) : aReadSize;

		if (STKErrNone == fseek(iFile, aReadPos, SEEK_SET)) 
			nErr = (int)fread((void*)aReadBuffer, 1, nReadSize, iFile);
		else
			nErr = STKErrAccessDenied;
	}
	else
	{
		nErr = STKErrOverflow;
	}

	return nErr;
}

int STFileReader::CheckPreRead(unsigned char* aReadBuffer, int aReadPos, int aReadSize)
{
	int nErr = STKErrUnderflow;
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

void STFileReader::PreRead(int aReadPos)
{
	STASSERT(iPreReadBuffer != NULL);
	int nErr = ReadFile(iPreReadBuffer, aReadPos, KPreReadBufferSize);
	iCurPreReadBufferPos = (nErr > 0) ? aReadPos : KReadPosInvalid;//如果读取了文件，则更新位置
}

int STFileReader::Size() const
{
	return iFileSize;
}
int STFileReader::GetDownloadPercent()
{

	return Size();
}

int STFileReader::CheckReadInt(int aReadPos, int aIntSize, int& aIntOffset)
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
