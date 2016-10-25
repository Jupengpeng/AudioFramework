/**
* File : TTPureDataParser.cpp
* Created on : 2011-9-7
* Author : hu.cao
* Copyright : Copyright (c) 2011 Shuidushi Software Ltd. All rights reserved.
* Description : CTTPureDataParser实现文件
*/

//  Include Files  
#include "TTMacrodef.h"
#include "TTFileReader.h"
#include "TTPureDataParser.h"
#include "TTIPodLibraryPCMOutputFormat.h"
#include "TTIPodLibraryFileReader.h"


CTTIPodLibraryParser::CTTIPodLibraryParser(ITTDataReader& aDataReader, ITTMediaParserObserver& aObserver)
: CTTMediaParser(aDataReader, aObserver)
{
}

CTTIPodLibraryParser::~CTTIPodLibraryParser()
{

}

TTInt CTTIPodLibraryParser::StreamCount()
{
	return 1;
}

TTInt CTTIPodLibraryParser::Parse(TTMediaInfo& aMediaInfo)
{
    TTAudioInfo* pAudioInfo = new TTAudioInfo();			
    pAudioInfo->iBitRate = 0;
    pAudioInfo->iChannel = ((CTTIPodLibraryFileReader*)(&iDataReader))->GetChannels();			
    pAudioInfo->iSampleRate = ((CTTIPodLibraryFileReader*)(&iDataReader))->GetSampleRate();
    pAudioInfo->iMediaTypeAudioCode = TTAudioInfo::KTTMediaTypeAudioCodeIPodLibrary;
    pAudioInfo->iStreamId = EMediaStreamIdAudioL;
    TTASSERT(aMediaInfo.iAudioInfoArray.Count() == 0);
    aMediaInfo.iAudioInfoArray.Append(pAudioInfo);    
    return TTKErrNone;
}

TTUint CTTIPodLibraryParser::MediaDuration()
{
    return ((CTTIPodLibraryFileReader*)(&iDataReader))->Duration();
}

TTUint CTTIPodLibraryParser::MediaDuration(TTInt aStreamId)
{
    return ((CTTIPodLibraryFileReader*)(&iDataReader))->Duration();
}

TTInt CTTIPodLibraryParser::GetFrameLocation(TTInt aStreamId, TTInt aFrmIdx, TTMediaFrameInfo& aFrameInfo)
{
    aFrameInfo.iFrameSize = sizeof(TTPureDataOutputFormat);    
    return TTKErrNone;
}

TTInt CTTIPodLibraryParser::GetFrameLocation(TTInt aStreamId, TTInt& aFrmIdx, TTUint64 aTime)
{    
    return TTKErrNotSupported;
}

TTInt CTTIPodLibraryParser::GetFrameTimeStamp(TTInt aStreamId, TTInt aFrmIdx, TTUint64 aPlayTime)
{
    return TTKErrNotSupported;
}

void CTTIPodLibraryParser::StartFrmPosScan()
{
    
}

TTBool CTTIPodLibraryParser::IsCreateFrameIdxComplete()
{
    return ETTFalse;
}

TTInt CTTIPodLibraryParser::SetParam(TTInt aType, TTPtr aParam)
{
    return 0;
}


TTInt CTTIPodLibraryParser::GetParam(TTInt aType, TTPtr aParam)
{
    return 0;
}

TTInt CTTIPodLibraryParser::SeekWithinFrmPosTab(TTInt aStreamId, TTInt aFrmIdx, TTMediaFrameInfo& aFrameInfo)
{
    return TTKErrNotFound;
};


//end of file
