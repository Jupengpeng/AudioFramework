/* Copyright (c) 2004, Nokia. All rights reserved */


#ifndef __TT_MEDIA_PAESER_H__
#define __TT_MEDIA_PAESER_H__

// INCLUDES
#include "TTTypedef.h"
#include "TTMacrodef.h"
#include "TTSemaphore.h"
#include "TTMediaParserItf.h"
#include "TTDataReaderItf.h"
#include "TTMediadef.h"
#include "TTMediainfoDef.h"
//#include "AVCDecoderTypes.h"

static TTInt KMinSeekOverflowOffset = 1000;//1000ms 
static TTUint32 KOnlineWaitIntervalMs = 50;//unit:微秒

enum TTReadResult
{
	EReadErr	 			=  -1
	, EReadOK		 		=   0
	, EReadEof	 			=   1
	, EReadOverflow			=   2
	, EReadUnderflow	    =   3
};


#define SYNC_COMPLETED_SHIFT 1
#define SYNC_COMPLETED_MASK	 2
#define SYNC_EOF_SHIFT 0
#define SYNC_EOF_MASK 1

#define SYNC_COMPLETED(result)	(result) & SYNC_COMPLETED_MASK
#define SYNC_EOF(result)		(result) & SYNC_EOF_MASK

enum TTFrmSyncResult
{
	EFrmSyncInComplete	= -2
	, EFrmSyncReadErr		= -1
	, EFrmSyncEofInComplete	= (0<<SYNC_COMPLETED_SHIFT) | (1<<SYNC_EOF_SHIFT)
	, EFrmSyncComplete		= (1<<SYNC_COMPLETED_SHIFT) | (0<<SYNC_EOF_SHIFT)
	, EFrmSyncEofComplete	= (1<<SYNC_COMPLETED_SHIFT) | (1<<SYNC_EOF_SHIFT)
};


static const TTInt KTabSize = 10 * KILO;//初始字数
static const TTInt KReLINCSize = 2 * KILO;//每次增加的字数
static const TTInt KSyncBufferSize = 8 * KILO;//同步读Buffer大小
static const TTInt KAsyncBufferSize = 16 * KILO;//异步读Buffer大小
static const TTInt KMaxFirstFrmOffset = 320 * KILO; //IDv2后的第一帧最大偏移，超过这个范围为不支持
static const TTInt KSyncReadSize = KSyncBufferSize;


class CTTMediaParser : public ITTMediaParser
{

public: // Constructors and destructor

	/*destructor*/
	virtual							~CTTMediaParser();


public: // Functions from ITTDataParser

	/**
	* \fn							TTInt StreamCount()
	* \brief						连接MTTDataSink
	* \return						数据流个数
	*/
	virtual TTInt					StreamCount();

	/**
	* \fn							TTInt GetFrameLocation(TTMediaStreamId aStreamId, TTInt aFrmIdx, TTMediaFrameInfo& aFrameInfo)
	* \brief						获取一帧数据位置
	* \param	aStreamId[in]		媒体流ID
	* \param	aFrmIdx[in]			帧索引
	* \param	aFrameInfo[out]		帧信息
	* \return						成功返回KErrNone，否则返回错误值。
	*/
	virtual TTInt					GetFrameLocation(TTInt aStreamId, TTInt aFrmIdx, TTMediaFrameInfo& aFrameInfo);


	/**
	* \fn							TTInt GetFrameLocation(TTInt aStreamId, TTInt& aFrmIdx, TTUint aTime);
	* \brief						根据时间获取帧索引位置
	* \param[in]	aStreamId		媒体流ID
	* \param[out]	aFrmIdx			帧索引
	* \param[in]	aTime			时间
	* \return						成功返回KErrNone，否则返回错误值。
	*/
	virtual TTInt					GetFrameLocation(TTInt aStreamId, TTInt& aFrmIdx, TTUint64 aTime);

	/**
	* \fn							TTMediaInfo GetMediaSample();
	* \brief						获得audio，video，等sample信息
	* \param[in]	aUrl			文件路径
	*/
	virtual TTInt			 		GetMediaSample(TTMediaType aStreamType, TTBuffer* pMediaBuffer);

	/**
	* \fn							TTInt GetFrameTimeStamp(TTInt aStreamId, TTInt aFrmIdx, TTUint64 aPlayTime)	
	* \brief						获取一帧数据位置
	* \param [in]	aStreamId		媒体流ID
	* \param [in]	aFrmIdx			帧索引
	* \param [out]	aPlayTime		帧起始时间(毫秒)
	* \return						成功返回KErrNone，否则返回错误值。
	*/
	virtual TTInt					GetFrameTimeStamp(TTInt aStreamId, TTInt aFrmIdx, TTUint64 aPlayTime);
		
	/**
    * \fn                           void StartFrmPosScan();
    * \brief                        开始扫描帧位置，建立帧索引表
    */
    virtual void                    StartFrmPosScan();

	/**
	* \fn                           TTBool IsCreateFrameIdxComplete();
	* \brief                        建索引是否完成
	* \return                       完成为ETTrue
	*/
	virtual TTBool					IsCreateFrameIdxComplete();

	/**
	* \fn							TTUint MediaDuration()
	* \brief						获取媒体时长(毫秒)
	* \return						媒体时长
	*/
	virtual TTUint					MediaDuration();

	/**
	* \fn							TTUint MediaDuration()
	* \brief						获取媒体时长(毫秒)
	* \param[in]					流Id
	* \return						媒体时长
	*/
	virtual TTUint					MediaDuration(TTInt aStreamId);

	/**
	* \fn							TTInt SelectStream()
	* \brief						选择audio stream。
	* \param[in]	aStreamId		Media Stream ID,或者audio或者video stream
	* \return						返回状态
	*/
	virtual TTInt	     			SelectStream(TTMediaType aType, TTInt aStreamId);
	
	/**
	* \fn							void Seek(TTUint aPosMS);
	* \brief						Seek操作
	* \param[in]	aPosMS			位置，单位：毫秒
	* \return						返回真正seek的位置，如果是负数，那是seek 失败。
	*/
	virtual TTInt64					Seek(TTUint64 aPosMS,TTInt aOption = 0);

	/**
	* \fn							TTInt SetParam()
	* \brief						设置参数。
	* \param[in]	aType			参数类型
	* \param[in]	aParam			参数值
	* \return						返回状态
	*/
	virtual TTInt	     			SetParam(TTInt aType, TTPtr aParam);

	/**
	* \fn							TTInt GetParam()
	* \brief						获取参数值。
	* \param[in]	aType			参数类型
	* \param[in/out]	aParam		参数值
	* \return						返回状态
	*/
	virtual TTInt	     			GetParam(TTInt aType, TTPtr aParam);


private:

	/**
	* \fn							ParseFrmPos(const TUint8* aData, TTInt aParserSize);
	* \param[in]	aData			数据指针
	* \param[in]	aParserSize     数据大小
	*/
	virtual	void					ParseFrmPos(const TTUint8* aData, TTInt aParserSize);
	
	/**
	* \fn							TTInt SeekWithinFrmPosTab(TTInt aFrmIdx, TTMediaFrameInfo& aFrameInfo)
	* \brief						在帧索引表中查找帧位置
	* \param[in]	aFrmIdx		    帧序号
	* \param[out]	aFrameInfo		帧信息
	* \return						操作是否成功
	*/
	virtual TTInt					SeekWithinFrmPosTab(TTInt aStreamId, TTInt aFrmIdx, TTMediaFrameInfo& aFrameInfo) = 0;

	/**
	* \fn                           TTInt SeekWithIdx(TTInt aFrmIdx, TTInt& aFrmPos, TTInt& aFrmSize)
	* \brief                        根据帧索引查找帧位置
	* \param[in]    aFrmIdx         帧序号
	* \param[out]   aFrmPos         帧位置
	* \param[out]   aFrmSize        帧长度
	* \return                       操作是否成功
	*/
	virtual TTInt                   SeekWithIdx(TTInt aStreamId, TTInt aFrmIdx, TTInt64& aFrmPos, TTInt& aFrmSize);

	/**
	* \fn                           TTInt SeekWithPos(TTInt aPos, TTInt& aFrmPos, TTInt& aFrmSize)
	* \brief                        根据位置查找帧位置
	* \param[in]    aPos            起始查找位置
	* \param[out]   aFrmPos         帧位置
	* \param[out]   aFrmSize        帧长度
	* \return                       操作是否成功
	*/
	virtual TTInt                   SeekWithPos(TTInt aStreamId, TTInt64 aPos, TTInt64& aFrmPos, TTInt& aFrmSize);


protected:
	/**
	* \fn							TTInt SeekWithoutFrmPosTab(TTInt aFrmIdx, TTMediaFrameInfo& aFrameInfo)	
	* \brief						在帧索引表外查找帧位置
	* \param[in] 	aFrameIdx		帧号
	* \param[out]	aFrameInfo		帧信息
	* \return						操作是否成功
	*/
	virtual TTInt					SeekWithoutFrmPosTab(TTInt aStreamId, TTInt aFrmIdx, TTMediaFrameInfo& aFrameInfo);


protected:

	/**
	* \fn							CTTMediaParser.
	* \brief						C++ constructor.
	*/
	CTTMediaParser(ITTDataReader& aDataReader, ITTMediaParserObserver& aObserver);

	/**
	* \fn							ReadStreamData(const TTInt aReadPos, TTUint8*& aDataPtr, TTInt& aReadSize);
	* \brief						读数据
	* \param[in]	aReadPos		读取数据的位置。
	* \param[out]	aDataPtr		传出读取数据的指针。
	* \param[out]	aReadSize		读取的字节数。
	* \return						错误码，-1表示aReadPos超出范围，0,表示正常且文件未结束，1表示文件读完。
	*/
	TTReadResult					ReadStreamData(TTInt aReadPos, TTUint8*& aDataPtr, TTInt& aReadSize);

	/**
	* \fn							ReadStreamDataAsync(TTInt aReadPos);
	* \brief						异步读数据
	* \param[in]	aReadPos		读取数据的位置。
	*/
	/*void							ReadStreamDataAsync(TTInt aReadPos);*/

	/**
	* \fn							void FrmIdxTabReAlloc();
	* \brief						索引不够内存不够的话重新分配内存
	*/
	void							FrmIdxTabReAlloc();

	/**
	* \fn							void FrmIdxTabAlloc();
	* \brief						为索引表分配内存
	*/
	void							FrmIdxTabAlloc();

//	TTInt							ConvertAVCHead(TTAVCDecoderSpecificInfo* AVCDecoderSpecificInfo, TTPBYTE pInBuffer, TTUint32 nInSzie);
//	TTInt							ConvertHEVCHead(TTPBYTE pOutBuffer, TTUint32& nOutSize, TTPBYTE pInBuffer, TTUint32 nInSzie);
//	TTInt							ConvertAVCFrame(TTPBYTE pBuffer, TTUint32 nSize, TTUint32& nFrameLen, TTInt& IsKeyFrame);	

protected:
	
	ITTDataReader&					iDataReader;
	ITTMediaParserObserver&			iObserver;

	TTUint8*						iSyncReadBuffer;//同步读所用Buffer
	TTInt							iSyncReadBufferSize;
	TTUint8*						iAsyncReadBuffer;//异步读所用Buffer

	TTMediaInfo*					iParserMediaInfoRef;
	
	TTUint*							iFrmPosTab;//索引表指针
	TTInt							iFrmTabSize;//索引表大小

	TTBool							iFrmPosTabDone;//建立索引是否完成标志
	TTBool							iAccessBeyond;//访问超过索引范围
	TTInt							iFrmPosTabCurOffset;//当前需要读取文件的位置
	TTInt							iFrmPosTabCurIndex;//当前建立索引的位置
	
	TTInt64							iNextSeekPos;//下一次seek的位置
	TTInt							iPreSeekIdx;//上一次seek的索引值
	TTInt							iPreSeekFrameSize;//上次seek的帧大小

	RTTSemaphore					mSemaphore;

	TTInt							iNALLengthSize;
	TTUint8*						iAVCBuffer;
	TTInt							iAVCSize;

	TTInt							iStreamAudioCount;
	TTInt							iStreamVideoCount;

	TTInt64							iRawDataBegin;//媒体数据的起始位置
	TTInt64							iRawDataEnd;//媒体数据的结束位置

	TTPBYTE							iAudioBuffer;
	TTInt32							iAudioSize;

	TTPBYTE							iVideoBuffer;
	TTInt32							iVideoSize;

	TTInt							iCurAudioReadFrmIdx;//记录每条流读到的帧数
	TTInt							iCurVideoReadFrmIdx;//记录每条流读到的帧数
	TTInt							iAudioStreamId;
	TTInt							iVideoStreamId;

	TTBool							iReadEof;
	TTBool							iAudioSeek;
	TTBool							iVideoSeek;
};

#endif // __MP3_PAESER_H__

// End of File
