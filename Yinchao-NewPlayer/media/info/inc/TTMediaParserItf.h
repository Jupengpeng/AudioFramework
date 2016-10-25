#ifndef __TT_DATA_PARSER_H__
#define __TT_DATA_PARSER_H__

// INCLUDES
#include "TTInterface.h"
#include "TTMediadef.h"
#include "TTMediainfoDef.h"

class TTMediaFrameInfo
{
public:
	TTMediaFrameInfo()
		:iFrameLocation(0)
		,iFrameStartTime(0)
		,iFrameDuration(0)
		,iFrameSize(0)
		,iFlag(0)
		,iExtraInfo(0)
	{
		
	}

public:
	TTInt64		iFrameLocation;
	TTInt64		iFrameStartTime;
	TTInt		iFrameDuration;
	TTInt		iFrameSize;
	TTInt		iFlag;
	TTInt		iExtraInfo;
};

// FORWARD DECLARATIONS
 class ITTDataReader;
 class TTMediaInfo;

class ITTMediaParserObserver
{
public:
	virtual void CreateFrameIdxComplete() = 0;
};
// CLASS DECLARATION
class ITTMediaParser: virtual public ITTInterface
{

public: // Functions from base classes

	/**
	* \fn							TTInt Parse(TTMediaInfo& aMediaInfo);
	* \brief						解析媒体源
	* \param[out] 	aMediaInfo		解析好的媒体信息
	* \return						成功返回KErrNone，否则返回错误值。
	*/
	virtual TTInt					Parse(TTMediaInfo& aMediaInfo) = 0;

	/**
	* \fn							TTInt StreamCount()
	* \brief						连接MTTDataSink
	* \return						数据流个数
	*/
	virtual TTInt					StreamCount() = 0;

	/**
	* \fn							TTUint MediaDuration()
	* \brief						获取媒体时长(毫秒)
	* \return						媒体时长
	*/
	virtual TTUint					MediaDuration() = 0;

	/**
	* \fn							TTUint MediaDuration()
	* \brief						获取媒体时长(毫秒)
	* \param[in]					流Id
	* \return						媒体时长
	*/
	virtual TTUint					MediaDuration(TTInt aStreamId) = 0;

	/**
	* \fn							TTInt GetFrameLocation(TTint aStreamId, TTInt aFrmIdx, TTMediaFrameInfo& aFrameInfo)
	* \brief						获取一帧数据位置
	* \param	aStreamId[in]		媒体流ID
	* \param	aFrmIdx[in]			帧索引
	* \param	aFrameInfo[out]		帧信息
	* \return						成功返回KErrNone，否则返回错误值。
	*/
	virtual TTInt					GetFrameLocation(TTInt aStreamId, TTInt aFrmIdx, TTMediaFrameInfo& aFrameInfo) = 0;

	/**
	* \fn							TTMediaInfo GetMediaSample();
	* \brief						获得audio，video，等sample信息
	* \param[in]	aUrl			文件路径
	*/
	virtual TTInt			 		GetMediaSample(TTMediaType aStreamType, TTBuffer* pMediaBuffer) = 0;


	/**
	* \fn							TInt GetFrameLocation(TInt aStreamId, TInt& aFrmIdx, TTUint aTime)
	* \brief						根据时间获取帧索引位置
	* \param	aStreamId[in]		媒体流ID
	* \param	aFrmIdx[out]		帧索引
	* \param	aTime[in]			时间
	* \return						成功返回KErrNone，否则返回错误值。
	*/
	virtual TTInt					GetFrameLocation(TTInt aStreamId, TTInt& aFrmIdx, TTUint64 aTime) = 0;		

	/**
	* \fn							TTInt GetFrameTimeStamp(TTInt aStreamId, TTInt aFrmIdx, TTUint32 aPlayTime)
	* \brief						获取一帧数据位置
	* \param	aStreamId[in]		媒体流ID
	* \param	aFrmIdx[in]			帧索引
	* \param	aPlayTime[out]		帧起始时间(毫秒)
	* \return						成功返回KErrNone，否则返回错误值。
	*/
	virtual TTInt					GetFrameTimeStamp(TTInt aStreamId, TTInt aFrmIdx, TTUint64 aPlayTime) = 0;

	/**
	* \fn							void StartFrmPosScan();
	* \brief						开始扫描帧位置，建立帧索引表
	*/
	virtual void					StartFrmPosScan() = 0;

	/**
	* \fn                           TTBool IsCreateFrameIdxComplete();
	* \brief                        建索引是否完成
	* \return                       完成为ETTrue
	*/
	virtual TTBool					IsCreateFrameIdxComplete() = 0;

	/**
	* \fn							TTInt SelectStream()
	* \brief						选择audio stream。
	* \param[in]	aStreamId		Media Stream ID,或者audio或者video stream
	* \return						返回状态
	*/
	virtual TTInt	     			SelectStream(TTMediaType aType, TTInt aStreamId) = 0;
	
	/**
	* \fn							void Seek(TTUint aPosMS);
	* \brief						Seek操作
	* \param[in]	aPosMS			位置，单位：毫秒
	* \return						返回真正seek的位置，如果是负数，那是seek 失败。
	*/
	virtual TTInt64					Seek(TTUint64 aPosMS, TTInt aOption = 0) = 0;

	/**
	* \fn							TTInt SetParam()
	* \brief						设置参数。
	* \param[in]	aType			参数类型
	* \param[in]	aParam			参数值
	* \return						返回状态
	*/
	virtual TTInt	     			SetParam(TTInt aType, TTPtr aParam) = 0;

	/**
	* \fn							TTInt GetParam()
	* \brief						获取参数值。
	* \param[in]	aType			参数类型
	* \param[in/out]	aParam		参数值
	* \return						返回状态
	*/
	virtual TTInt	     			GetParam(TTInt aType, TTPtr aParam) = 0;
};

#endif // __DATA_PARSER_H__

// End of File
