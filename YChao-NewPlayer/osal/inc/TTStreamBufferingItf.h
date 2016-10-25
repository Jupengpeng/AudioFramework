#ifndef __TT_STREAM_BUFFER_ITF_H__
#define __TT_STREAM_BUFFER_ITF_H__

#include "TTTypedef.h"
#include "TTOsalConfig.h"

#define KAllocateSize  4096
#define MAX_RECONNECT_COUNT  3
#define INVALID_VALUE  -1
#define CONNECT_INTERUPTER 604
#define	MAX_RECEIVED_COUNT	100

static const TTInt   KMaxFileNameSize = 64;
static const TTInt	 KMaxFilePathSize = 1024;
static const TTInt   KMaxExtSize = 16;
static const TTUint32 KWaitTimes = 600;  
static const TTUint32 KWaitIntervalMs = 100;   
static const TTInt	 KBUFFER_SIZE = 4 * KILO;


class ITTStreamBufferingObserver
{
public:
	/**
	* \fn							void BufferingStart();
	* \brief						网络数据进行缓冲	
	*/
	virtual void					BufferingStart(TTInt nErr, TTInt nStatus, TTUint32 aParam) = 0;

	/**
	* \fn							void BufferingDone();
	* \brief						网络数据缓冲完成	
	*/
	virtual void					BufferingDone() = 0;

	/**
	* \fn							void DNSDone();
	* \brief						DNS解析完成
	*/
	virtual void					DNSDone() = 0;

	/**
	* \fn							void ConnectDone();
	* \brief						连接完成
	*/
	virtual void					ConnectDone() = 0;

	/**
	* \fn							void HttpHeaderReceived();
	* \brief						Http头接收成功
	*/
	virtual void					HttpHeaderReceived() = 0;

	/**
	* \fn							void PrefetchStart();
	* \brief						网络开始接收
	*/
    virtual void                    PrefetchStart(TTUint32 aParam) = 0;

	/**
	* \fn							void PrefetchCompleted();
	* \brief						预取缓存完成
	*/
	virtual void					PrefetchCompleted() = 0;

	/**
	* \fn							void CacheCompleted(const TTChar* pFileName);
	* \brief						网络文件缓存完成	
	* \param[out]	pFileName		缓存文件路径
	*/							
	virtual void					CacheCompleted(const TTChar* pFileName) = 0 ;

	/**
	* \fn							void CacheCompleted(const TTChar* pFileName);
	* \brief						网络文件缓存完成	
	* \param[out]	pFileName		缓存文件路径
	*/							
	virtual void					DownLoadException(TTInt errorCode, TTInt nParam2, void *pParam3) = 0 ;
};

#endif //__TT_STREAM_BUFFER_ITF_H__
