// INCLUDES
#include <stdlib.h>
#include <string.h>
#include "TTMacrodef.h"
#include "TTFileReader.h"
#include "TTOsalConfig.h"
//#include "TThread.h"

static const TTInt KPreReadBufferSize = 64 * KILO;
static const TTInt KReadPosInvalid = -1;

TTBool CTTFileReader::IsSourceValid(const TTChar* aUrl)
{
    FILE* pFile = fopen(aUrl, "rb");
	if (pFile != NULL)
	{
		fclose(pFile);
		return ETTTrue;
	}
    
	return ETTFalse;    
}

CTTFileReader::CTTFileReader()
: iFile(NULL)
, iFileSize(0)
, iCurPreReadBufferPos(KReadPosInvalid)
{
	iPreReadBuffer = (TTUint8*)malloc(KPreReadBufferSize);
	TTASSERT(iPreReadBuffer != NULL);
}

CTTFileReader::~CTTFileReader()
{
	Close();

	SAFE_FREE(iPreReadBuffer);
}

TTInt CTTFileReader::Open(const TTChar* aUrl)
{
	Close();

	iFile = fopen(aUrl, "rb");

	return ((iFile != NULL) && ((TTKErrNone == fseek(iFile, 0, SEEK_END)) && ((iFileSize = TTInt(ftell(iFile))) != TTKErrNotFound))) ? TTKErrNone : TTKErrAccessDenied;
}

TTInt CTTFileReader::Close()
{
	TTInt nErr = TTKErrNone;

	if ((iFile != NULL) && ((nErr = fclose(iFile)) == TTKErrNone))	
		iFile = NULL;

	iCurPreReadBufferPos = KReadPosInvalid;
	iFileSize = 0;
	iAudioBitrate = 0;
	iVideoBitrate = 0;

	return nErr;
}

TTInt CTTFileReader::ReadSync(TTUint8* aReadBuffer, TTInt aReadPos, TTInt aReadSize)
{
	if (iFileSize > aReadPos)
	{
		TTInt nErr = TTKErrNone;
	
		TTInt nReadSize = (aReadPos + aReadSize > iFileSize) ? (iFileSize - aReadPos) : aReadSize;
		
		if (nReadSize <= KPreReadBufferSize)
		{
			nErr = CheckPreRead(aReadBuffer, aReadPos, nReadSize);
			if(nErr == TTKErrUnderflow)
			{
				PreRead(aReadPos);
				nErr = CheckPreRead(aReadBuffer, aReadPos, nReadSize);		
			}
			else
			{
				nErr = aReadSize;
			}
		}
		else
		{
			nErr = Read(aReadBuffer, aReadPos, aReadSize);
		}

		return nErr;
	}

	return TTKErrOverflow;
}

TTInt CTTFileReader::ReadWait(TTUint8* aReadBuffer, TTInt aReadPos, TTInt aReadSize)
{
	return ReadSync(aReadBuffer, aReadPos, aReadSize);
}


TTInt CTTFileReader::Read(TTUint8* aReadBuffer, TTInt aReadPos, TTInt aReadSize)
{
	TTInt nErr = TTKErrNone;

	if ((aReadPos >= 0) && (aReadPos <= iFileSize) && (aReadSize > 0))
	{
		TTInt nReadSize = (aReadPos + aReadSize > iFileSize) ? (iFileSize - aReadPos ) : aReadSize;

		if (TTKErrNone == fseek(iFile, aReadPos, SEEK_SET)) 
			nErr = (TTInt)fread((void*)aReadBuffer, 1, nReadSize, iFile);
		else
			nErr = TTKErrAccessDenied;
	}
	else
	{
		nErr = TTKErrOverflow;
	}

	return nErr;
}

TTInt CTTFileReader::CheckPreRead(TTUint8* aReadBuffer, TTInt aReadPos, TTInt aReadSize)
{
	TTInt nErr = TTKErrUnderflow;
	if (iCurPreReadBufferPos != KReadPosInvalid)
	{
		if ((iCurPreReadBufferPos <= aReadPos) && ((iCurPreReadBufferPos + KPreReadBufferSize) >= (aReadSize + aReadPos)))
		{
			TTASSERT(iPreReadBuffer != NULL);
			memcpy(aReadBuffer, iPreReadBuffer + aReadPos - iCurPreReadBufferPos, aReadSize);
			nErr = aReadSize;
		}
	}

	return nErr;
}

void CTTFileReader::PreRead(TTInt aReadPos)
{
	TTASSERT(iPreReadBuffer != NULL);
	TTInt nErr = Read(iPreReadBuffer, aReadPos, KPreReadBufferSize);
	iCurPreReadBufferPos = (nErr > 0) ? aReadPos : KReadPosInvalid;//如果读取了文件，则更新位置
}

TTInt CTTFileReader::Size() const
{
	return iFileSize;
}

TTUint CTTFileReader::BufferedSize()
{
	return iFileSize;
}

//void CTTFileReader::ReadAsync(TTUint8* aReadBuffer, TTInt aReadPos, TTInt aReadSize)
//{
//	//if (iAsyncReaderEnable)
//	//{
//	//	TTASSERT((aReadSize > 0) && (aReadPos >= 0) && (aReadBuffer != NULL) && (iReaderObserver != NULL));
//
//	//	iAsyncReadPos = aReadPos;
//	//	iAsyncReadSize = aReadSize;
//	//	iAsyncReadBuffer = aReadBuffer;
//
//	//	TTRequestStatus* status = &iStatus;
//	//	iStatus = TTRequestStatus::ERequestPending;
//	//	SetActive();
//	//	TTUser::RequestComplete(status, TTKErrNone);
//	//}
//}


TTInt CTTFileReader::PrepareCache(TTInt aReadPos, TTInt aReadSize, TTInt aFlag)
{
	if(aReadPos > iFileSize || aReadPos + aReadSize > iFileSize)
		return TTKErrOverflow;
	
	return TTKErrNone;
}

TTInt CTTFileReader::CheckReadInt(TTInt aReadPos, TTInt aIntSize, TTInt& aIntOffset)
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

		return TTKErrNone;
	}

	return TTKErrUnderflow;
}

TTUint16 CTTFileReader::ReadUint16(TTInt aReadPos)// 按高字节在后面读
{
	TTInt nIntOffset = 0;
	if (TTKErrNone == CheckReadInt(aReadPos, sizeof(TTUint16), nIntOffset))
	{
		TTUint8* ptr = iPreReadBuffer + nIntOffset;

		return TTUint16((*(ptr + 1)) << 8) | (*(ptr));
	}

	TTASSERT(ETTFalse);
	return 0;	
}

TTUint16 CTTFileReader::ReadUint16BE(TTInt aReadPos)// 按高字节在前面读
{
	TTInt nIntOffset = 0;
	if (TTKErrNone == CheckReadInt(aReadPos, sizeof(TTUint16), nIntOffset))
	{
		TTUint8* ptr = iPreReadBuffer + nIntOffset;

		return TTUint16(*(ptr) << 8) | ((*(ptr + 1)));
	}

	TTASSERT(ETTFalse);
	return 0;	
}

TTUint32 CTTFileReader::ReadUint32(TTInt aReadPos)
{
	TTInt nIntOffset = 0;
	if (TTKErrNone == CheckReadInt(aReadPos, sizeof(TTUint32), nIntOffset))
	{
		TTUint8* ptr = iPreReadBuffer + nIntOffset;

		return TTUint32((*(ptr + 3)) << 24) | TTUint32((*(ptr + 2)) << 16)
			| TTUint32((*(ptr + 1)) << 8) | ((*(ptr)));
	}

	TTASSERT(ETTFalse);
	return 0;
}

TTUint32 CTTFileReader::ReadUint32BE(TTInt aReadPos)
{
	TTInt nIntOffset = 0;
	if (TTKErrNone == CheckReadInt(aReadPos, sizeof(TTUint32), nIntOffset))
	{
		TTUint8* ptr = iPreReadBuffer + nIntOffset;

		return TTUint32((*(ptr)) << 24) | TTUint32((*(ptr + 1)) << 16)
			| TTUint32((*(ptr + 2)) << 8) | ((*(ptr + 3)));
	}

	TTASSERT(ETTFalse);
	return 0;
}

TTUint64 CTTFileReader::ReadUint64(TTInt aReadPos)
{
	TTInt nIntOffset = 0;
	if (TTKErrNone == CheckReadInt(aReadPos, sizeof(TTUint64), nIntOffset))
	{
		TTUint8* ptr = iPreReadBuffer + nIntOffset;
		TTUint32 nLsb = TTUint32((*(ptr + 3)) << 24) | TTUint32((*(ptr + 2)) << 16)
							| TTUint32((*(ptr + 1)) << 8) | ((*(ptr)));

		ptr += 4;
		TTUint32 nMsb =  TTUint32((*(ptr + 3)) << 24) | TTUint32((*(ptr + 2)) << 16)
			| TTUint32((*(ptr + 1)) << 8) | ((*(ptr)));	

		TTUint64 nRetVal = nMsb;

		return ((nRetVal << 32) | nLsb);
	}

	TTASSERT(ETTFalse);
	return 0;
}

TTUint64 CTTFileReader::ReadUint64BE(TTInt aReadPos)
{
	TTInt nIntOffset = 0;
	if (TTKErrNone == CheckReadInt(aReadPos, sizeof(TTUint64), nIntOffset))
	{
		TTUint8* ptr = iPreReadBuffer + nIntOffset;

		TTUint32 nMsb = TTUint32((*(ptr)) << 24) | TTUint32((*(ptr + 1)) << 16)
			| TTUint32((*(ptr + 2)) << 8) | ((*(ptr + 3)));

		ptr += 4;
		TTUint32 nLsb = TTUint32((*(ptr)) << 24) | TTUint32((*(ptr + 1)) << 16)
			| TTUint32((*(ptr + 2)) << 8) | ((*(ptr + 3)));		

		TTUint64 nRetVal = nMsb;

		return ((nRetVal << 32) | nLsb);
	}

	TTASSERT(ETTFalse);
	return 0;
}

CTTFileReader::TTDataReaderId CTTFileReader::Id()
{
	return ITTDataReader::ETTDataReaderIdFile;
}
//end of file
