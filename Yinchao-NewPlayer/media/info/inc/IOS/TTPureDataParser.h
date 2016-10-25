/* Copyright (c) 2004, Nokia. All rights reserved */


#ifndef __TT_IPOD_LIBRARY_PARSER_H__
#define __TT_IPOD_LIBRARY_PARSER_H__

// INCLUDES
#include "TTTypedef.h"
#include "TTMacrodef.h"
#include "TTMediaParserItf.h"
#include "TTDataReaderItf.h"
#include "TTMediaInfoDef.h"

#include "TTMediaParser.h"

class CTTIPodLibraryParser : public CTTMediaParser
{

public: // Constructors and destructor

    /**
     * \fn							CTTIPodLibraryParser.
     * \brief						C++ constructor.
     */
	CTTIPodLibraryParser(ITTDataReader& aDataReader, ITTMediaParserObserver& aObserver);
    
	/*destructor*/
	virtual							~CTTIPodLibraryParser();


public: // Functions from ITTDataParser

    /**
     * \fn							TTInt Parse(TTMediaInfo& aMediaInfo);
     * \brief						解析媒体源
     * \param[out] 	aMediaInfo		解析好的媒体信息
     * \return						成功返回KErrNone，否则返回错误值。
     */
	virtual TTInt					Parse(TTMediaInfo& aMediaInfo);
    
	/**
     * \fn							TTInt StreamCount()
     * \brief						连接MTTDataSink
     * \return						数据流个数
     */
	virtual TTInt					StreamCount();
    
	/**
     * \fn							TTUint MediaDuration()
     * \brief						获取媒体时长(毫秒)
     * \param[in]					流Id
     * \return						媒体时长
     */
	virtual TTUint					MediaDuration();
    
    virtual TTUint					MediaDuration(TTInt aStreamId);
     
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
     * \fn							TInt GetFrameLocation(TInt aStreamId, TInt& aFrmIdx, TTUint aTime)
     * \brief						根据时间获取帧索引位置
     * \param	aStreamId[in]		媒体流ID
     * \param	aFrmIdx[out]		帧索引
     * \param	aTime[in]			时间
     * \return						成功返回KErrNone，否则返回错误值。
     */
	virtual TTInt					GetFrameLocation(TTInt aStreamId, TTInt& aFrmIdx, TTUint64 aTime);
    
    /**
     * \fn							TTInt GetFrameTimeStamp(TTInt aStreamId, TTInt aFrmIdx, TTUint32 aPlayTime)
     * \brief						获取一帧数据位置
     * \param	aStreamId[in]		媒体流ID
     * \param	aFrmIdx[in]			帧索引
     * \param	aPlayTime[out]		帧起始时间(毫秒)
     * \return						成功返回KErrNone，否则返回错误值。
     */
    virtual TTInt					GetFrameTimeStamp(TTInt aStreamId, TTInt aFrmIdx, TTUint64 aPlayTime);

	/**
     * \fn							void StartFrmPosScan();
     * \brief						开始扫描帧位置，建立帧索引表
     */
	virtual void					StartFrmPosScan();
    
	/**
     * \fn                           TTBool IsCreateFrameIdxComplete();
     * \brief                        建索引是否完成
     * \return                       完成为ETTrue
     */
	virtual TTBool					IsCreateFrameIdxComplete();
    
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
     * \fn							TTInt SeekWithinFrmPosTab(TTInt aFrmIdx, TTMediaFrameInfo& aFrameInfo)
     * \brief						在帧索引表中查找帧位置
     * \param[in]	aFrmIdx		    帧序号
     * \param[out]	aFrameInfo		帧信息
     * \return						操作是否成功
     */
    virtual TTInt					SeekWithinFrmPosTab(TTInt aStreamId, TTInt aFrmIdx, TTMediaFrameInfo& aFrameInfo);
};

#endif 

// End of File
