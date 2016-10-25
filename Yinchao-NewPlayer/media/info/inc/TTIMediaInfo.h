#ifndef __TT_IMEDIA_INFO_H__
#define __TT_IMEDIA_INFO_H__

#include "TTTypedef.h"
#include "TTMediadef.h"

enum TTSourceType
{
	TTSourceNone = 0
	, TTSourcePlayList = 1
	, TTSourceTS = 2
	, TTSourceAudio = 3
	, TTSourceSubtitle = 4
};

enum TTSrcNotifyMsg
{
	ESrcNotifyUpdateDuration = 7
	, ESrcNotifyBufferingStart = 16
	, ESrcNotifyBufferingDone = 17
	, ESrcNotifyDNSDone = 18
	, ESrcNotifyConnectDone = 19
	, ESrcNotifyHttpHeaderReceived = 20
    , ESrcNotifyPrefetchStart = 21
	, ESrcNotifyPrefetchCompleted = 22
	, ESrcNotifyCacheCompleted = 23
	, ESrcNotifyException = 24
};

// CLASS DECLARATION
class ITTMediaInfo
{
public:

	/**
	* \fn								 TTInt Open(const TTChar* aUrl);
	* \brief							 打开媒体文件
	* \param[in]   aUrl					 文件路径  
	* \param[in]   aStreamBufferingObserver	 ITTStreamBufferingObserver对象指针 
	* \return							 返回状态
	*/
	virtual TTInt						  Open(const TTChar* aUrl, TTInt aFlag) = 0;
	
	/**
	* \fn								 TTInt Parse();
	* \brief							 解析文件
	* \return							 返回状态
	*/
	virtual TTInt						 Parse() = 0;

	/**
	* \fn								 void Close();
	* \brief							 关闭解析器
	*/
	virtual void						 Close() = 0;
	
	/**
	* \fn								 TTUint MediaDuration()
	* \brief							 获取媒体时长(毫秒)
	* \param[in]   aMediaStreadId		 媒体流Id  
	* \return							 媒体时长
	*/
	virtual TTUint						 MediaDuration() = 0;

	/**
	* \fn								 TTMediaInfo GetMediaInfo();
	* \brief							 分析文件
	* \param[in]	aUrl				 文件路径
	*/
	virtual const TTMediaInfo&	     	 GetMediaInfo() = 0;

	/**
	* \fn								 TTUint MediaSize()
	* \brief							 获取媒体大小(字节)
	* \return							 媒体大小
	*/
	virtual TTUint						 MediaSize() = 0;

	/**
	* \fn								 TTBool IsSeekAble();
	* \brief							 是否可以进行Seek操作
	* \return                            ETTTrue为可以
	*/
	virtual TTBool						 IsSeekAble() = 0;
	/**
	* \fn								 void CreateFrameIndex();
	* \brief							 开始创建帧索引，对于没有索引的文件，需要异步建立索引，
	* \									 对于已有索引的文件，直接读取
	*/
	virtual void						 CreateFrameIndex() = 0;

	/**
	* \fn								 TTMediaInfo GetMediaSample();
	* \brief							 获得audio，video，等sample信息
	* \param[in]	aUrl				 文件路径
	*/
	virtual TTInt			 			 GetMediaSample(TTMediaType aStreamType, TTBuffer* pMediaBuffer) = 0;

	/**
	* \fn								 TTInt SelectStream()
	* \brief							 选择audio stream。
	* \param[in]	aStreamId			 Media Stream ID,或者audio或者video stream
	* \return							 返回状态
	*/
	virtual TTInt			     		 SelectStream(TTMediaType aType, TTInt aStreamId) = 0;
	
	/**
	* \fn								 void Seek(TTUint aPosMS);
	* \brief							 Seek操作
	* \param[in]	aPosMS				 位置，单位：毫秒
	* \return							 返回真正seek的位置，如果是负数，那是seek 失败。
	*/
	virtual TTInt64						 Seek(TTUint64 aPosMS,TTInt aOption = 0) = 0;


	virtual void						 SetNetWorkProxy(TTBool aNetWorkProxy) = 0;

	/**
	* \fn								 TTInt SetParam()
	* \brief							 设置参数。
	* \param[in]	aType				 参数类型
	* \param[in]	aParam				 参数值
	* \return							 返回状态
	*/
	virtual TTInt				     	 SetParam(TTInt aType, TTPtr aParam) = 0;

	/**
	* \fn								 TTInt GetParam()
	* \brief							 获取参数值。
	* \param[in]	aType				 参数类型
	* \param[in/out]	aParam			 参数值
	* \return							 返回状态
	*/
	virtual TTInt	     				 GetParam(TTInt aType, TTPtr aParam) = 0;

	/**
	* \fn								 void CancelReader()
	* \brief							 取消网络reader
	*/
	virtual void						 CancelReader() = 0;

	/**
	* \fn								 TTUint BandWidth()
	* \brief							 获取网络下载带宽大小
	* \return							 缓冲大小(单位:字节)
	*/
	virtual TTUint						 BandWidth() = 0;

	/**
	* \fn								 TTUint BandPercent()
	* \brief							 获取网络下载数据百分比
	* \return							 缓冲大小(单位:字节)
	*/
	virtual TTUint						 BandPercent() = 0;

	/**
	* \fn								 TTBool IsCreateFrameIdxComplete();
	* \brief							 建索引是否完成
	* \return							 完成为ETTrue
	*/
	virtual TTBool						 IsCreateFrameIdxComplete() = 0;

	/**
	* \fn								 TTUint BufferedSize()
	* \brief							 获取已缓冲的数据大小
	* \return							 已缓冲的数据大小
	*/
	virtual TTUint						 BufferedSize() = 0;


	virtual TTUint						 ProxySize() = 0;

	virtual void					     SetDownSpeed(TTInt aFast) = 0;

	/**
	* \fn								 TTInt BufferedPercent(TTInt& aBufferedPercent)
	* \brief							 获取已缓冲数据的百分比
	* \param[out]	aBufferedPercent	 百分比
	* \return							 操作状态
	*/
	virtual TTInt						 BufferedPercent(TTInt& aBufferedPercent) = 0;

	/**
	* \fn								 ReParse(TTInt aErrCode)
	* \brief							 根据错误码信息，解析文件格式
	* \param    aErrCode[in]			 错误码
	* \return							 成功解析为TTKErrNone，否则为TTKErrNotSupported
	*/
	virtual void						 SetObserver(TTObserver*	aObserver) = 0;
};

#endif  //__TT_HLS_INFO_PROXY_H__
