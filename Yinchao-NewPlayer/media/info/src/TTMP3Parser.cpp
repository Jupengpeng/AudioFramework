//INCLUDS
#include "TTTypedef.h"
#include "TTID3Tag.h"
#include "TTAPETag.h"
#include "TTMP3Parser.h"
#include "TTDataReaderItf.h"
#include "TTFileReader.h"
#include "TTMacrodef.h"
#include "TTMediainfoDef.h"
#include "TTLog.h"

CTTMP3Parser::CTTMP3Parser(ITTDataReader& aDataReader, ITTMediaParserObserver& aObserver) 
: CTTMediaParser(aDataReader, aObserver)
, iMP3Header(NULL)
, iAVEFrameSize(0)
{
}

CTTMP3Parser::~CTTMP3Parser()
{
    SAFE_DELETE(iMP3Header);
}

TTUint CTTMP3Parser::MediaDuration(TTInt aStreamId)
{
	TTASSERT(aStreamId == EMediaStreamIdAudioL);

	TTUint nDuration = 0;

	if (iFrmPosTabDone)
	{
		TTInt64 nTemp = iFrmPosTabCurIndex;
		nTemp *= iFrameTime;

		nDuration = (TTUint)(nTemp / 1000);
	}
	else
	{
		TTInt64 nDuration64 = 0;
		TTInt64 nDuration320 = 0;
		if((iFirstFrmInfo.nBitRate != 0) && (iRawDataBegin < iRawDataEnd)) {
			nDuration64 = (TTInt64)(((iRawDataEnd - iRawDataBegin) / (iFirstFrmInfo.nBitRate / 8)) * 1000);
			nDuration320 = (TTInt64)(((iRawDataEnd - iRawDataBegin) / (320000 / 8)) * 1000);
		}

		switch(iMP3Header->Type())
		{
		case MP3CBR_HEADER:
			{
				TTASSERT((iFirstFrmInfo.nBitRate != 0) && (iRawDataBegin < iRawDataEnd));
				nDuration = (TTUint)(((iRawDataEnd - iRawDataBegin) / (iFirstFrmInfo.nBitRate / 8)) * 1000);				
			}
			break;

		case MP3XING_HEADER:
			{
				CTTXingHeader* pHeader = static_cast<CTTXingHeader*>(iMP3Header);
				nDuration = ((pHeader->m_dwFrames * iFrameTime) / 1000);

				if(nDuration64) {
					if(nDuration > nDuration64 + 20000 || pHeader->m_dwBytes > iRawDataEnd - iRawDataBegin + 1024*100) {
						nDuration = nDuration64;
						pHeader->m_dwFrames = nDuration64*1000/iFrameTime;
						pHeader->m_dwBytes = iRawDataEnd - iRawDataBegin;
					}

					if(nDuration < nDuration320 || pHeader->m_dwBytes + 1024*500 < iRawDataEnd - iRawDataBegin) {
						nDuration = nDuration64;
						pHeader->m_dwFrames = nDuration64*1000/iFrameTime;
						pHeader->m_dwBytes = iRawDataEnd - iRawDataBegin;
					}
				}
			}
			break;

		case MP3VBRI_HEADER:
			{
				CTTVbriHeader* pHeader = static_cast<CTTVbriHeader*>(iMP3Header);
				nDuration = ((pHeader->m_dwFrames * iFrameTime) / 1000);

				if(nDuration64) {
					if(nDuration > nDuration64 + 20000 || pHeader->m_dwBytes > iRawDataEnd - iRawDataBegin + 1024*100) {
						nDuration = nDuration64;
						pHeader->m_dwFrames = nDuration64*1000/iFrameTime;
						pHeader->m_dwBytes = iRawDataEnd - iRawDataBegin;
					}

					if(nDuration < nDuration320 || pHeader->m_dwBytes + 1024*500 < iRawDataEnd - iRawDataBegin) {
						nDuration = nDuration64;
						pHeader->m_dwFrames = nDuration64*1000/iFrameTime;
						pHeader->m_dwBytes = iRawDataEnd - iRawDataBegin;
					}
				}
			}
			break;

		default:
			break;
		}
	}

	//LOGI("MP3Parser::MediaDuration. duration = %d, bitrate = %d, headertype = %d", nDuration, iFirstFrmInfo.nBitRate, iMP3Header->Type());
	return nDuration;
}

TTInt CTTMP3Parser::RawDataEnd()
{
	TTInt nID3v1TagSize = ID3v1TagSize(iDataReader);
	TTInt nAPETagSize = APETagSize(iDataReader);

	return iDataReader.Size() - nID3v1TagSize - nAPETagSize;
}

TTInt CTTMP3Parser::Parse(TTMediaInfo& aMediaInfo)
{
	TTInt nReadTagSize = 0;
	TTInt nReadPos = 0;

	do
	{
		nReadTagSize = ID3v2TagSize(iDataReader, nReadPos);
		nReadPos += nReadTagSize;
	} while (nReadTagSize > 0);

	TTInt nMaxFirstFrmOffset = KMaxFirstFrmOffset + nReadPos;

	iParserMediaInfoRef = &aMediaInfo;

	iRawDataEnd = RawDataEnd();

	TTFrmSyncResult tResult;
	TTInt nOffSet = 0;
	TTInt nProcessedSize = 0;
	TTInt nErr = TTKErrFilParseOutofRange;
	TTInt nCount = 0;

	do
	{
		if (nReadPos >= nMaxFirstFrmOffset)
		{
			LOGI("CTTMP3Parser::Parse. ReadPos >= nMaxFirstFrmOffset");
			break;
		}
		else
		{
			tResult = FrameSyncWithPos(nReadPos, nOffSet, nProcessedSize, iFirstFrmInfo, ETTTrue);
			LOGI("TTMP3Parser::FrameSyncWithPos : %d", tResult);
			if ((EFrmSyncComplete == tResult) || (EFrmSyncEofComplete == tResult))
			{
				iFrameTime = TTUint(((TTInt64)iFirstFrmInfo.nSamplesPerFrame * 1000000) / iFirstFrmInfo.nSampleRate);
				iAVEFrameSize = iFirstFrmInfo.nFrameSize;
				iRawDataBegin = nReadPos + nOffSet;

				TTAudioInfo* pAudioInfo = new TTAudioInfo();			
				pAudioInfo->iBitRate = iFirstFrmInfo.nBitRate;
				pAudioInfo->iChannel = iFirstFrmInfo.nChannels;			
				pAudioInfo->iSampleRate = iFirstFrmInfo.nSampleRate;
				pAudioInfo->iMediaTypeAudioCode = TTAudioInfo::KTTMediaTypeAudioCodeMP3;
				pAudioInfo->iStreamId = EMediaStreamIdAudioL;
				TTASSERT(aMediaInfo.iAudioInfoArray.Count() == 0);
				aMediaInfo.iAudioInfoArray.Append(pAudioInfo);
				iStreamAudioCount++;

				nErr = TTKErrNone;
				break;
			}

			nReadPos += nProcessedSize;

			if(nProcessedSize == 0) {
				if(iDataReader.Id() == ITTDataReader::ETTDataReaderIdFile || tResult == EFrmSyncReadErr)
					nCount++;

				if (iDataReader.Id() == ITTDataReader::ETTDataReaderIdHttp || iDataReader.Id() == ITTDataReader::ETTDataReaderIdBuffer) {
					mSemaphore.Wait(5);
				}
			} else {
				nCount = 0;
			}

			if(nCount > 100)
				break;
		}

	} while ((tResult != EFrmSyncReadErr) && (tResult != EFrmSyncEofInComplete));

	if (tResult == EFrmSyncReadErr)
		nErr = TTKErrSyncReadErr;
	else if(tResult == EFrmSyncEofInComplete)
		nErr = TTKErrSyncEofInComplete;
 
	LOGI("TTMP3Parser::Parse return: %d", nErr);
	return nErr;
}

void CTTMP3Parser::StartFrmPosScan()
{
	if(iDataReader.Size() > 1024*1024*15)
		return;

	CTTMediaParser::StartFrmPosScan();
}

TTFrmSyncResult CTTMP3Parser::FrameSyncWithPos(TTInt aReadPos, TTInt &aOffSet, TTInt &aProcessedSize, MP3_FRAME_INFO& aFrameInfo, TTBool aCheckNextFrameHeader)
{
	TTInt nTotalOffset = 0;
    TTUint8 *pData = NULL;
	TTInt nReadSize = KSyncReadSize;	
	TTBool bFoundAnySync = ETTFalse;
	TTFrmSyncResult tFrameSyncResult = EFrmSyncReadErr;

	aProcessedSize = 0; 
	
    TTReadResult nReadReturnStatus = ReadStreamData(aReadPos, pData, nReadSize);
    switch(nReadReturnStatus)
    {
    case EReadEof:
    case EReadOK:
		{
			MP3_FRAME_INFO tFrameInfo;
			TTInt nRemainSize = nReadSize;
			TTBool bFound = ETTFalse;

			while (!bFound)
			{
				TTInt nOffset = 0;
				if (!CTTMP3Header::MP3SyncFrameHeader(pData, nRemainSize, nOffset, tFrameInfo))
				{
					break;
				}

				bFoundAnySync = ETTTrue;
				bFound = ETTTrue;
				pData += nOffset;
				nTotalOffset += nOffset;
				nRemainSize -= nOffset;

				if (aCheckNextFrameHeader && nRemainSize > (KMP3MinFrameSize + tFrameInfo.nFrameSize))
				{
					MP3_HEADER mh;
					bFound = CTTMP3Header::MP3CheckHeader(pData + tFrameInfo.nFrameSize, mh);
					if (!bFound)
					{
						pData++;
						nTotalOffset++;
						nRemainSize--;
					}	
				}
			}

			aProcessedSize = bFoundAnySync ? nTotalOffset : nReadSize;

            if (bFound)
			{
				if (iMP3Header == NULL)
				{
					iMP3Header = MP3ParseFrameHeader(pData, nRemainSize, tFrameInfo);
					if (iMP3Header == NULL)
						return EFrmSyncInComplete;
				}
				aFrameInfo = tFrameInfo;
				aOffSet = nTotalOffset;
			}
  
			TTBool bEof = EReadEof == nReadReturnStatus;

			tFrameSyncResult = (TTFrmSyncResult)((bFound << SYNC_COMPLETED_SHIFT) | (bEof << SYNC_EOF_SHIFT));
        }
        break;

	case EReadUnderflow:
		tFrameSyncResult = EFrmSyncInComplete;
		break;
	case EReadOverflow:
    case EReadErr:
    default:
        break;
    }

	//LOGI("CTTMP3Parser::FrameSyncWithPos return %d", tFrameSyncResult);
	return tFrameSyncResult;
}

void CTTMP3Parser::ParseFrmPos(const TTUint8 *aData, TTInt aParserSize)
{
	if ((aData != NULL) && (aParserSize >= 4))
	{
        MP3_HEADER mh;
        MP3_FRAME_INFO mi;
        TTUint *pFrmPosTab = iFrmPosTab;
        TTInt nPos = iFrmPosTabCurOffset;

        do
		{
            if (CTTMP3Header::MP3CheckHeader(aData, mh) && CTTMP3Header::MP3ParseFrame(mh, mi) && (mi.nFrameSize > 0) && (mi.nFrameSize < KMP3MaxFrameSize))
            {
				*(pFrmPosTab + (iFrmPosTabCurIndex++)) = nPos;
				nPos         += mi.nFrameSize;
				aData        += mi.nFrameSize;
				aParserSize  -= mi.nFrameSize;
			}
			else
			{
				nPos        ++;
				aData       ++;
				aParserSize --;
			}
        } while ((aParserSize >= 4) && (iFrmPosTabCurIndex < iFrmTabSize));

        iFrmPosTabCurOffset = nPos;

        if (iFrmPosTabCurIndex >= iFrmTabSize)
        {
			FrmIdxTabReAlloc();
        }
	}
}

TTInt CTTMP3Parser::SeekWithinFrmPosTab(TTInt aStreamId, TTInt aFrmIdx, TTMediaFrameInfo& aFrameInfo)
{
	TTInt nErr = TTKErrNotFound;
	TTInt64& nFramePos = aFrameInfo.iFrameLocation;
	TTInt& nFrameSize = aFrameInfo.iFrameSize;

    TTUint *pFrmPosTab = iFrmPosTab + aFrmIdx;

	if (aFrmIdx < (iFrmPosTabCurIndex - 1))
    {
        nFramePos = *pFrmPosTab++;
        nFrameSize = *pFrmPosTab - nFramePos;
		nErr = (nFrameSize > KMP3MaxFrameSize) ? TTKErrTooBig : TTKErrNone;
    }
    else if (iFrmPosTabDone && aFrmIdx == iFrmPosTabCurIndex - 1)
    {
		nFramePos = *pFrmPosTab;

        TTInt nOffset = 0;
        TTInt nProcessSize = 0;
        MP3_FRAME_INFO tFrameInfo;

        FrameSyncWithPos(nFramePos, nOffset, nProcessSize, tFrameInfo);

		nFramePos += nOffset;
		nFrameSize = tFrameInfo.nFrameSize;	

		nErr = TTKErrEof;		
    }

	if ((nErr == TTKErrNone) || (nErr == TTKErrEof))
	{
		UpdateFrameInfo(aFrameInfo, aFrmIdx);
	}

	return nErr;
}

TTInt CTTMP3Parser::SeekWithoutFrmPosTab(TTInt aStreamId, TTInt aFrmIdx, TTMediaFrameInfo& aFrameInfo)
{
	TTInt nErr = CTTMediaParser::SeekWithoutFrmPosTab(aStreamId, aFrmIdx, aFrameInfo);

	if ((nErr == TTKErrNone) || (nErr == TTKErrEof))
	{
		UpdateFrameInfo(aFrameInfo, aFrmIdx);
	}

	return nErr;
}

void CTTMP3Parser::UpdateFrameInfo(TTMediaFrameInfo& aFrameInfo,  TTInt aFrameIdx)
{
	TTUint64 nTmp64 = aFrameIdx;
	aFrameInfo.iFrameStartTime = (TTUint)(nTmp64 * iFrameTime / 1000);
}

TTInt CTTMP3Parser::FramePosition(TTInt aFrameIdx)
{
	TTInt nPos = -1;

	switch(iMP3Header->Type())
	{
	case MP3CBR_HEADER:
		{
			nPos = iRawDataBegin + aFrameIdx * iAVEFrameSize;
		}
		break;

	case MP3XING_HEADER:
		{
			CTTXingHeader* pXingHeader = static_cast<CTTXingHeader*>(iMP3Header);
			TTInt nTotalFrames = pXingHeader->m_dwFrames;
			TTInt nTotalBytes = 0;
			if (pXingHeader->m_dwFlags & XING_BYTES)
			{
				nTotalBytes = pXingHeader->m_dwBytes;
			}

			if (nTotalFrames > 0 && nTotalBytes > 0)
			{
				if(pXingHeader->m_dwFlags & XING_TOC)
				{
					if(aFrameIdx >= nTotalFrames)
					{
						nPos = iRawDataEnd;
					}
					else if (aFrameIdx < nTotalFrames)
					{
						TTInt64 nTemp = (TTInt64)aFrameIdx;
						nTemp *=  100;
						TTInt nTableIndex =  (TTInt)(nTemp / nTotalFrames);
						if(nTableIndex == 100) nTableIndex = 99;
						nTemp = (TTInt64)nTotalBytes;
						nTemp *= pXingHeader->m_arToc[nTableIndex];
						nPos = (TTInt)(nTemp >> 8);
					}
				}
				else
				{
					TTInt64 nStreamSize = nTotalBytes;
					nPos = (TTInt)(aFrameIdx * nStreamSize / nTotalFrames);
				}
			}
			else
			{
				if (iFrmPosTabCurIndex == 0)
				{
                    //online xing header file reach here
					TTInt nTotalDataSize = iRawDataEnd - iRawDataBegin ;
					if (nTotalDataSize > 0 && nTotalFrames > 0)
					{
						float fPos = (float)nTotalDataSize * aFrameIdx / nTotalFrames;
						nPos = iRawDataBegin + fPos;
					}
				}
				else
				{
					if (iFrmPosTabCurIndex > 0)
					{
						TTUint *pFrmPosTab = iFrmPosTab;
						TTInt nFramesParsed = iFrmPosTabCurIndex - 1;
						iAVEFrameSize = (*(pFrmPosTab + nFramesParsed) - iRawDataBegin) / nFramesParsed;
					}
					nPos = iRawDataBegin + iAVEFrameSize * aFrameIdx;
				}
			}
		}
		break;

	case MP3VBRI_HEADER:
		{
			CTTVbriHeader* pVBRIHeader = static_cast<CTTVbriHeader*>(iMP3Header);
			TTInt nTotalFrames = pVBRIHeader->m_dwFrames;
			
			if(nTotalFrames > 0 && pVBRIHeader->m_wTableSize > 0)
			{
				if(aFrameIdx >= nTotalFrames)
				{
					nPos = iRawDataEnd;
				}
				else if(aFrameIdx < nTotalFrames)
				{
					TTInt64 nTemp = (TTInt64)aFrameIdx;
					nTemp *=  pVBRIHeader->m_wTableSize;
					TTInt nTableIndex =  (TTInt)(nTemp / nTotalFrames);
					nPos = pVBRIHeader->m_pnTable[nTableIndex];
				}
			}
			else
			{
				if (iFrmPosTabCurIndex > 0)
				{
					TTUint *pFrmPosTab = iFrmPosTab;
					TTInt nFramesParsed = iFrmPosTabCurIndex - 1;
					iAVEFrameSize = (*(pFrmPosTab + nFramesParsed) - iRawDataBegin) / nFramesParsed;
				}
				nPos = iRawDataBegin + iAVEFrameSize * aFrameIdx;
			}
		}
		break;

	default:
		break;
	}
    
	return nPos;
}

TTInt CTTMP3Parser::SeekWithIdx(TTInt aStreamId, TTInt aFrmIdx, TTInt64 &aFrmPos, TTInt &aFrmSize)
{
	TTInt nPos = FramePosition(aFrmIdx);

	if(nPos == -1) return TTKErrNotFound;

    CLIP(nPos, iRawDataBegin, iRawDataEnd);

    return SeekWithPos(aStreamId, nPos, aFrmPos, aFrmSize);
}

TTInt CTTMP3Parser::SeekWithPos(TTInt aStreamId, TTInt64 aPos, TTInt64 &aFrmPos, TTInt &aFrmSize)
{
    TTInt nOffSet;
    TTInt nReadSize;
    MP3_FRAME_INFO tMP3FrameInfo;

	TTInt nErr = TTKErrNotFound;

	TTFrmSyncResult tResult = FrameSyncWithPos(aPos, nOffSet, nReadSize, tMP3FrameInfo);

	if ( EFrmSyncInComplete == tResult)
	{
		return TTKErrUnderflow;
	}
	else if (EFrmSyncReadErr == tResult)
	{
		return TTKErrOverflow;
	}

    if (SYNC_COMPLETED(tResult))
    {
        aFrmSize = tMP3FrameInfo.nFrameSize;
        aFrmPos = aPos + nOffSet;
		nErr = TTKErrNone;
	}

	if (SYNC_EOF(tResult))
	{
        if (SYNC_COMPLETED(tResult)) {
        }
        else{
            aFrmPos = iRawDataEnd;
            aFrmSize = 0;
            nErr = TTKErrEof;
        }
	}
	return nErr;
}

TTInt CTTMP3Parser::GetFrameLocation(TTInt aStreamId, TTInt& aFrmIdx, TTUint64 aTime)
{
	TTASSERT(iFrameTime > 0);
	TTInt64 nTemp = aTime;
	nTemp *= 1000;
	aFrmIdx = (nTemp + iFrameTime / 2) / iFrameTime;
	return TTKErrNone;
}
