#ifndef __TT_SRCDEMUX_H__
#define __TT_SRCDEMUX_H__

// INCLUDES
#include "TTMediainfoDef.h"
#include "TTMediainfoProxy.h"
//#include "TTHLSInfoProxy.h"
#include "TTCritical.h"
#include "TTMediadef.h"

class CTTSrcDemux
{
public:
	/**
	* \fn								 CTTDemux(TTObserver& aObserver);
	* \brief							 构造函数
	* \param[in] aElementObserver		 接口引用
	*/
	CTTSrcDemux(TTObserver*	aObserver);
	
	/**
	* \fn								 ~CTTDemux();
	* \brief							 析构函数
	*/
	virtual ~CTTSrcDemux();

public:
	/**
	* \fn								 TTMediaInfo GetMediaInfo();
	* \brief							 分析文件
	* \param[in]	aUrl				 文件路径
	*/
	const TTMediaInfo&	     			 GetMediaInfo();

	/**
	* \fn								 TTInt AddDataSource(const TTChar* aUrl)
	* \brief							 添加源，分析文件
	* \param[in]	aUrl				 文件路径
	* \return							 返回状态
	*/
	TTInt	     						 AddDataSource(const TTChar* aUrl, TTInt aFlag);

	/**
	* \fn								 TTInt RemoveDataSource()
	* \brief							 移去数据源
	* \param[in]		aStreamId		 Media Stream ID,或者audio或者video stream
	* \param[in/out]	pSampleInfo		 获取sample的信息，带入的可能是时间搓，输出的是获得的时间搓。
	* \return							 返回状态
	*/
	TTInt	     						 RemoveDataSource();


	/**
	* \fn								 TTMediaInfo GetMediaSample();
	* \brief							 获得audio，video，等sample信息
	* \param[in]	aUrl				 文件路径
	*/
	TTInt			 					GetMediaSample(TTMediaType aStreamType, TTBuffer* pMediaBuffer);

	/**
	* \fn								void Seek(TTUint aPosMS);
	* \brief							Seek操作
	* \param[in]	aPosMS				位置，单位：毫秒
	* \return							返回真正seek的位置，如果是负数，那是seek 失败。
	*/
	TTInt64								Seek(TTUint64 aPosMS, TTInt aOption = 0);

	/**
	* \fn								TTInt SelectStream()
	* \brief							选择audio stream。
	* \param[in]	aStreamId			Media Stream ID,或者audio或者video stream
	* \return							返回状态
	*/
	TTInt	     						SelectStream(TTMediaType aType, TTInt aStreamId);

	/**
	* \fn                               TTUint MediaDuration()
	* \brief                            获取媒体时长(毫秒)
	* \return                           媒体时长
	*/
	TTUint								MediaDuration();

	/**
	* \fn								TTUint MediaSize()
	* \brief							获取媒体大小(字节)
	* \return							媒体大小
	*/
	TTUint								MediaSize();


	/**
	* \fn								TTUint BandWidth()
	* \brief							获取网络下载带宽大小
	* \return							缓冲大小(单位:字节)
	*/
	TTUint								BandWidth();

	/**
	* \fn								TTUint BandPercent()
	* \brief							获取网络下载数据百分比
	* \return							缓冲大小(单位:字节)
	*/
	TTUint								BandPercent();

	/**
	* \fn								TTBool IsSeekAble();
	* \brief							是否可以进行Seek操作
	* \return                           ETTTrue为可以
	*/
	TTBool								IsSeekAble();


	void								SetNetWorkProxy(TTBool aNetWorkProxy);


	void								SetObserver(TTObserver*	aObserver);

public:
	/**
	* \fn                               TTBool IsCreateFrameIdxComplete();
	* \brief                            建索引是否完成
	* \return                           完成为ETTrue
	*/
	TTBool								IsCreateFrameIdxComplete();

	/**
	* \fn								 TTUint BufferedSize()
	* \brief							 获取已缓冲的数据大小
	* \return							 已缓冲的数据大小
	*/
	TTUint								 BufferedSize();


	TTUint								 ProxySize();

	/**
	* \fn								 TTInt BufferedPercent(TTInt& aBufferedPercent)
	* \brief							 获取已缓冲数据的百分比
	* \param[out]	aBufferedPercent	 百分比
	* \return							 操作状态
	*/
	TTInt								 BufferedPercent(TTInt& aBufferedPercent);
		
	/**
	* \fn								 void CancelReader()
	* \brief							 取消网络reader
	*/
	 void								 CancelReader();

	 void								 SetDownSpeed(TTInt aFast);

	TTInt								 SetParam(TTInt aID, void* pValue);

	TTInt								 GetParam(TTInt aID, void* pValue);

protected:
	int									 IsHLSSource(const TTChar* aUrl);
	TTObserver*							 iObserver;

private:
	CTTMediaInfoProxy*					 iMediaInfoProxy;
//	CTTHLSInfoProxy*					 iHLSInfoProxy;
	ITTMediaInfo*						 iInfoProxy;
	RTTCritical							 iCritical;
	RTTCritical							 iCriEvent;
};

#endif
