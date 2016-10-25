/**
* File : TTExtAudioFileParser.cpp
* Created on : 2011-12-6
* Author : hu.cao
* Copyright : Copyright (c) 2011 Shuidushi Software Ltd. All rights reserved.
* Description : CTTExtAudioFileParser实现文件
*/

//  Include Files  
#include "TTMacrodef.h"
#include "TTExtAudioFileParser.h"
#include "TTExtAudioFileReader.h"
CTTExtAudioFileParser::CTTExtAudioFileParser(ITTDataReader& aDataReader, ITTMediaParserObserver& aObserver)
: CTTMediaParser(aDataReader, aObserver)
, iTotalPCMSize(0)
{
    memset(&iWavInfo, 0, sizeof(iWavInfo));    
}

CTTExtAudioFileParser::~CTTExtAudioFileParser()
{

}

TTInt CTTExtAudioFileParser::StreamCount()
{
	return 1;
}

TTInt CTTExtAudioFileParser::Parse(TTMediaInfo& aMediaInfo)
{
    TTAudioInfo* pAudioInfo = new TTAudioInfo();			
    pAudioInfo->iBitRate = 0;
    pAudioInfo->iChannel = ((CTTExtAudioFileReader*)(&iDataReader))->GetChannels();		
    pAudioInfo->iSampleRate = ((CTTExtAudioFileReader*)(&iDataReader))->GetSampleRate();
    pAudioInfo->iMediaTypeAudioCode = TTAudioInfo::KTTMediaTypeAudioCodeWAV;
    pAudioInfo->iStreamId = EMediaStreamIdAudioL;
    iWavInfo.iBitsPerSample = 16;
    iWavInfo.iSamplePerSec = pAudioInfo->iSampleRate;
    if (pAudioInfo->iSampleRate > HARDWARE_MAXSAMPLERATE) {
        iWavInfo.iSamplePerSec = HARDWARE_MAXSAMPLERATE;
    }
    iWavInfo.iFmtChannels = pAudioInfo->iChannel;
    pAudioInfo->iDecInfo = &iWavInfo;
    TTASSERT(aMediaInfo.iAudioInfoArray.Count() == 0);
    aMediaInfo.iAudioInfoArray.Append(pAudioInfo);  
    
    iTotalPCMSize = ((CTTExtAudioFileReader*)(&iDataReader))->TotalFrame() * sizeof(TTInt16) * pAudioInfo->iChannel;
    
    iFrameTime = KReadSizePerTime * 1000 / (pAudioInfo->iChannel * pAudioInfo->iSampleRate * sizeof(TTInt16));
    return TTKErrNone;
}

TTUint CTTExtAudioFileParser::MediaDuration(TTInt aStreamId)
{
    return ((CTTExtAudioFileReader*)(&iDataReader))->Duration();
}

TTUint CTTExtAudioFileParser::MediaDuration()
{
    return ((CTTExtAudioFileReader*)(&iDataReader))->Duration();
}

TTInt CTTExtAudioFileParser::GetFrameLocation(TTInt aStreamId, TTInt aFrmIdx, TTMediaFrameInfo& aFrameInfo)
{
    TTInt64 nTemp = aFrmIdx;
    nTemp *= KReadSizePerTime;
    TTUint nStartTime = nTemp * 1000/ ((iWavInfo.iFmtChannels * iWavInfo.iSamplePerSec * sizeof(TTInt16)));
    aFrameInfo.iFrameStartTime = nStartTime;
    TTInt64 nPCMRemainSize = iTotalPCMSize - KReadSizePerTime * aFrmIdx;
    if (nPCMRemainSize >= KReadSizePerTime)
    {
        aFrameInfo.iFrameSize = KReadSizePerTime;  
        //aFrameInfo.iFrameStopTime = nStartTime + iFrameTime;
        return nPCMRemainSize == KReadSizePerTime ? TTKErrEof : TTKErrNone;
    }
    else if (nPCMRemainSize > 0)
    {
        //aFrameInfo.iFrameStopTime = iFrameTime * nPCMRemainSize / KReadSizePerTime;
        aFrameInfo.iFrameSize = nPCMRemainSize;
        return TTKErrEof;
    }
    
    return TTKErrOverflow;
}

TTInt CTTExtAudioFileParser::GetFrameLocation(TTInt aStreamId, TTInt& aFrmIdx, TTUint64 aTime)
{   
    aFrmIdx = aTime / iFrameTime;
    
    return TTKErrNone;
}

TTInt CTTExtAudioFileParser::GetFrameTimeStamp(TTInt aStreamId, TTInt aFrmIdx, TTUint64 aPlayTime)
{
    return TTKErrNotSupported;
}

void CTTExtAudioFileParser::StartFrmPosScan()
{
    
}

TTBool CTTExtAudioFileParser::IsCreateFrameIdxComplete()
{
    return ETTFalse;
}

TTInt CTTExtAudioFileParser::SetParam(TTInt aType, TTPtr aParam)
{
    return TTKErrNone;
}

TTInt CTTExtAudioFileParser::GetParam(TTInt aType, TTPtr aParam)
{
    return TTKErrNone;
}

TTInt CTTExtAudioFileParser::SeekWithinFrmPosTab(TTInt aStreamId, TTInt aFrmIdx, TTMediaFrameInfo& aFrameInfo)
{
    return TTKErrNotFound;
};
//end of file 
