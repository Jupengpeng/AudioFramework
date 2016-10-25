#include "PureDecodeEntity.h"
#include "TTLog.h"
#ifdef __TT_OS_IOS__
#include "TTIPodLibraryFileReader.h"
#endif

#define MAX_SRC_LENGTH 10*1024
#define MAX_DST_LENGTH 80*1024
#define MAX_DECPDE_DATA 2560*1024
#define MAX_DURATION    20000 //20s 

#include<stdlib.h>
#include<time.h>

#include "RIFF.h"
#define  FORMAT_TAG_IEEE_FLOAT 3

CTTPureDecodeEntity* CTTPureDecodeEntity::iDecodeEntity = NULL;

CTTPureDecodeEntity* CTTPureDecodeEntity::Instance()
{
	if (iDecodeEntity == NULL)
	{
		iDecodeEntity = new CTTPureDecodeEntity();
	}

	TTASSERT(iDecodeEntity != NULL);
	return iDecodeEntity;
}

void CTTPureDecodeEntity::Release()
{
	SAFE_DELETE(iDecodeEntity);
}

CTTPureDecodeEntity::CTTPureDecodeEntity()
: mAudioDecSize(0)
, iSleepDataSize(6)
, iWaitIntervalUs(3)
, iCurSrcMediaFormat(0)
, iDecodeDataSize(MAX_DECPDE_DATA)
, iCopyPos(0)
, iCancel(ETTFalse)
, iDecodrStartPos(0)
, mAudioStepSize(0)
, mDoSampleBits(0)
, mDoDownMix(0)
, mDoSampleRateConv(0)
, mResampleObj(NULL)
{
	observer.pObserver = NULL;
	observer.pUserData = NULL;
	iPluginManager = new CTTAudioPluginManager();
	TTASSERT(iPluginManager != NULL);

	iMediaInfoProxy =  new CTTMediaInfoProxy(&observer);

	iSemaphore.Create();

	iDecodeDataPtr = (TTUint8*)malloc(iDecodeDataSize);

	aDstBuffer = new TTBuffer; 
	aDstBuffer->nSize = KSinkBufferSize;
	aDstBuffer->pBuffer = (TTPBYTE)malloc(KSinkBufferSize);
	//aDstBuffer->nFlag = TT_FLAG_BUFFER_SEEKING;

	memset(&mSrcBuffer, 0, sizeof(mSrcBuffer));
	mSrcBuffer.nFlag = TT_FLAG_BUFFER_SEEKING;
}

CTTPureDecodeEntity::~CTTPureDecodeEntity()
{
	free(iDecodeDataPtr); 
	iPluginManager->uninitPlugin();
	delete iMediaInfoProxy;
	delete iPluginManager;
	free(aDstBuffer->pBuffer);
	delete  aDstBuffer;
	SAFE_DELETE(mResampleObj);

	iSemaphore.Destroy();
}


TTInt CTTPureDecodeEntity::updateParam(TTAudioInfo* pCurAudioInfo)
{
	TTInt32 nErr = TTKErrNone;
	TTWAVFormat		 mWAVFormat;
	//if(pCurAudioInfo->iMediaTypeAudioCode == TTAudioInfo::KTTMediaTypeAudioCodeAAC) {
	//	TTAACFRAMETYPE nType = TTAAC_ADTS; 
	//	if(pCurAudioInfo->iFourCC == MAKEFOURCC('A','D','T','S')) {
	//		nType = TTAAC_ADTS;
	//		iPluginManager->setParam(TT_AACDEC_PID_FRAMETYPE, &nType);
	//	}else if(pCurAudioInfo->iFourCC == MAKEFOURCC('A','D','I','F')) {
	//		nType = TTAAC_ADIF;
	//		iPluginManager->setParam(TT_AACDEC_PID_FRAMETYPE, &nType);
	//	}else if(pCurAudioInfo->iFourCC == MAKEFOURCC('R','A','W',' ')) {
	//		nType = TTAAC_RAWDATA;
	//		iPluginManager->setParam(TT_AACDEC_PID_FRAMETYPE, &nType);
	//	}
	//} else if(pCurAudioInfo->iMediaTypeAudioCode == TTAudioInfo::KTTMediaTypeAudioCodeWAV) {
	//	if(pCurAudioInfo->iDecInfo)
	//		memcpy(&mWAVFormat, pCurAudioInfo->iDecInfo, sizeof(TTWAVFormat));
	//	
	//	mAudioFormat.Channels = pCurAudioInfo->iChannel;
	//	mAudioFormat.SampleRate = pCurAudioInfo->iSampleRate;
	//	mAudioFormat.SampleBits = mWAVFormat.iBitsPerSample;		
	//	if(mAudioFormat.SampleBits == 0)
	//		mAudioFormat.SampleBits = 16;
	//	if(mWAVFormat.iFmtTag == FORMAT_TAG_IEEE_FLOAT)
	//		mAudioFormat.nReserved = FORMAT_TAG_IEEE_FLOAT;

	//	if (mAudioFormat.SampleBits != 16)
	//		mDoSampleBits = 1;
	//	if (mAudioFormat.Channels > 2)
	//		mDoDownMix = 1;

	//	if (mAudioFormat.SampleRate > 48000)
	//	{
	//		mDoSampleRateConv = 1;
	//		SAFE_DELETE(mResampleObj);

	//		TTInt32 tgtSamplerate = 48000;

	//		if ( mAudioFormat.SampleRate %44100 == 0)
	//			tgtSamplerate = 44100;

	//		mResampleObj = new aflibConverter(ETTFalse, ETTFalse, ETTTrue);
	//		mResmpFactor = (double)tgtSamplerate / mAudioFormat.SampleRate;
	//		mResampleObj->initialize(mResmpFactor, mAudioFormat.Channels);
	//	}
	//	//if (mAudioFormat > )
	//	return nErr;
	//}

	nErr = iPluginManager->getParam(TT_PID_AUDIO_FORMAT, &mAudioFormat);

	if(nErr != TTKErrNone || mAudioFormat.Channels == 0 || mAudioFormat.SampleRate == 0) {
		mAudioFormat.Channels = pCurAudioInfo->iChannel;
		mAudioFormat.SampleRate = pCurAudioInfo->iSampleRate;
		mAudioFormat.SampleBits = 16;

		iPluginManager->setParam(TT_PID_AUDIO_FORMAT, &mAudioFormat);
	}

	updateStep();
	return nErr;
}

TTInt CTTPureDecodeEntity::DecBuffer(TTPBYTE pBuffer, TTInt32 nSize)
{
	TTBuffer TmpBuffer;
	TTInt32 nErr = TTKErrNone;

	memset(&TmpBuffer, 0, sizeof(TTBuffer));

	TTAudioFormat AudioFormat;
	memcpy(&AudioFormat, &mAudioFormat, sizeof(TTAudioFormat));
	
	int nNum = 0;
	while(1) {
		nNum++;
		if(nNum > 50)	{
			nErr = TTKErrUnderflow;
			break;
		}

		TmpBuffer.pBuffer = pBuffer + mAudioDecSize;
		TmpBuffer.nSize = nSize - mAudioDecSize;

		nErr = iPluginManager->process(&TmpBuffer, &AudioFormat);

		if(nErr == TTKErrNone)	{
			if(AudioFormat.Channels != mAudioFormat.Channels || AudioFormat.SampleBits != mAudioFormat.SampleBits || AudioFormat.SampleRate != mAudioFormat.SampleRate)	{
				mAudioFormat.Channels = AudioFormat.Channels;
				mAudioFormat.SampleBits = AudioFormat.SampleBits;
				mAudioFormat.SampleRate = AudioFormat.SampleRate;
				nErr = TTKErrFormatChanged;
				break;
			}

			mAudioDecSize += TmpBuffer.nSize;
			if (mAudioDecSize > mAudioStepSize)
				break;
		} else if(nErr == TTKErrOverflow || nErr == TTKErrUnderflow) {
			break;
		}
		else{
			nErr = TTKErrUrlInValid;
			break;
		}
	}

	return nErr;
}

TTInt CTTPureDecodeEntity::OpenAndParse(const TTChar* aUrl)
{
	LOGI("CTTPureDecodeEntity::OpenAndParse");
	
	iSleepDataSize = 2;
	iWaitIntervalUs = 5;
	iCopyPos = 0;

	mDoSampleBits =0;
	mDoDownMix =0;
	mDoSampleRateConv =0;

	aDstBuffer->nSize = KSinkBufferSize;

	memset(&mSrcBuffer, 0, sizeof(mSrcBuffer));
	mSrcBuffer.nFlag = TT_FLAG_BUFFER_SEEKING;
	
	TTInt nErr = iMediaInfoProxy->Open(aUrl, 0);
	if(TTKErrNone != nErr)
    {
        iMediaInfoProxy->Close();
		return nErr;
	}
	nErr = iMediaInfoProxy->Parse();
	if(TTKErrNone != nErr)
	{
		iMediaInfoProxy->Close();
		return nErr;
	}
	return nErr;
}

TTInt CTTPureDecodeEntity::InitDecodePlugin()
{
	TTInt nErr = TTKErrNone;
	const TTMediaInfo& curMediaInfo = iMediaInfoProxy->GetMediaInfo();
	const RTTPointerArray<TTAudioInfo>& rMediaAudioArrary = curMediaInfo.iAudioInfoArray;
	TTInt mAudioStreamId;
	if(rMediaAudioArrary.Count() > 0) 
	{
		if(rMediaAudioArrary[0])
		{
			mAudioStreamId = rMediaAudioArrary[0]->iStreamId;
			iMediaInfoProxy->SelectStream(EMediaTypeAudio, mAudioStreamId);

			TTAudioInfo* pCurAudioInfo = rMediaAudioArrary[0];
			TTUint32 mAudioCodec = pCurAudioInfo->iMediaTypeAudioCode;

			if (iCurSrcMediaFormat != mAudioCodec)
			{
				iPluginManager->uninitPlugin();
				iCurSrcMediaFormat = mAudioCodec;
				if( mAudioCodec != TTAudioInfo::KTTMediaTypeAudioCodeWAV) {
					nErr = iPluginManager->initPlugin(pCurAudioInfo->iMediaTypeAudioCode, pCurAudioInfo->iDecInfo);
				}
			}	

			//set flag for aac
			//if(iCurSrcMediaFormat == TTAudioInfo::KTTMediaTypeAudioCodeAAC ){
			//	int value = 1;
			//	iPluginManager->setParam(TT_AACDEC_PID_DISABLEAACPLUSV1,&value);
			//	iPluginManager->setParam(TT_AACDEC_PID_DISABLEAACPLUSV2,&value);
			//}

			updateParam(pCurAudioInfo);
		}
		else
			nErr = TTKErrNotSupported;
	}
	else
		nErr = TTKErrNotSupported;

#ifdef __TT_OS_IOS__
    if (iCurSrcMediaFormat == TTAudioInfo::KTTMediaTypeAudioCodeMP3) {
        iSleepDataSize = 1;//3.7s 12%
		iWaitIntervalUs = 10;
    }
    else if (iCurSrcMediaFormat == TTAudioInfo::KTTMediaTypeAudioCodeWAV) {
        iSleepDataSize = 1;//3.7s 12%
		iWaitIntervalUs = 8;
    }
    else if (iCurSrcMediaFormat == TTAudioInfo::KTTMediaTypeAudioCodeAPE )
	{
		iSleepDataSize = 1;//3.7s 12%
		iWaitIntervalUs = 100;
        
	}
	else if(iCurSrcMediaFormat == TTAudioInfo::KTTMediaTypeAudioCodeFLAC )
	{
		iSleepDataSize = 1;//2s
		iWaitIntervalUs = 40;
	}
	else if(iCurSrcMediaFormat == TTAudioInfo::KTTMediaTypeAudioCodeAAC )
	{
		iSleepDataSize = 1;//1.3s
		iWaitIntervalUs = 8;
	}
	else if(iCurSrcMediaFormat == TTAudioInfo::KTTMediaTypeAudioCodeWMA || iCurSrcMediaFormat == TTAudioInfo::KTTMediaTypeAudioCodeWMAPRO || iCurSrcMediaFormat == TTAudioInfo::KTTMediaTypeAudioCodeWMAFLOAT)
	{
		iSleepDataSize = 1;//1.3s
		iWaitIntervalUs = 60;
	}
    else if(iCurSrcMediaFormat == TTAudioInfo::KTTMediaTypeAudioCodeIPodLibrary)
    {
        iSleepDataSize = 1;//3.7s 12%
		iWaitIntervalUs = 8;
    }
#else
	if (iCurSrcMediaFormat == TTAudioInfo::KTTMediaTypeAudioCodeAPE )
	{
		iSleepDataSize = 1;//3.7s 12%
		iWaitIntervalUs = 800;
		LOGE("ape "); 
	}
	else if(iCurSrcMediaFormat == TTAudioInfo::KTTMediaTypeAudioCodeDTS )
	{
		iSleepDataSize = 3;//5s 10%
		iWaitIntervalUs = 40;
		LOGE("DTS");
	}
	else if(iCurSrcMediaFormat == TTAudioInfo::KTTMediaTypeAudioCodeFLAC )
	{
		iSleepDataSize = 1;//2s
		iWaitIntervalUs = 5;
		LOGE("flac");
	}
	else if(iCurSrcMediaFormat == TTAudioInfo::KTTMediaTypeAudioCodeAAC )
	{
		iSleepDataSize = 2;//1.3s
		iWaitIntervalUs = 6;
		LOGE("aac");
	}
	else if(iCurSrcMediaFormat == TTAudioInfo::KTTMediaTypeAudioCodeWMA)
	{
		iSleepDataSize = 1;//1.3s
		iWaitIntervalUs = 40;

		LOGE("wma");
	}
#endif
	
	return nErr;
}

TTInt CTTPureDecodeEntity::SeekToStartPos()
{ 
	TTUint duration ;
	//iDecodrStartPos = 367029;
	if (iDecodrStartPos > 0)
	{
		duration = iDecodrStartPos;
	}
	else{
		duration = iMediaInfoProxy->MediaDuration();
		if (duration > MAX_DURATION)
			duration = duration/2;
	}

	TTInt pos = iMediaInfoProxy->Seek(duration, 0);
	if ( pos < 0)
		return  TTKErrNotFound;

	return TTKErrNone;
}

TTInt CTTPureDecodeEntity::DecodeData(TTInt nArraySize)
{
	//TTChar sdcardpath[256]={0};

	//strcpy(sdcardpath,"/sdcard/tmp/");
	//strcpy(sdcardpath,"d:/");

	//const TTChar*  str = strrchr(aUrl,'/');
	//str++;
	//strcat(sdcardpath,str);
	//srand((int)time(0));
	//int a = rand();
	//TTChar times[20] = {0};
	//sprintf(times,"%d.pcm",a);
	//strcat(sdcardpath,times);
	//FILE*	 fp = fopen(sdcardpath,"wb+");
	TTInt InvalidNum = 0;
	TTInt UnderflowNum = 0;
	TTInt nDecodeBlock = 0;
	TTInt nErr;
	TTInt32	nSize;
	//TTInt32 nFlag;
	TTPBYTE	pBuffer;
	TTInt dataSize = 0;
	TTInt srcdataSize = 0;
	TTAudioFormat AudioFormat;
	memcpy(&AudioFormat, &mAudioFormat, sizeof(TTAudioFormat));

_begin:
	if (iCancel)
		return TTKErrNone;

	if (UnderflowNum > 50)
		return TTKErrNotSupported;

	nDecodeBlock++;
	//sleep 10 ms per 40k data;
	if ((nDecodeBlock % iSleepDataSize) == 0 )
		iSemaphore.Wait(iWaitIntervalUs);

	nErr = iMediaInfoProxy->GetMediaSample(EMediaTypeAudio, &mSrcBuffer);

	if(iCurSrcMediaFormat == TTAudioInfo::KTTMediaTypeAudioCodeWAV) {
		srcdataSize = mSrcBuffer.nSize;
		if (mSrcBuffer.nFlag != 0)
			mSrcBuffer.nFlag = 0;

		if(nErr == TTKErrEof) 
		{
			if (iCopyPos >= (iDecodeDataSize*2/3))
				return TTKErrNone;
			else
				return TTKErrNotSupported;
		}

		if(nErr != TTKErrNone)	{
			UnderflowNum++;
			goto _begin;
		}
		dataSize = 0;
		if(mDoSampleBits)
		{
			doSampleBitsConv(&mSrcBuffer, aDstBuffer);
			dataSize = aDstBuffer->nSize;
		}
		if (mDoDownMix)
		{
			if (mDoSampleBits)
				doChannelDoMix(aDstBuffer,aDstBuffer);
			else
				doChannelDoMix(&mSrcBuffer, aDstBuffer);
			dataSize = aDstBuffer->nSize;
		}

		if (mDoSampleRateConv >0 &&  mDoDownMix > 0){
			doSampleRateConv(aDstBuffer, &mSrcBuffer);
			dataSize = 0;
		}
		else if(mDoSampleRateConv >0 && mDoDownMix == 0 && mDoSampleBits == 1)
		{
			doSampleRateConv(aDstBuffer, &mSrcBuffer);
			dataSize = 0;
		}
		else if(mDoSampleRateConv >0 && mDoDownMix == 0 && mDoSampleBits == 0)
		{
			doSampleRateConv(&mSrcBuffer, aDstBuffer);
			dataSize =  aDstBuffer->nSize;
		}
		int nFillSize ;
		if (dataSize == 0){
			dataSize = mSrcBuffer.nSize ;
			nFillSize = (dataSize > nArraySize) ? nArraySize : dataSize;
			memcpy(iDecodeDataPtr + iCopyPos, mSrcBuffer.pBuffer, nFillSize);
		}
		else {
			nFillSize = (dataSize > nArraySize) ? nArraySize : dataSize;
			memcpy(iDecodeDataPtr + iCopyPos, aDstBuffer->pBuffer, nFillSize);
		}
		nArraySize -= nFillSize;
		iCopyPos += nFillSize;

		aDstBuffer->nSize = KSinkBufferSize;
		mSrcBuffer.nSize = srcdataSize;
		if(nArraySize <= 0)
			return TTKErrNone;

		goto _begin;
	}


	if (mSrcBuffer.nFlag != 0)
		mSrcBuffer.nFlag = 0;
	if (nErr == TTKErrNone)
	{
		nErr = iPluginManager->setInput(&mSrcBuffer);
		if(nErr != TTKErrNone)
			return TTKErrNone;

__retry:
		nSize = aDstBuffer->nSize;
		pBuffer = aDstBuffer->pBuffer;

		nErr = DecBuffer(pBuffer,nSize);

		if (nErr == TTKErrUnderflow)
		{
			UnderflowNum++;
			goto _begin;
		}
		else if (nErr == TTKErrNone || nErr == TTKErrOverflow)
		{
			UnderflowNum = 0;
			//copy data
			if(mAudioDecSize >= mAudioStepSize) 
			{
				//aDstBuffer->nSize = mAudioDecSize;
				int nFillSize = (mAudioDecSize > nArraySize) ? nArraySize : mAudioDecSize;
				memcpy(iDecodeDataPtr + iCopyPos, pBuffer, nFillSize);
				nArraySize -= nFillSize;
				iCopyPos += nFillSize;
				if(nArraySize <= 0)
					return TTKErrNone;

				mAudioDecSize = 0;

				//decode one more
				goto __retry;
			}
			InvalidNum = 0;
			goto _begin;
		}else
		{
			InvalidNum++;
			if (InvalidNum < 50)
				goto _begin;	
		}
	}
	else if (nErr ==TTKErrEof)
	{
		if (iCopyPos >= (iDecodeDataSize*2/3))
			return TTKErrNone;
		else
			return TTKErrNotSupported;
	}
	else
		return TTKErrNotSupported;
    
    return TTKErrNotSupported;
}

TTUint8* CTTPureDecodeEntity::GetPCMData()
{
	return iDecodeDataPtr;
}

TTInt CTTPureDecodeEntity::Decode(const TTChar* aUrl, TTInt size)
{
	TTASSERT(aUrl != NULL);

	if (size > iDecodeDataSize)
	{
		free(iDecodeDataPtr);
		iDecodeDataSize = size;
		iDecodeDataPtr = (TTUint8*)malloc(iDecodeDataSize);
	}
	//LOGE("--Start decode--");
	TTInt nRet = OpenAndParse(aUrl);
	if (TTKErrNone != nRet)
		return nRet;
	//init decode lib
	if (TTKErrNone != InitDecodePlugin())
	{
		return nRet;
	}
	nRet = SeekToStartPos();
	if (TTKErrNone == nRet)
	{
		nRet = DecodeData(size);
	}
	if (nRet == 0 )
	{ 
		//TTChar* sdcardpath ="d:/ttmp9.pcm";
		//FILE*	 fp = fopen(sdcardpath,"wb+");
		//if (fp)
		//{
		//	fwrite(iDecodeDataPtr,1, iCopyPos,fp);
		//	fclose(fp);
		//}
		//if (mDoDownMix >0 && mAudioFormat.Channels >2)
		//{
		//	mAudioFormat.Channels = 2;
		//}

		if (mDoSampleRateConv >0 && mAudioFormat.SampleRate > 48000){
			if ( mAudioFormat.SampleRate%44100 == 0)
				mAudioFormat.SampleRate = 44100;
			else
				mAudioFormat.SampleRate = 48000;
		}
	}

	//LOGE("--decode nErr = %d --",nRet);

	iPluginManager->resetPlugin();
	iMediaInfoProxy->Close();

	//if (iCancel)
	//	nRet = TTKErrCancel;

	return nRet;
}


TTInt CTTPureDecodeEntity::GetSamplerate()
{
	return mAudioFormat.SampleRate;
}


TTInt CTTPureDecodeEntity::GetChannels()
{
	return mAudioFormat.Channels;
}

TTInt CTTPureDecodeEntity::GetPCMDataSize()
{
	return iCopyPos;
}

void CTTPureDecodeEntity::Cancel()
{
	iCancel = ETTTrue;
}

void CTTPureDecodeEntity::SetTimeRange(TTInt start, TTInt end, TTInt decodeStartPos)
{
	if (decodeStartPos > 0)
	{
		iDecodrStartPos = start + decodeStartPos;
	}
	else{

		if (start > 0 && end > 0 && start != end)
		{
			iDecodrStartPos = start + (end - start) / 2;
		}
		else
			iDecodrStartPos = 0;
	}
}

TTInt CTTPureDecodeEntity::updateStep()
{
	if(mAudioFormat.Channels != 0 && mAudioFormat.Channels < 10 && mAudioFormat.SampleRate != 0 
		&& mAudioFormat.SampleBits != 0) {
		mAudioStepSize = mAudioFormat.Channels*mAudioFormat.SampleRate*mAudioFormat.SampleBits*50/(1000*8);
	} else	{
		mAudioStepSize = 2*44100*2*50/1000;
	}

	return TTKErrNone;
}

TTInt CTTPureDecodeEntity::doSampleBitsConv(TTBuffer* srcBuffer, TTBuffer* dstBuffer)
{
	if (mAudioFormat.SampleBits == 8) {
		convert8BitTo16Bit(srcBuffer, dstBuffer);
	} else if (mAudioFormat.SampleBits == 24) {
		convert24BitTo16Bit(srcBuffer, dstBuffer);
	} else if (mAudioFormat.SampleBits == 32) {
		if(mAudioFormat.nReserved == 0x3)
			convert32BitFloatTo16Bit(srcBuffer, dstBuffer);
		else
			convert32BitIntTo16Bit(srcBuffer, dstBuffer);
	} else if (mAudioFormat.SampleBits == 64)
		convert64BitTo16Bit(srcBuffer, dstBuffer);

	return TTKErrNone;
}

void  CTTPureDecodeEntity::convert8BitTo16Bit(TTBuffer* srcBuffer, TTBuffer* dstBuffer)
{
	TTInt nCnt = srcBuffer->nSize;

	TTUint8* pSrc = srcBuffer->pBuffer;
	TTInt16* pDst = (TTInt16*)(dstBuffer->pBuffer);
	
	TTInt16 nSignedShort; 
	for (TTInt i = 0; i < nCnt; i++) {
		nSignedShort = *pSrc++ - 128;
		nSignedShort = nSignedShort<<8;
		*pDst++ = nSignedShort;
	}
	
	dstBuffer->nSize = nCnt*sizeof(TTInt16);

	//if(mDoConvert > 4 ) {
	//	memcpy(srcBuffer->pBuffer, dstBuffer->pBuffer, dstBuffer->nSize);
	//	srcBuffer->nSize = dstBuffer->nSize;
	//}
}

void  CTTPureDecodeEntity::convert24BitTo16Bit(TTBuffer* srcBuffer, TTBuffer* dstBuffer)
{
	TTInt nCnt = srcBuffer->nSize / 3;

	TTUint8* pSrc = srcBuffer->pBuffer;
	TTUint8* pDst = dstBuffer->pBuffer;

	for (TTInt i = 0; i < nCnt; i++) {
		pSrc++;
		*pDst++ = *pSrc++;
		*pDst++ = *pSrc++;
	}

	dstBuffer->nSize = nCnt * sizeof(TTInt16);
}

void  CTTPureDecodeEntity::convert32BitFloatTo16Bit(TTBuffer* srcBuffer, TTBuffer* dstBuffer)
{
	TTInt nCnt = srcBuffer->nSize / 4;

	TTUint8* pSrc = srcBuffer->pBuffer;
	TTUint8* pDst = dstBuffer->pBuffer;

	for (TTInt i = 0; i < nCnt; i++) {
		TTInt32 intValue = *(float*)pSrc * 32768;
		*pDst++ = intValue & 0x00ff;
		*pDst++ = (intValue & 0xff00) >> 8;
		pSrc += 4;
	}
	dstBuffer->nSize = nCnt * sizeof(TTInt16);
}

void  CTTPureDecodeEntity::convert32BitIntTo16Bit(TTBuffer* srcBuffer, TTBuffer* dstBuffer)
{
	TTInt nCnt = srcBuffer->nSize / 4;

	TTUint8* pSrc = srcBuffer->pBuffer;
	TTUint8* pDst =  dstBuffer->pBuffer;

	for (TTInt i = 0; i < nCnt; i++) {
		pSrc += 2;
		*pDst++ = *pSrc++;
		*pDst++ = *pSrc++;
	}

	dstBuffer->nSize = nCnt * sizeof(TTInt16);
}

void  CTTPureDecodeEntity::convert64BitTo16Bit(TTBuffer* srcBuffer, TTBuffer* dstBuffer)
{
	TTInt nCnt = srcBuffer->nSize / 8;

	TTUint8* pSrc = srcBuffer->pBuffer;
	TTUint8* pDst = dstBuffer->pBuffer;

	for (TTInt i = 0; i < nCnt; i++) {
		TTInt32 intValue = *(double*)pSrc * 32768;
		*pDst++ = intValue & 0x00ff;
		*pDst++ = (intValue & 0xff00) >> 8;
		pSrc += 8;
	}

	dstBuffer->nSize = nCnt * sizeof(TTInt16);
}

TTInt CTTPureDecodeEntity::doChannelDoMix(TTBuffer* srcBuffer, TTBuffer* dstBuffer)
{
	TTInt i;
	TTInt16* InBuf = (TTInt16*)srcBuffer->pBuffer;
	TTInt16* OutBuf = (TTInt16*)dstBuffer->pBuffer;
	TTInt  Length = srcBuffer->nSize/(mAudioFormat.Channels*sizeof(TTInt16));
	TTInt  InChan = mAudioFormat.Channels;
	TTInt  OutChan = 2;

	if(InChan == 6)	{
		TTInt C,L_S,R_S,tmp,tmp1,cum;
#define DM_MUL 5248/16384  //3203/10000
#define RSQRT2 5818/8192	//7071/10000
#define CLIPTOSHORT(x)  ((((x) >> 31) == (x >> 15))?(x):((x) >> 31) ^ 0x7fff)

		for(i = 0; i < Length; i++)
		{
			C   = InBuf[2]*RSQRT2;
			L_S = InBuf[4]*RSQRT2;
			cum = InBuf[0] + C + L_S;
			tmp = cum*DM_MUL;

			R_S = InBuf[5]*RSQRT2;
			cum = InBuf[1] + C + R_S;
			tmp1 = cum*DM_MUL;

			OutBuf[0] = (TTInt16)CLIPTOSHORT(tmp);
			OutBuf[1] = (TTInt16)CLIPTOSHORT(tmp1);
			OutBuf+=OutChan;
			InBuf+=InChan;
		}

		dstBuffer->nSize = Length*sizeof(TTInt16)*2;
	} else if(InChan > 2 && InChan != 6 && OutChan == 2) {
		for(i = 0; i < Length; i++)	{
			OutBuf[0] = InBuf[0];
			OutBuf[1] = InBuf[1];
			OutBuf += OutChan;
			InBuf += InChan;
		}

		dstBuffer->nSize = Length*sizeof(TTInt16)*2;		
	}

	return TTKErrNone;
}

TTInt CTTPureDecodeEntity::doSampleRateConv(TTBuffer* srcBuffer, TTBuffer* dstBuffer)
{
	TTInt32 outlen = 0;
	TTInt inlen = 0;
	TTInt16* output = (TTInt16*)dstBuffer->pBuffer;
	TTInt16* input = (TTInt16*)srcBuffer->pBuffer;

	inlen = srcBuffer->nSize/sizeof(short)/2;
	outlen = inlen * mResmpFactor;

	if(inlen > 0)
		outlen = mResampleObj->resample(inlen, outlen, input, output);

	dstBuffer->nSize = outlen*sizeof(short)*2;
	return TTKErrNone;
}

