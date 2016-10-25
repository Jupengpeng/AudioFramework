/**
* File : TTMP4Parser.h 
* Created on : 2011-6-27
* Author : hu.cao
* Copyright : Copyright (c) 2011 Shuidushi Software Ltd. All rights reserved.
* Description : TTMP4Parser定义文件
*/

#ifndef __TT_MP4_PARSER_H__
#define __TT_MP4_PARSER_H__

#include "TTMediaParser.h"
#include "M4a.h"
#include "ALACAudioTypes.h"
#include "AVCDecoderTypes.h"

#define  TT_BOX_FLAG_STSD  0x00000001
#define  TT_BOX_FLAG_STTS  0x00000002
#define  TT_BOX_FLAG_STSC  0x00000004
#define  TT_BOX_FLAG_STCO  0x00000008
#define  TT_BOX_FLAG_STSZ  0x00000010
#define  TT_BOX_FLAG_STSS  0x00000020
#define  TT_BOX_FLAG_CTTS  0x00000040

typedef struct 
{
	TTUint iSampleCount;
	TTUint iSampleOffset;
}TCompositionTimeSample;

typedef struct 
{
	TTUint iSampleCount;
	TTUint iSampleDelta;
}TTimeToSample;

typedef struct 
{
	TTUint iFirstChunk;
	TTUint iSampleNum;
}TTChunkToSample;

typedef struct  
{
	TTUint iSampleIdx;
	TTUint iSampleFileOffset;
	TTUint iSampleEntrySize;
	TTUint iFlag;
	TTInt64 iSampleTimeStamp;
}TTSampleInfo;

typedef struct 
{
	TCompositionTimeSample*				iComTimeSampleTab;
	TTInt								iComTimeSampleEntryNum;

	TTimeToSample*						iTimeToSampleTab;
	TTInt								iTimeToSampleEntryNum;

	TTInt								iConstantSampleSize;
	TTInt								iSampleCount;
	TTInt*								iVariableSampleSizeTab;

	TTChunkToSample*					iChunkToSampleTab;
	TTInt								iChunkToSampleEntryNum;
	
	TTInt*								iChunkOffsetTab;
	TTInt								iChunkOffsetEntryNum;

	TTInt*								iKeyFrameSampleTab;
	TTInt								iKeyFrameSampleEntryNum;

	TTUint								iFrameTime;
	TTMP4DecoderSpecificInfo*			iMP4DecoderSpecificInfo;
	TTM4AWaveFormat*					iM4AWaveFormat;
	TTALACCookie*						iALACCookiePtr;
	TTAVCDecoderSpecificInfo*			iAVCDecoderSpecificInfo;

	TTSampleInfo*						iSampleInfoTab;

	TTInt64								iDuration;
	TTUint64							iTotalSize;
	TTInt								iScale;
	TTInt								iAudio;
	TTInt								iWidth;
	TTInt								iHeight;
	TTInt								iCodecType;
	TTInt								iFourCC;
	TTInt								iStreamID;
	TTInt								iMaxFrameSize;
	TTUint32							iReadBoxFlag;
	TTUint8								iLang_code[4];
	TTInt								iErrorTrackInfo;
}TTMP4TrackInfo;


class CTTMP4Parser : public CTTMediaParser
{
public: // Constructors and destructor

	/**
	* \fn                               ~CTTMP4Parser.
	* \brief                            C++ destructor.
	*/
	~CTTMP4Parser();

	/**
	* \fn                               CTTAPEParser.
	* \brief                            C++ constructor.
	*/
	CTTMP4Parser(ITTDataReader& aDataReader, ITTMediaParserObserver& aObserver);


public: // Functions from ITTMediaParser

	/**
	* \fn								TTInt Parse(TTMediaInfo& aMediaInfo);
	* \brief							解析媒体源
	* \param[out] 	aMediaInfo			解析好的媒体信息
	* \return							成功返回KErrNone，否则返回错误值。
	*/
	virtual TTInt						Parse(TTMediaInfo& aMediaInfo);

	/**
	* \fn                               TTimeIntervalMicroSeconds32 MediaDuration()
	* \brief                            获取媒体时长(毫秒)
	* \param[in]						流Id
	* \return                           媒体时长
	*/
	virtual TTUint						MediaDuration(TTInt aStreamId);

	/**
	* \fn								void Seek(TTUint aPosMS);
	* \brief							Seek操作
	* \param[in]	aPosMS				位置，单位：毫秒
	* \return							返回真正seek的位置，如果是负数，那是seek 失败。
	*/
	virtual TTInt64						Seek(TTUint64 aPosMS, TTInt aOption);

	/**
	* \fn								TInt GetFrameLocation(TInt aStreamId, TInt& aFrmIdx, TTUint aTime)
	* \brief							根据时间获取帧索引位置
	* \param[in]	aStreamId			媒体流ID
	* \param[out]	aFrmIdx				帧索引
	* \param[in]	aTime				时间
	* \return							成功返回KErrNone，否则返回错误值。
	*/
	virtual TTInt						GetFrameLocation(TTInt aStreamId, TTInt& aFrmIdx, TTUint64 aTime);

	/**
	* \fn								TTMediaInfo GetMediaSample();
	* \brief							获得audio，video，等sample信息
	* \param[in]	aUrl				文件路径
	*/
	virtual TTInt			 			GetMediaSample(TTMediaType aStreamType, TTBuffer* pMediaBuffer);

private:

	/**
	* \fn								TTInt SeekWithinFrmPosTab(TTInt aFrmIdx, TTMediaFrameInfo& aFrameInfo)
	* \brief							在帧索引表中查找帧位置
	* \param[in] 	aFrmIdx				帧序号
	* \param[out] 	aFrameInfo			帧位置
	* \return							操作是否成功
	*/
	virtual TTInt						SeekWithinFrmPosTab(TTInt aStreamId, TTInt aFrmIdx, TTMediaFrameInfo& aFrameInfo);


private:

	TTInt								ReadBoxStco(TTUint32 aBoxPos, TTUint32 aBoxLen);
	TTInt								ReadBoxCo64(TTUint32 aBoxPos, TTUint32 aBoxLen);
	TTInt								ReadBoxStsc(TTUint32 aBoxPos, TTUint32 aBoxLen);
	TTInt								ReadBoxCtts(TTUint32 aBoxPos, TTUint32 aBoxLen);
	TTInt								ReadBoxStsz(TTUint32 aBoxPos, TTUint32 aBoxLen);
	TTInt								ReadBoxStts(TTUint32 aBoxPos, TTUint32 aBoxLen);
	TTInt								ReadBoxStss(TTUint32 aBoxPos, TTUint32 aBoxLen);
	TTInt								ReadBoxStsd(TTUint32 aBoxPos, TTUint32 aBoxLen);
	TTInt								ReadBoxStsdVide(TTUint32 aBoxPos, TTUint32 aBoxLen);
	TTInt								ReadBoxStsdSoun(TTUint32 aBoxPos, TTUint32 aBoxLen);
	TTInt								ReadBoxAvcC(TTUint32 aBoxPos,TTUint32 aBoxLen);
	TTInt								ReadBoxHevC(TTUint32 aBoxPos,TTUint32 aBoxLen);
	TTInt								ReadBoxEsds(TTUint32 aBoxPos, TTUint32 aBoxLen);
	TTInt								ParseEsDescriptor(TTUint32 aDesPos, TTUint32 aDesLen);	
	TTInt								ParseDecoderConfigDescriptor(TTUint32 aDesPos, TTUint32 aDesLen);
	TTInt								ParseDecoderSpecificInfo(TTUint32 aDesPos, TTUint32 aDesLen);
	TTInt								ParseSLConfigDescriptor(TTUint32 aDesPos, TTUint32 aDesLen);
	TTInt								ReadBoxMoov(TTUint32 aBoxPos, TTUint32 aBoxLen);
	TTInt								LocationBox(TTUint32& aLocation, TTUint32& aBoxSize, const TTChar* aBoxType);
	TTInt								updateTrackInfo();
	TTInt								removeTrackInfo(TTMP4TrackInfo*	pTrackInfo);
	TTInt								buildSampleTab(TTMP4TrackInfo*	pTrackInfo);
	TTInt								getCompositionTimeOffset(TTMP4TrackInfo* pTrackInfo, TTInt aIndex, TTInt& aCurIndex, TTInt& aCurCom);

	TTInt								findNextSyncFrame(TTSampleInfo* pTrackInfo, TTSampleInfo* pCurInfo, TTInt64 nTimeStamp);

private:

	TTUint								iDuration;
	TTUint								iTotalBitrate;
	
	RTTPointerArray<TTMP4TrackInfo>		iAudioTrackInfoTab;
	TTMP4TrackInfo*						iVideoTrackInfo;

	TTMP4TrackInfo*						iCurTrackInfo;

	TTSampleInfo*						iCurAudioInfo;
	TTSampleInfo*						iCurVideoInfo;
};

#endif // __TT_MP4_PARSER_H__
