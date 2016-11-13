//  Include Files  
#include "TTMacrodef.h"
#include "TTFileReader.h"
#include "TTMediaParser.h"
#include "TTIntReader.h"
#include "TTLog.h"
#ifdef __TT_OS_IOS__
#include "TTIPodLibraryFileReader.h"
#include "TTExtAudioFileReader.h"
#include "TTIPodLibraryPCMOutputFormat.h"
//#include "AVCDecoderTypes.h"
#endif

#define RETURN_IF_EOF(start, length, end)	{	\
			if((start) + (length) >= (end))		\
				return TTKErrEof;				\
			}

CTTMediaParser::CTTMediaParser(ITTDataReader& aDataReader, ITTMediaParserObserver& aObserver)
: iDataReader(aDataReader)
, iObserver(aObserver)
, iParserMediaInfoRef(NULL)
, iFrmPosTab(NULL)
, iFrmTabSize(0)
, iFrmPosTabDone(ETTFalse)
, iAccessBeyond(ETTFalse)
, iFrmPosTabCurOffset(0)
, iFrmPosTabCurIndex(0)
, iNextSeekPos(0)
, iPreSeekIdx(-2)
, iPreSeekFrameSize(0)
, iNALLengthSize(0)
, iAVCBuffer(NULL)
, iAVCSize(0)
, iStreamAudioCount(0)
, iStreamVideoCount(0)
, iRawDataBegin(0)
, iRawDataEnd(0)
, iAudioBuffer(NULL)
, iAudioSize(0)
, iVideoBuffer(NULL)
, iVideoSize(0)
, iCurAudioReadFrmIdx(0)
, iCurVideoReadFrmIdx(0)
, iAudioStreamId(-1)
, iVideoStreamId(-1)
, iReadEof(ETTFalse)
, iAudioSeek(ETTFalse)
, iVideoSeek(ETTFalse)
{
	mSemaphore.Create();
	iSyncReadBufferSize = KSyncBufferSize;
	iSyncReadBuffer = new TTUint8[iSyncReadBufferSize];
	iAsyncReadBuffer = new TTUint8[KAsyncBufferSize];
}

CTTMediaParser::~CTTMediaParser()
{
	SAFE_FREE(iAudioBuffer);
	SAFE_FREE(iVideoBuffer);
	SAFE_DELETE_ARRAY(iAVCBuffer);
	SAFE_DELETE_ARRAY(iFrmPosTab);
	SAFE_DELETE_ARRAY(iAsyncReadBuffer);
	SAFE_DELETE_ARRAY(iSyncReadBuffer);
	mSemaphore.Signal();
	mSemaphore.Destroy();
}

TTInt CTTMediaParser::StreamCount()
{
	return iStreamAudioCount + iStreamVideoCount;
}

TTReadResult CTTMediaParser::ReadStreamData(TTInt aReadPos, TTUint8 *&aDataPtr, TTInt &aReadSize)
{
	TTReadResult tResult = EReadOK;

	if (aReadPos < 0)
	{
		tResult = EReadErr;
	}
	else if (aReadPos >= iRawDataEnd)
	{
		aReadSize = 0;
		return EReadEof;
	}
	else
	{
		TTInt nRemainSize = iRawDataEnd - aReadPos;
		TTInt nReadSize = MIN(nRemainSize, aReadSize);
		if (iSyncReadBufferSize < nReadSize)
		{
			SAFE_DELETE_ARRAY(iSyncReadBuffer);
			iSyncReadBufferSize = nReadSize;
			iSyncReadBuffer = new TTUint8[iSyncReadBufferSize];
		}

		aReadSize = iDataReader.ReadSync(iSyncReadBuffer, aReadPos, nReadSize);
		aDataPtr = iSyncReadBuffer;
		if(aReadPos + aReadSize >= iRawDataEnd)
		{
			tResult = EReadEof;
		}
		else if(aReadSize == 0)
		{
			tResult = EReadUnderflow;
		}
		else if(aReadSize < 0)
		{
			tResult = EReadErr;
		}
		else if (aReadSize != nReadSize)
		{
			tResult = EReadUnderflow;
		}
	}

	//LOGI("CTTMediaParser::ReadStreamData return: %d", tResult);
	return tResult;	
}

//void CTTMediaParser::ReadStreamDataAsync(TTInt aReadPos)
//{
//	TTASSERT(aReadPos >= 0);
//	
//	TTInt nReadSize = KAsyncBufferSize;
//	if (iRawDataEnd - aReadPos < KAsyncBufferSize)
//	{
//		nReadSize = iRawDataEnd - aReadPos;
//		
//		if ((nReadSize <= 0) || iReadEof)
//		{
//			iFrmPosTabDone = ETTTrue;
//			iObserver.CreateFrameIdxComplete();
//			return;
//		}
//
//		iReadEof = ETTTrue;
//	}
//	
//	iDataReader.ReadAsync(iAsyncReadBuffer, aReadPos, nReadSize);	
//}

TTInt CTTMediaParser::GetFrameLocation(TTInt aStreamId, TTInt aFrmIdx, TTMediaFrameInfo& aFrameInfo)
{
	TTInt nErr = SeekWithinFrmPosTab(aStreamId, aFrmIdx, aFrameInfo);

	if(TTKErrNotFound == nErr)
	{
		nErr = SeekWithoutFrmPosTab(aStreamId, aFrmIdx, aFrameInfo);
	}
	
	return nErr;
}

TTInt CTTMediaParser::GetFrameLocation(TTInt aStreamId, TTInt& aFrmIdx, TTUint64 aTime)
{
	return TTKErrNotFound;
}

TTInt CTTMediaParser::GetFrameTimeStamp(TTInt /*aStreamId*/, TTInt /*aFrmIdx*/, TTUint64 /*aPlayTime*/)
{
	return TTKErrNotSupported;
}

void CTTMediaParser::StartFrmPosScan()
{
	if(iFrmPosTabDone)
		return;

	if (iFrmPosTab == NULL)
	{
		FrmIdxTabAlloc();
		iFrmPosTabCurOffset = iRawDataBegin;
	}

	for( ; ; )
	{
		TTInt nReadSize = KAsyncBufferSize;
		if (iRawDataEnd - iFrmPosTabCurOffset < KAsyncBufferSize)
		{
			nReadSize = iRawDataEnd - iFrmPosTabCurOffset;

			if ((nReadSize <= 0) || iReadEof)
			{
				iFrmPosTabDone = ETTTrue;
				//iObserver.CreateFrameIdxComplete();
				return;
			}

			iReadEof = ETTTrue;
		}

		iDataReader.ReadSync(iAsyncReadBuffer, iFrmPosTabCurOffset, nReadSize);

		ParseFrmPos(iAsyncReadBuffer, nReadSize);
	}
}

void CTTMediaParser::FrmIdxTabAlloc()
{
	iFrmTabSize = KTabSize;
	TTASSERT(iFrmPosTab == NULL);
	iFrmPosTab = new TTUint[iFrmTabSize];
}

void CTTMediaParser::FrmIdxTabReAlloc()
{
	TTUint* pNewFrmTab = new TTUint[iFrmTabSize + KReLINCSize];

	if (pNewFrmTab != NULL)
	{
		memcpy((void*)pNewFrmTab, (void*)iFrmPosTab, iFrmTabSize * sizeof(TTUint32));
		iFrmTabSize += KReLINCSize;
		delete iFrmPosTab;
		iFrmPosTab = pNewFrmTab;
	}
	else
	{
		TTASSERT(ETTFalse);
		iFrmPosTabDone = ETTTrue;
	}
}

TTInt CTTMediaParser::SeekWithoutFrmPosTab(TTInt aStreamId, TTInt aFrmIdx, TTMediaFrameInfo& aFrameInfo)
{
	TTInt64& nFramePos  = aFrameInfo.iFrameLocation;
	TTInt& nFrameSize = aFrameInfo.iFrameSize;

	//if (iFrmPosTabDone)
	//{
	//	return TTKErrOverflow;
	//}
	
	if (aFrmIdx == (iPreSeekIdx + 1))//如果访问下一帧
	{
		TTInt nErr = SeekWithPos(aStreamId, iNextSeekPos, nFramePos, nFrameSize);
		if (TTKErrUnderflow == nErr)
		{
			return nErr;
		}
		
		if (TTKErrNone != nErr)
		{
			return TTKErrOverflow;
		}

		RETURN_IF_EOF(nFramePos, nFrameSize, iRawDataEnd);
	}
	else if ((aFrmIdx == iPreSeekIdx) && (aFrmIdx != 0))//如果上次seek后没有分配到buf，则需要再seek同一帧
	{
		iNextSeekPos -= iPreSeekFrameSize;
		nFramePos = iNextSeekPos;
		nFrameSize = iPreSeekFrameSize;
	}
	else
	{
		TTInt nErr = SeekWithIdx(aStreamId, aFrmIdx, nFramePos, nFrameSize);
		if (TTKErrUnderflow == nErr)
		{
			return nErr;
		}

		if (TTKErrNone != nErr && TTKErrEof != nErr)
		{
			return TTKErrOverflow;
		}

		RETURN_IF_EOF(nFramePos, nFrameSize, iRawDataEnd);
	}

	iPreSeekFrameSize = nFrameSize;
	iPreSeekIdx = aFrmIdx;
	iNextSeekPos = nFramePos + nFrameSize;

	TTInt64 nNextFramePos = 0;
	TTInt nNextFrameSize = 0;
	TTInt nErr = SeekWithPos(aStreamId, iNextSeekPos, nNextFramePos, nNextFrameSize);
	if (TTKErrUnderflow == nErr)
	{
		return nErr;
	}
	if (nErr != TTKErrEof && nErr != TTKErrNone)
	{
		return TTKErrOverflow;
	}

	return nErr;
}

TTBool CTTMediaParser::IsCreateFrameIdxComplete()
{
	return iFrmPosTabDone;
}

TTUint CTTMediaParser::MediaDuration(TTInt aStreamId)
{
	return 0;
}

TTUint CTTMediaParser::MediaDuration()
{
	TTUint nDuration = 0;

	if(iAudioStreamId != -1) {
		nDuration = MediaDuration(iAudioStreamId);
	} 
	
	if(iVideoStreamId != -1) {
		if(nDuration < MediaDuration(iVideoStreamId))	
			nDuration = MediaDuration(iVideoStreamId);
	}

	return nDuration;
}

TTInt CTTMediaParser::SelectStream(TTMediaType aType, TTInt aStreamId)
{
	if(aType == EMediaTypeAudio) {
		if(aStreamId >= iStreamAudioCount)
			return TTKErrNotFound;

		iAudioStreamId = aStreamId;
		iDataReader.SetBitrate((TTInt)(EMediaTypeAudio), iParserMediaInfoRef->iAudioInfoArray[aStreamId]->iBitRate);
	} else if(aType == EMediaTypeVideo) {
		if(aStreamId >= EMediaStreamIdVideo && aStreamId < EMediaStreamIdVideo + iStreamVideoCount)	{
			iVideoStreamId = aStreamId;
			iDataReader.SetBitrate((TTInt)(EMediaTypeVideo), iParserMediaInfoRef->iVideoInfo->iBitRate);
			return TTKErrNone;
		}		
	}

	return TTKErrNone;
}

TTInt64 CTTMediaParser::Seek(TTUint64 aPosMS, TTInt aOption)
{
#ifdef __TT_OS_IOS__
    if (iDataReader.Id() == ITTDataReader::ETTDataReaderIdIPodLibraryFile)
    {
        if ((aPosMS + KMinSeekOverflowOffset) < ((CTTIPodLibraryFileReader*)(&iDataReader))->Duration())
        {
            ((CTTIPodLibraryFileReader*)(&iDataReader))->Seek(aPosMS);
          //  ((CTTIPodLibraryFileReader*)iDataReader)->Seek(aPosMS);
        }
        else if (!iReadEof)
        {
			iReadEof = true;
        }
        
        return aPosMS;
    }        
#endif

	TTMediaFrameInfo tMediaFrameInfo;
	TTUint64 TempPos = aPosMS;
	TTInt nFrameIdx = 0;
	TTInt nErr = TTKErrNotSupported;
	TTInt tStreamId = iAudioStreamId;

	if ((TempPos + KMinSeekOverflowOffset) >= MediaDuration(tStreamId)
		|| ((nErr = GetFrameLocation(tStreamId, nFrameIdx, TempPos)) == TTKErrEof))
	{
		return TTKErrEof;
	}
	else
	{
		TTASSERT(TTKErrNone == nErr);
		iCurAudioReadFrmIdx = nFrameIdx;

		nErr = GetFrameLocation(tStreamId, nFrameIdx, tMediaFrameInfo);
		//LOGI("Seek  Index: %d, Time %d", nFrameIdx, tMediaFrameInfo.iFrameStartTime);

		if(nErr == TTKErrNone)
		{
			TempPos = tMediaFrameInfo.iFrameStartTime;
			TTInt nFlag = TTREADER_CACHE_ASYNC;
			iDataReader.PrepareCache(tMediaFrameInfo.iFrameLocation, tMediaFrameInfo.iFrameSize*200, nFlag);
		}

#ifdef __TT_OS_IOS__
		if (iDataReader.Id() == ITTDataReader::ETTDataReaderIdExtAudioFile)
		{
			((CTTExtAudioFileReader*)(&iDataReader))->Seek(nFrameIdx);
		}
#endif
	}		

	if(nErr != TTKErrNone) {
		return nErr;
	}

	iVideoSeek = true;
	iAudioSeek = true;

	return TempPos;
}

TTInt CTTMediaParser::SetParam(TTInt aType, TTPtr aParam)
{
	return TTKErrNotSupported;
}

TTInt CTTMediaParser::GetParam(TTInt aType, TTPtr aParam)
{
	return TTKErrNotSupported;
}

TTInt CTTMediaParser::GetMediaSample(TTMediaType tMediaType, TTBuffer* pMediaBuffer)
{
	TTInt64 nTime = pMediaBuffer->llTime;
	TTUint8* avBuffer = NULL;
	TTMediaFrameInfo tMediaFrameInfo;
	TTInt  *pCurIndex = &iCurAudioReadFrmIdx;
	TTInt tMediaStreamId;

	if(tMediaType == EMediaTypeAudio) {
		if(iAudioSeek) {
			if(pMediaBuffer->nFlag & TT_FLAG_BUFFER_SEEKING) {
				iAudioSeek = false;
				//LOGI("CTTMediaParser::GetMediaSample. iAudioSeek %d", iAudioSeek);
			} else {
				//LOGI("CTTMediaParser::GetMediaSample not ready. iAudioSeek %d", iAudioSeek);
				return TTKErrInUse;
			}
		}
	} else if(tMediaType == EMediaTypeVideo) {
		if(iVideoSeek) {
			if(pMediaBuffer->nFlag & TT_FLAG_BUFFER_SEEKING) {
				iVideoSeek = false;
				//LOGI("CTTMediaParser::GetMediaSample. iAudioSeek %d", iAudioSeek);
			} else {
				//LOGI("CTTMediaParser::GetMediaSample not ready. iAudioSeek %d", iAudioSeek);
				return TTKErrInUse;
			}
		}
	}

	if(tMediaType == EMediaTypeAudio){
		tMediaStreamId = iAudioStreamId;
		pCurIndex = &iCurAudioReadFrmIdx;
	} else {
		tMediaStreamId = iVideoStreamId;
		pCurIndex = &iCurVideoReadFrmIdx;
	}

	TTInt nErr = GetFrameLocation(tMediaStreamId, *pCurIndex, tMediaFrameInfo);

	switch (nErr)
	{
	case TTKErrNone:
	case TTKErrEof:	
		{
			TTInt nFrameSize = tMediaFrameInfo.iFrameSize;
			if (nFrameSize > 0)	{

				if(tMediaType == EMediaTypeAudio){
					if(nFrameSize > iAudioSize)	{
						SAFE_FREE(iAudioBuffer);

						iAudioBuffer = (TTPBYTE)malloc(nFrameSize + 32);
						iAudioSize = nFrameSize + 32;
					}

					avBuffer = iAudioBuffer;
				} else {
					if(nFrameSize > iVideoSize)	{
						SAFE_FREE(iVideoBuffer);

						iVideoBuffer = (TTPBYTE)malloc(nFrameSize + 32);
						iVideoSize = nFrameSize + 32;
					}

					avBuffer = iVideoBuffer;
				}

				TTInt nReadSize = iDataReader.ReadSync(avBuffer, tMediaFrameInfo.iFrameLocation, nFrameSize);

				if (nReadSize == nFrameSize) {
					pMediaBuffer->llTime = tMediaFrameInfo.iFrameStartTime;
					pMediaBuffer->nDuration = tMediaFrameInfo.iFrameDuration;
					pMediaBuffer->nSize = nFrameSize;
					pMediaBuffer->pBuffer = avBuffer;
					pMediaBuffer->lReserve = tMediaFrameInfo.iExtraInfo;
                    
#ifdef __TT_OS_IOS__
                    
                    if (iDataReader.Id() == ITTDataReader::ETTDataReaderIdIPodLibraryFile)
                    {
                        TTPureDataOutputFormat* pFormat = (TTPureDataOutputFormat*)avBuffer;
                        pMediaBuffer->llTime = pFormat->iStartTime;
                        pMediaBuffer->nDuration = pFormat->iStopTime - pFormat->iStartTime;;
                    }
#endif

					(*pCurIndex)++;						
				}
#ifdef __TT_OS_IOS__
				else if (iDataReader.Id() == ITTDataReader::ETTDataReaderIdIPodLibraryFile)
				{
					if (nReadSize == TTKErrEof)
					{
						nErr = TTKErrEof;
					}
					else 
					{
						nErr = nReadSize;//FileException(nReadSize);
					}         
				}
#endif
				else
				{
					//online m4a file may read fail, but that doesn't mean socket read fail ,just need to bufferring more
					if (iDataReader.Id() == ITTDataReader::ETTDataReaderIdHttp || iDataReader.Id() == ITTDataReader::ETTDataReaderIdBuffer) {
						iDataReader.CheckOnLineBuffering();
						return TTKErrNotReady;
					} else {
						nErr = TTKErrEof;
					}
				}
			}
		}
		break;

	case TTKErrUnderflow:
		if (iDataReader.Id() == ITTDataReader::ETTDataReaderIdHttp || iDataReader.Id() == ITTDataReader::ETTDataReaderIdBuffer) {
			iDataReader.CheckOnLineBuffering();
			return TTKErrNotReady;
		} else {
			(*pCurIndex)++;
		}
		break;

	case TTKErrOverflow:
		(*pCurIndex)++;
		break;

	case TTKErrNotFound:
	case TTKErrTooBig:
		{	
			(*pCurIndex)++;
		}
		break;

	default:
		TTASSERT(ETTFalse);
		break;
	}

	return nErr;
}

void CTTMediaParser::ParseFrmPos(const TTUint8* /*aData*/, TTInt /*aParserSize*/)
{
	TTASSERT(ETTFalse);
}

TTInt CTTMediaParser::SeekWithIdx(TTInt aStreamId, TTInt /*aFrmIdx*/, TTInt64& /*aFrmPos*/, TTInt& /*aFrmSize*/)
{
	TTASSERT(ETTFalse);
	return TTKErrOverflow;
}

TTInt CTTMediaParser::SeekWithPos(TTInt aStreamId, TTInt64 /*aPos*/, TTInt64& /*aFrmPos*/, TTInt& /*aFrmSize*/)
{
	TTASSERT(ETTFalse);
	return TTKErrOverflow;
}

//TTInt CTTMediaParser::ConvertAVCHead(TTAVCDecoderSpecificInfo* AVCDecoderSpecificInfo, TTPBYTE pInBuffer, TTUint32 nInSzie)
//{
//	if (pInBuffer == NULL)
//		return TTKErrArgument;
//	
//	if(AVCDecoderSpecificInfo == NULL || AVCDecoderSpecificInfo->iData == NULL 
//		|| AVCDecoderSpecificInfo->iPpsData == NULL || AVCDecoderSpecificInfo->iSpsData == NULL) {
//			return TTKErrArgument; 
//	}
//
//	if (nInSzie < 12)
//		return TTKErrNotSupported;
//
//	TTUint8 configurationVersion = pInBuffer[0];
//	TTUint8 AVCProfileIndication = pInBuffer[1];
//	TTUint8 profile_compatibility = pInBuffer[2];
//	TTUint8 AVCLevelIndication  = pInBuffer[3];
//
//	iNALLengthSize =  (pInBuffer[4]&0x03)+1;
//	TTUint32 nNalWord = 0x01000000;
//	if (iNALLengthSize == 3)
//		nNalWord = 0X010000;
//
//	TTInt nNalLen = iNALLengthSize;
//	if (iNALLengthSize < 3)	{
//		nNalLen = 4;
//	}
//
//	TTUint32 HeadSize = 0;
//	TTInt i = 0;
//
//	TTInt nSPSNum = pInBuffer[5]&0x1f;
//	TTInt nSPSSize = 0;
//	TTPBYTE pBuffer = pInBuffer + 6;
//	TTPBYTE pOutBuffer = AVCDecoderSpecificInfo->iData;
//
//	for (i = 0; i< nSPSNum; i++)
//	{
//		nSPSSize = 0;
//		TTUint32 nSPSLength = (pBuffer[0]<<8)| pBuffer[1];
//		pBuffer += 2;
//
//		memcpy (pOutBuffer + HeadSize, &nNalWord, nNalLen);
//		HeadSize += nNalLen;
//		memcpy(AVCDecoderSpecificInfo->iSpsData, &nNalWord, nNalLen);
//		nSPSSize += nNalLen;
//
//		if(nSPSLength > (nInSzie - (pBuffer - pInBuffer))){
//			return TTKErrNotSupported;
//		}
//
//		memcpy (pOutBuffer + HeadSize, pBuffer, nSPSLength);
//        memcpy(AVCDecoderSpecificInfo->iSpsData + nSPSSize, pBuffer, nSPSLength);
//		HeadSize += nSPSLength;
//		pBuffer += nSPSLength;
//		nSPSSize += nSPSLength;
//	}
//    AVCDecoderSpecificInfo->iSpsSize = nSPSSize;
//
//
//	TTInt nPPSNum = *pBuffer++;
//	TTInt nPPSSize = 0;
//	for (i=0; i< nPPSNum; i++)
//	{
//		TTUint32 nPPSLength = (pBuffer[0]<<8) | pBuffer[1];
//		pBuffer += 2;
//		
//		nPPSSize = 0;
//		memcpy (pOutBuffer + HeadSize, &nNalWord, nNalLen);
//		HeadSize += nNalLen;
//		memcpy(AVCDecoderSpecificInfo->iPpsData, &nNalWord, nNalLen);
//		nPPSSize += nNalLen;
//		
//		if(nPPSLength > (nInSzie - (pBuffer - pInBuffer))){
//			return TTKErrNotSupported;
//		}
//
//		memcpy (pOutBuffer + HeadSize, pBuffer, nPPSLength);
//        memcpy (AVCDecoderSpecificInfo->iPpsData + nPPSSize, pBuffer, nPPSLength);
//		HeadSize += nPPSLength;
//		pBuffer += nPPSLength;
//		nPPSSize += nPPSLength;
//	}
//    AVCDecoderSpecificInfo->iPpsSize = nPPSSize;
//	AVCDecoderSpecificInfo->iSize = HeadSize;
//
//	return TTKErrNone;
//}
//
//TTInt CTTMediaParser::ConvertHEVCHead(TTPBYTE pOutBuffer, TTUint32& nOutSize, TTPBYTE pInBuffer, TTUint32 nInSzie)
//{
//	if (pOutBuffer == NULL || pInBuffer == NULL)
//		return TTKErrArgument;
//
//	if (nInSzie < 22)
//		return TTKErrNotSupported;
//
//	TTPBYTE pData = pInBuffer;
//	iNALLengthSize =  (pData[21]&0x03)+1;
//	TTInt nNalLen = iNALLengthSize;
//	if (iNALLengthSize < 3)	{
//		nNalLen = 4;
//	}
//
//	TTUint32 nNalWord = 0x01000000;
//	if (iNALLengthSize == 3)
//		nNalWord = 0X010000;
//
//	TTInt nHeadSize = 0;
//	TTPBYTE pBuffer = pOutBuffer;
//	TTInt nArrays = pData[22];
//	TTInt nNum = 0;;
//
//	pData += 23;
//	if(nArrays)
//	{
//		for(nNum = 0; nNum < nArrays; nNum++)
//		{
//			unsigned char nal_type = 0;
//			nal_type = pData[0]&0x3F;
//			pData += 1;
//			switch(nal_type)
//			{
//			case 33://sps
//				{
//					TTUint32 nSPSNum = (pData[0] << 8)|pData[1];
//					pData += 2;
//					for(int i = 0; i < nSPSNum; i++)
//					{
//						memcpy (pBuffer + nHeadSize, &nNalWord, nNalLen);
//						nHeadSize += nNalLen;
//						TTInt nSPSLength = (pData[0] << 8)|pData[1];
//						pData += 2;
//						if(nSPSLength > (nInSzie - (pData - pInBuffer))){
//							nOutSize = 0;
//							return TTKErrNotSupported;
//						}
//
//						memcpy (pBuffer + nHeadSize, pData, nSPSLength);
//						nHeadSize += nSPSLength;
//						pData += nSPSLength;
//					}
//				}
//				break;
//			case 34://pps
//				{
//					TTUint32 nPPSNum = (pData[0] << 8) | pData[1];
//					pData += 2;
//					for(int i = 0; i < nPPSNum; i++)
//					{
//						memcpy (pBuffer + nHeadSize, &nNalWord, nNalLen);
//						nHeadSize += nNalLen;
//						TTInt nPPSLength = (pData[0] << 8)| pData[1];
//						pData += 2;
//						if(nPPSLength > (nInSzie - (pData - pInBuffer))){
//							nOutSize = 0;
//							return TTKErrNotSupported;
//						}
//						memcpy (pBuffer + nHeadSize, pData, nPPSLength);
//						nHeadSize += nPPSLength;
//						pData += nPPSLength;
//					}
//				}
//				break;
//			case 32: //vps
//				{
//					TTUint32 nVPSNum = (pData[0] << 8 )| pData[1] ;
//					pData += 2;
//					for(int i = 0; i < nVPSNum; i++)
//					{
//						memcpy (pBuffer + nHeadSize, &nNalWord, nNalLen);
//						nHeadSize += nNalLen;
//						TTInt nVPSLength = (pData[0] << 8 )|pData[1];
//						pData += 2;
//						if(nVPSLength > (nInSzie - (pData - pInBuffer))){
//							nOutSize = 0;
//							return TTKErrNotSupported;
//						}
//						memcpy (pBuffer + nHeadSize, pData, nVPSLength);
//						nHeadSize += nVPSLength;
//						pData += nVPSLength;
//					}
//				}
//				break;
//			default://just skip the data block
//				{
//					TTUint32 nSKP = (pData[0] << 8 )|pData[1];
//					pData += 2;
//					for(int i = 0; i < nSKP; i++)
//					{
//						TTInt nAKPLength = (pData[0] << 8) | pData[1];
//						if(nAKPLength > (nInSzie - (pData - pInBuffer))){
//							nOutSize = 0;
//							return TTKErrNotSupported;
//						}
//						pData += 2;
//						pData += nAKPLength;
//					}
//
//				}
//				break;
//			}
//		}
//	}
//
//	nOutSize = nHeadSize;
//
//	return TTKErrNone;
//}
//
//TTInt CTTMediaParser::ConvertAVCFrame(TTPBYTE pFrame, TTUint32 nSize, TTUint32& nFrameLen, TTInt& IsKeyFrame)	
//{
//	if (iNALLengthSize == 0)
//		return TTKErrNotSupported;
//
//	TTPBYTE  pBuffer = pFrame;
//	TTInt32	 nNalLen = 0;
//	TTInt32	 nNalType = 0;
//	
//	nFrameLen = 0;
//
//	TTUint32 nNalWord = 0x01000000;
//	if (iNALLengthSize == 3)
//		nNalWord = 0X010000;
//
//	if(iNALLengthSize < 3) 	{
//		if(iAVCSize < nSize + 512)
//		{
//			SAFE_DELETE_ARRAY(iAVCBuffer);
//			iAVCBuffer = new TTUint8[nSize + 512];
//			iAVCSize = nSize + 512;
//		}
//	} else {
//		nFrameLen = nSize;
//	}
//
//	TTInt i = 0;
//	TTInt leftSize = nSize;
//
//	while (pBuffer - pFrame + iNALLengthSize < nSize)
//	{
//		nNalLen = *pBuffer++;
//		for (i = 0; i < (int)iNALLengthSize - 1; i++)
//		{
//			nNalLen = nNalLen << 8;
//			nNalLen += *pBuffer++;
//		}
//
//		if(nNalType != 1 && nNalType != 5) {
//			nNalType = pBuffer[0]&0x0f;
//		}
//
//		leftSize -= iNALLengthSize;
//
//		if(nNalLen > leftSize)
//		{
//			nFrameLen = 0;
//			return TTKErrGeneral;
//		}
//
//		if (iNALLengthSize == 3 || iNALLengthSize == 4)
//		{
//			memcpy ((pBuffer - iNALLengthSize), &nNalWord, iNALLengthSize);
//		}
//		else
//		{
//			memcpy (iAVCBuffer + nFrameLen, &nNalWord, 4);
//			nFrameLen += 4;
//			memcpy (iAVCBuffer + nFrameLen, pBuffer, nNalLen);
//			nFrameLen += nNalLen;
//		}
//
//		leftSize -= nNalLen;
//		pBuffer += nNalLen;
//	}
//
//	if(nNalType == 5)
//		IsKeyFrame = 1;
//
//
//	return TTKErrNone;
//}
//end of file 
