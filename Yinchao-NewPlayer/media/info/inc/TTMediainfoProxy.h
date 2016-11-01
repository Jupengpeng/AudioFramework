#ifndef __TT_MEDIA_PARSER_PROXY_H__
#define __TT_MEDIA_PARSER_PROXY_H__

#include "TTTypedef.h"
#include "TTMediainfoDef.h"
#include "TTFileReader.h"
#include "TTHttpReader.h"
#include "TTMediaParser.h"
#include "TTIMediaInfo.h"

static const TTInt KMaxMediaFlagSize = 32;

class ITTDataReader;
class ITTStreamBufferingObserver;

// CLASS DECLARATION
class CTTMediaInfoProxy : public ITTMediaParserObserver, public ITTStreamBufferingObserver, public ITTMediaInfo
{
public:

	enum TTMediaFormatId
	{
		EMediaExtIdNone
		, EMediaExtIdAAC
		, EMediaExtIdALAC
		, EMediaExtIdAMR
		, EMediaExtIdAPE
		, EMediaExtIdFLAC
		, EMediaExtIdM4A
		, EMediaExtIdMIDI
		, EMediaExtIdMP3
		, EMediaExtIdOGG
		, EMediaExtIdWAV
		, EMediaExtIdWMA
		, EMediaExtIdDTS
	};

public:

	/**
	* \fn								 CTTMediaInfoProxy(TTObserver& aObserver);
	* \brief							 构造函数
	* \param[in]   aObserver			 Observer引用
	*/
	CTTMediaInfoProxy(TTObserver* aObserver);

	/**
	* \fn								 ~CTTMediaInfoProxy();
	* \brief							 析构函数
	*/
	virtual ~CTTMediaInfoProxy();

public:

	/**
	* \fn								 TTInt Open(const TTChar* aUrl);
	* \brief							 打开媒体文件
	* \param[in]   aUrl					 文件路径
	* \param[in]   aStreamBufferingObserver	 ITTStreamBufferingObserver对象指针 
	* \return							 返回状态
	*/
	virtual TTInt						 Open(const TTChar* aUrl, TTInt aFlag);
	
	/**
	* \fn								 TTInt Parse();
	* \brief							 解析文件
	* \return							 返回状态
	*/
	virtual TTInt						 Parse();

	/**
	* \fn								 void Close();
	* \brief							 关闭解析器
	*/
	virtual void						 Close();
	
	/**
	* \fn								 TTUint MediaDuration()
	* \brief							 获取媒体时长(毫秒)
	* \param[in]   aMediaStreadId		 媒体流Id  
	* \return							 媒体时长
	*/
	virtual TTUint						 MediaDuration();

	/**
	* \fn								 TTMediaInfo GetMediaInfo();
	* \brief							 分析文件
	* \param[in]	aUrl				 文件路径
	*/
	virtual const TTMediaInfo&	     	 GetMediaInfo();

	/**
	* \fn								 TTUint MediaSize()
	* \brief							 获取媒体大小(字节)
	* \return							 媒体大小
	*/
	virtual TTUint						 MediaSize();

	/**
	* \fn								 TTBool IsSeekAble();
	* \brief							 是否可以进行Seek操作
	* \return                            ETTTrue为可以
	*/
	virtual TTBool						 IsSeekAble();
	/**
	* \fn								 void CreateFrameIndex();
	* \brief							 开始创建帧索引，对于没有索引的文件，需要异步建立索引，
	* \									 对于已有索引的文件，直接读取
	*/
	virtual void						 CreateFrameIndex();

	/**
	* \fn								 TTMediaInfo GetMediaSample();
	* \brief							 获得audio，video，等sample信息
	* \param[in]	aUrl				 文件路径
	*/
	virtual TTInt			 			 GetMediaSample(TTMediaType aStreamType, TTBuffer* pMediaBuffer);

	/**
	* \fn								 TInt GetFrameLocation(TTInt aStreamId, TTInt aFrmIdx, TTMediaFrameInfo& aFrameInfo)
	* \brief							 获取一帧数据位置
	* \param	aStreamId[in]			 媒体流ID
	* \param	aFrmIdx[in]				 帧索引
	* \param	aFrameInfo[out]			 帧信息
	* \return							 成功返回KErrNone，否则返回错误值。
	*/
	virtual TTInt						 GetFrameLocation(TTInt aId, TTInt aFrmIdx, TTMediaFrameInfo& aFrameInfo);								 

	/**
	* \fn								 TInt GetFrameLocation(TInt aStreamId, TInt& aFrmIdx, TTUint aTime)
	* \brief							 根据时间获取帧索引位置
	* \param	aStreamId[in]			 媒体流ID
	* \param	aFrmIdx[out]			 帧索引
	* \param	aTime[in]				 时间
	* \return							 成功返回KErrNone，否则返回错误值。
	*/
	virtual TTInt						 GetFrameLocation(TTInt aStreamId, TTInt& aFrmIdx, TTUint64 aTime);		

	/**
	* \fn								 ITTDataReader::TTDataReaderId GetSrcReaderId();	
	* \brief							 获取MediaInfoProxy所用的DataReader的Id
	* \return							 DataReader的Id
	*/
	ITTDataReader::TTDataReaderId		 GetSrcReaderId();	


	/**
	* \fn								TTInt SelectStream()
	* \brief							选择audio stream。
	* \param[in]	aStreamId			Media Stream ID,或者audio或者video stream
	* \return							返回状态
	*/
	virtual TTInt			     		SelectStream(TTMediaType aType, TTInt aStreamId);
	
	/**
	* \fn								void Seek(TTUint aPosMS);
	* \brief							Seek操作
	* \param[in]	aPosMS				位置，单位：毫秒
	* \return							返回真正seek的位置，如果是负数，那是seek 失败。
	*/
	virtual TTInt64						Seek(TTUint64 aPosMS,TTInt aOption = 0);


	virtual void						SetNetWorkProxy(TTBool aNetWorkProxy);

	/**
	* \fn								TTInt SetParam()
	* \brief							设置参数。
	* \param[in]	aType				参数类型
	* \param[in]	aParam				参数值
	* \return							返回状态
	*/
	virtual TTInt				     	SetParam(TTInt aType, TTPtr aParam);

	/**
	* \fn								TTInt GetParam()
	* \brief							获取参数值。
	* \param[in]	aType				参数类型
	* \param[in/out]	aParam			参数值
	* \return							返回状态
	*/
	virtual TTInt	     				GetParam(TTInt aType, TTPtr aParam);

public:
	/**
	* \fn								 void CancelReader()
	* \brief							 取消网络reader
	*/
	 void								 CancelReader();

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


public:
	/**
	* \fn								 TTBool IsCreateFrameIdxComplete();
	* \brief							 建索引是否完成
	* \return							 完成为ETTrue
	*/
	TTBool								 IsCreateFrameIdxComplete();

	/**
	* \fn								 TTUint BufferedSize()
	* \brief							 获取已缓冲的数据大小
	* \return							 已缓冲的数据大小
	*/
	TTUint								 BufferedSize();


	TTUint								 ProxySize();

	void								 SetDownSpeed(TTInt aFast);

	/**
	* \fn								 TTInt BufferedPercent(TTInt& aBufferedPercent)
	* \brief							 获取已缓冲数据的百分比
	* \param[out]	aBufferedPercent	 百分比
	* \return							 操作状态
	*/
	TTInt								 BufferedPercent(TTInt& aBufferedPercent);

public:
	/**
	* \fn								 TTMediaFormatId  IdentifyMedia(ITTDataReader& aDataReader, const TTChar* aUrl, const TTUint8* aHeader, TTInt aHeaderSize)
	* \brief							 解析文件的媒体格式
	* \param	aDataReader[in]			 DataReader对象引用
	* \param	aUrl[in]				 文件路径
	* \return							 完成为ETTrue
	*/
	static TTMediaFormatId				 IdentifyMedia(ITTDataReader& aDataReader, const TTChar* aUrl);

	/**
	* \fn								 ReParse(TTInt aErrCode)
	* \brief							 根据错误码信息，解析文件格式
	* \param    aErrCode[in]			 错误码
	* \return							 成功解析为TTKErrNone，否则为TTKErrNotSupported
	*/
	TTInt								 Parse(TTInt aErrCode);

	void								 SetObserver(TTObserver*	aObserver);

public:// from ITTStreamBufferingObserver
	/**
	* \fn								 void StreamIsBuffering();
	* \brief						     网络数据真正缓冲	
	*/
	virtual void						 BufferingStart(TTInt nErr, TTInt nStatus, TTUint32 aParam);

	/**
	* \fn								 void StreamBufferCompleted();
	* \brief						 	 网络数据缓冲完成	
	*/
	virtual void						 BufferingDone();
    
	/**
	* \fn								 void DNSDone();
	* \brief							 DNS解析完成
	*/
	virtual void						 DNSDone();

	/**
	* \fn								 void ConnectDone();
	* \brief							 连接完成
	*/
	virtual void						 ConnectDone();

	/**
	* \fn								 void HttpHeaderReceived();
	* \brief							 Http头接收成功
	*/
	virtual void						 HttpHeaderReceived();

    /**
	* \fn								 void PrefetchStart(TTUint32 aParam);
	* \brief						 	 网络开始接收
	*/
    virtual void                    	 PrefetchStart(TTUint32 aParam);

	/**
	* \fn								 void PrefetchCompleted();
	* \brief							 预取缓存完成
	*/
	virtual void						 PrefetchCompleted();
	
	/**
	* \fn								 void StreamBufferCompleted();
	* \brief						 	 网络媒体数据全部完成	
	*/
	virtual void						 CacheCompleted(const TTChar* pFileName);

	virtual void	    				 DownLoadException(TTInt errorCode, TTInt nParam2, void *pParam3);

protected:
	virtual void	    				 FileException(TTInt nReadSize);

private:

	/**
	* \fn								 void CreateFrameIdxComplete();
	* \brief							 创建帧索引完成
	*/
	void								 CreateFrameIdxComplete();

	/**
	* \fn								 ITTDataReader* SelectSrcReader(const TTChar* aUrl);
	* \brief							 根据Url选择一个合适的Reader
	* \param	aUrl[in]				 路径
	* \return							 Reader实例
	*/
	void								 AdaptSrcReader(const TTChar* aUrl, TTInt aFlag, TTBool aHarddecode = ETTTrue);
	TTInt								 AdaptLocalFileParser(const TTChar* aUrl);
	TTInt								 AdaptHttpFileParser(const TTChar* aUrl);

	TTBool								 IsLocalFileSource(const TTChar* aUrl);
	TTBool								 IsIPodLibrarySource(const TTChar* aUrl);
	TTBool								 IsHttpSource(const TTChar* aUrl);
    TTBool                               IsLocalExtAudioFileSource(const TTChar* aUrl);

private:
	static TTBool IsALAC(const TTUint8* aHeader);
	static TTBool IsAPE(const TTUint8* aHeader);
	static TTBool IsAMR(const TTUint8* aHeader);
	static TTBool IsFLAC(const TTUint8* aHeader);
	static TTBool IsM4A(const TTUint8* aHeader);
	static TTBool IsMIDI(const TTUint8* aHeader);
	static TTBool IsMP3(const TTUint8* aHeader);
	static TTBool IsDTS(const TTUint8* aHeader);
	static TTBool IsWAV(const TTUint8* aHeader);
	static TTBool IsWMA(const TTUint8* aHeader);

	static TTBool ShouldIdentifiedByHeader(TTMediaFormatId aMediaFormatId);
	static TTMediaFormatId IdentifyMediaByHeader(ITTDataReader& aDataReader);
	static TTMediaFormatId IdentifyMediaByExtension(const TTChar* aUrl);

private:
	TTObserver*				iObserver;

	ITTMediaParser*			iMediaParser;	
	ITTDataReader*			iDataReader;
	TTMediaInfo				iMediaInfo;

	RTTCritical				iCriEvent;
};

#endif
