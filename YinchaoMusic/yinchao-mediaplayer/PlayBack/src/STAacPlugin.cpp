#include "STAacPlugin.h"
#include "STLog.h"
#include "STUrlUtils.h"
#include "STDataReaderSelector.h"
#include "STHttpReader.h"
#include "STBufferConfig.h"

#define MAX_TRY_BYTE_COUNT (1024)
#define MAX_CHANNELS (64)
#define MAX_FRAME_SIZE (16*KILO)

STAacPlugin::STAacPlugin()
:iStreamCount(0)
,iCurStreamIdx(0)
,iLastRemainCount(0)
{
    memset(iByteOffset, 0, sizeof(iByteOffset));
    for(int i = 0; i < KStreamMaxCount; i++)
    { 
        iCurReadPos[i] = 0;
        iHDecoder[i] = 0;
        iDataReader[i] = NULL;
    }

    iReadBuffer = new STUint8[FAAD_MIN_STREAMSIZE*MAX_CHANNELS];
    iLastRemainBuffer = new STUint8[MAX_FRAME_SIZE];
}

STAacPlugin::~STAacPlugin()
{
    SAFE_DELETE(iReadBuffer);
    SAFE_DELETE(iLastRemainBuffer);
}

STInt STAacPlugin::InitPlugin(const STChar* aAacFilePath, const STChar* aParams)
{
    STASSERT(aAacFilePath != NULL);

    STUint32 nSamplerate = 0;
    STUint8 nChannels = 0;

    memset(iBackgroundName, 0x00, KMaxPathLength);
    memset(iOriginName, 0x00, KMaxPathLength);

    if(STKErrNone == STUrlUtils::GetParam(aParams, ParamKeyBackgroundSourceName, iBackgroundName, ParamKeySeparator, KMaxPathLength - 1))
    {
    	sprintf(iTempFullNameBuffer, "%s/%s", aAacFilePath, iBackgroundName);
    }
    else
    {
    	strcpy(iTempFullNameBuffer, aAacFilePath);
    }

	iDataReader[KStreamBackgourndIdx] = STDataReaderSelector::SelectDataReader(iTempFullNameBuffer);

	if (iDataReader[KStreamBackgourndIdx]->Id() == ISTDataReaderItf::EDataReaderIdHttp)
	{
		//暂时使用一下iOriginName
		if (STKErrNone == STUrlUtils::GetParam(aParams, ParamKeyHttpCacheName, iOriginName, ParamKeySeparator, KMaxPathLength - 1))
		{
			((STHttpReader*)iDataReader[KStreamBackgourndIdx])->SetCachePath(iOriginName);
		}
		else
		{
			STASSERT(ESTFalse);
		}

		memset(iOriginName, 0, sizeof(iOriginName));
	}

	if (STKErrNone != iDataReader[KStreamBackgourndIdx]->Open(iTempFullNameBuffer))
	{
		STLOGE("STAacPlugin::InitPlugin >> Error opening file: %s\n", iTempFullNameBuffer);
		return STKErrNotFound;
	}
	iStreamCount++;
	iCurStreamIdx = KStreamBackgourndIdx;

    if(STKErrNone == STUrlUtils::GetParam(aParams, ParamKeyOriginSourceName, iOriginName, ParamKeySeparator, KMaxPathLength - 1))
    {
        sprintf(iTempFullNameBuffer, "%s/%s", aAacFilePath, iOriginName);
        iDataReader[KStreamOriginIdx] = STDataReaderSelector::SelectDataReader(iTempFullNameBuffer);
        STASSERT(iDataReader[KStreamOriginIdx]->Id() != ISTDataReaderItf::EDataReaderIdHttp);
        if (STKErrNone != iDataReader[KStreamOriginIdx]->Open(iTempFullNameBuffer))
        {
            STLOGE("STAacPlugin::InitPlugin >> Error opening file: %s\n", iTempFullNameBuffer);
            return STKErrNotFound;
        }
        iStreamCount++;
    }

    for(int i = 0; i < iStreamCount; i++)
    {
        iCurReadPos[i] = 0;

        STInt nRead = iDataReader[i]->Read(iReadBuffer, iCurReadPos[i], FAAD_MIN_STREAMSIZE*MAX_CHANNELS);

        if (!memcmp(&iReadBuffer[4],"ftyp", 4))
        {
            STLOGE("STAacPlugin::InitPlugin >> Can not support MP4.\n");
            return STKErrNotSupported;
        }

        iHDecoder[i] = NeAACDecOpen();

        if ((nRead = NeAACDecInit(iHDecoder[i], iReadBuffer, nRead, &nSamplerate, &nChannels)) < 0)
        {
            STLOGE("STAacPlugin::InitPlugin >> Error initializing decoder library.\n");
            NeAACDecClose(iHDecoder[i]);
            iHDecoder[i] = 0;
            return STKErrUnknown;
        }

        iCurReadPos[i]  += nRead;

        const STPointerArray<STAudioInfo>& tAudioStreamArray = iMediaInfo.GetAudioStreamArray();
        if (tAudioStreamArray.Count() > 0)
        {
        	STAudioInfo* pAudioInfo = tAudioStreamArray[tAudioStreamArray.Count() - 1];
        	if (pAudioInfo->GetSampleRate() != nSamplerate || pAudioInfo->GetChannels() != nChannels)
        	{
        		STASSERT(ESTFalse);
        	}
        }

        iMediaInfo.AddAudioStream(new STAudioInfo(nSamplerate, nChannels, i));
    }

    return STKErrNone;
}

void STAacPlugin::ResetPlugin()
{

}

void STAacPlugin::UnInitPlugin()
{
    STBasePlugin::UnInitPlugin();

    for(int i = 0; i < iStreamCount; i++)
	{
		iDataReader[i]->Close();
		iCurReadPos[i] = 0;
		NeAACDecClose(iHDecoder[i]);
		iHDecoder[i] = 0;
	}

	memset(iBackgroundName, 0x00, KMaxPathLength);
	memset(iOriginName, 0x00, KMaxPathLength);

	iMediaInfo.Reset();
}

STInt STAacPlugin::StartReading()
{
    if(iReadBuffer != NULL)
    {
        iReadStatus = ESTReadStatusReading;
        return STKErrNone;
    }
    return STKErrNoMemory;
}

STInt STAacPlugin::Read(STSampleBuffer* aBuffer)
{
    STASSERT(aBuffer != NULL);

    STInt nErr = STKErrUnknown;

    if (iReadStatus != ESTReadStatusReading)
    {
		STLOGE("STAacPlugin::Read() >> iReadStatus != ESTReadStatusReading");
        return STKErrNotReady;
    }

    nErr = Decode(iCurStreamIdx, aBuffer);
    aBuffer->SetByteOffset(iByteOffset[iCurStreamIdx]);
    iByteOffset[iCurStreamIdx] += aBuffer->Size();
	aBuffer->SetStreamIndex(iCurStreamIdx);

	iCurStreamIdx++;
	if (iCurStreamIdx >= iStreamCount)
	{
		iCurStreamIdx = 0;
	}

    return nErr;
} 

void STAacPlugin::Abort()
{
	for (STInt i = 0; i < KStreamMaxCount; i++)
	{
		if (iDataReader[i] != NULL)
		{
			iDataReader[i]->Abort();
		}
	}
}

void STAacPlugin::Seek(STUint aPos)
{
	STInt nSampleRate = iMediaInfo.GetAudioStreamArray()[0]->GetSampleRate();
	STInt nChannels = iMediaInfo.GetAudioStreamArray()[0]->GetChannels();

	for (STInt i = 0; i < iStreamCount; i++)
	{
		STInt64 tmp = aPos;
		tmp *= nSampleRate * nChannels * sizeof(STInt16);
		int nByteOffset = tmp / 1000;
		tmp = iCurReadPos[i];
		tmp *= nByteOffset;
		iCurReadPos[i] = tmp / iByteOffset[0];

		iByteOffset[i] = nByteOffset;

		NeAACDecPostSeekReset(iHDecoder[i], nByteOffset / 1024 / sizeof(STInt16) / nChannels);
	}
}

STInt STAacPlugin::Decode(STUint aIdx, STSampleBuffer* aBuffer)
{
    STASSERT(aIdx < KStreamMaxCount);
    STASSERT(aBuffer != NULL);

    STUint8 *                   pBuffer = NULL;
    STUint8 *                   pSampleBuffer = NULL;
    STUint8 *                   pDesBufer = aBuffer->Ptr();
    STInt                       nDesSize = aBuffer->Size();
    STInt                       nRead = 0;
    STInt                       nDataCnt = 0;
    NeAACDecFrameInfo           tFrameInfo;	
	
    //处理上次解码后未送出的数据
    if(iLastRemainCount > 0)
    {
    	if (nDesSize <= iLastRemainCount)
    	{
    		memcpy(pDesBufer, iLastRemainBuffer, nDesSize);
    		iLastRemainBuffer -= nDesSize;
    		if (iLastRemainBuffer > 0)
    		{
    			memmove(iLastRemainBuffer, iLastRemainBuffer + nDesSize, iLastRemainCount);
    		}
    		iByteOffset[aIdx] += nDesSize;
    		return STKErrNone;
    	}
    	else
    	{
    		memcpy(pDesBufer, iLastRemainBuffer, iLastRemainCount);
    		pDesBufer += iLastRemainCount;
    		nDesSize -= iLastRemainCount;
    		iByteOffset[aIdx] += iLastRemainCount;
    		iLastRemainCount = 0;
    	}
    }

    STInt nTryByteCount = 0;
    while((nRead = iDataReader[aIdx]->Read(iReadBuffer, iCurReadPos[aIdx], FAAD_MIN_STREAMSIZE*MAX_CHANNELS)) > 0)
    {
        pBuffer = iReadBuffer;
        while(nRead > 0)
        {
            pSampleBuffer = (STUint8 *)NeAACDecDecode(iHDecoder[aIdx], &tFrameInfo, pBuffer, nRead);
            if (tFrameInfo.error > 0)
            {
                STLOGE("STAacPlugin::Decode hh >>%d, %s\n", nRead, NeAACDecGetErrorMessage(tFrameInfo.error));
                pBuffer++;
                nRead--;
                nTryByteCount++;
                if (nTryByteCount < MAX_TRY_BYTE_COUNT)
                {
                	continue;
                }

                iReadStatus = ESTReadStatusReadErr;
                return STKErrUnknown;
            }

            nTryByteCount = 0;
            nDataCnt = tFrameInfo.samples * tFrameInfo.channels;
			
            //如果传入buffer已经小于已解码数据
            if(nDesSize < nDataCnt)
            {
                memcpy(pDesBufer, pSampleBuffer, nDesSize);
                iLastRemainCount = nDataCnt - nDesSize;
                memcpy(iLastRemainBuffer, pSampleBuffer + nDesSize, iLastRemainCount);
                iByteOffset[aIdx] += nDesSize;
                iCurReadPos[aIdx] += tFrameInfo.bytesconsumed;
                return STKErrNone;
            }
            else if(nDesSize == nDataCnt)
            {
                memcpy(pDesBufer, pSampleBuffer, nDataCnt);
                iCurReadPos[aIdx] += tFrameInfo.bytesconsumed;
                return STKErrNone;
            }

            memcpy(pDesBufer, pSampleBuffer, nDataCnt);
            nDesSize -= nDataCnt;
            pDesBufer += nDataCnt;
            nRead -= tFrameInfo.bytesconsumed;
            pBuffer += tFrameInfo.bytesconsumed;
            iCurReadPos[aIdx] += tFrameInfo.bytesconsumed;
        }
    }

    if(nDesSize > 0)
    {
        memset(pDesBufer, 0x00, nDesSize);
    }

    if(nDesSize == aBuffer->Size() && aIdx == iStreamCount - 1)
    {
        STLOGI("STAacPlugin::Decode >> Decode complete.\n");
        iReadStatus = ESTReadStatusComplete;
    }

    return STKErrNone;
}

STInt STAacPlugin::Switch2Stream(const STChar *aStreamName)
{
	STASSERT(NULL != aStreamName);

	if(!strcmp(iBackgroundName, aStreamName))
	{
		STLOGI("STAacPlugin::Switch2Stream >> switch to Background.\n");
		return KStreamBackgourndIdx;
	}	
    else if(!strcmp(iOriginName, aStreamName))
	{
		STLOGI("STAacPlugin::Switch2Stream >> switch to Origin.\n");
		return KStreamOriginIdx;
	}
    else
    {
        STLOGE("STAacPlugin::Switch2Stream >> Unknown Stream Name.\n");
    }

	return STKErrNotFound;
}

const STChar* STAacPlugin::GetStreamName(STInt aIndex)
{
	if (KStreamBackgourndIdx == aIndex)
	{
		return iBackgroundName;
	}
	else if (KStreamOriginIdx == aIndex)
	{
		return iOriginName;
	}

	return NULL;
}


STInt	STAacPlugin::GetDownloadPercent()
{
	STUint pos = -1;
	if(iDataReader[KStreamBackgourndIdx] != NULL)
	{
		if (iDataReader[KStreamBackgourndIdx]->Id() == ISTDataReaderItf::EDataReaderIdHttp)
		{
			pos = ((STHttpReader*)iDataReader[KStreamBackgourndIdx])->GetDownloadPercent(); //必须STHttpReader类才能调
		}
		else if(iDataReader[KStreamBackgourndIdx]->Id() == ISTDataReaderItf::EDataReaderIdLoacal)
		{
			STLOGE("STAacPlugin::GetDownloadPercent EDataReaderIdLoacal" );
			pos = 100;//本地直接返回百分百
		}
	}
	return pos;
}