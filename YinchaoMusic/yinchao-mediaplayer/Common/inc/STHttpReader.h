/**
* File : TTHttpReaderProxy.h  
* Created on : 2012-3-19
* Author : hu.cao
* Copyright : Copyright (c) 2011 Shuidushi Software Ltd. All rights reserved.
* Description : STHttpReader定义文件
*/
#ifndef __ST_HTTP_READER_H__
#define __ST_HTTP_READER_H__

#include "STInterface.h"
#include "STThread.h"
#include "STCritical.h"
#include "STSemaphore.h"
#include "STMacrodef.h"
#include "STDataReaderItf.h"

class STHttpClient;
class STHttpCacheFile;

class STHttpReader : public ISTDataReaderItf
{	
public:
	/**
	* \fn							STHttpReader();
	* \brief						构造函数	
	*/
	STHttpReader();

	/**
	* \fn							~STHttpReader();
	* \brief						析构函数	
	*/
	virtual ~STHttpReader();

public:

	/**
	* \fn							STInt Open(const STChar* aUrl);
	* \brief						打开Http流	
	* \param[in]  aUrl				Http流路径
	* \return						操作状态
	*/
	STInt							Open(const STChar* aUrl);

	/**
	* \fn							STInt Close()
	* \brief                    	关闭文件
	* \return						操作状态
	*/
	STInt							Close();

	/**
	* \fn							STInt Read(STUint8* aReadBuffer, STInt aReadPos, STInt aReadSize);
	* \brief						读取文件	
	* \param[in]  aReadBuffer		Http流路径
	* \param[in]  aReadPos			读取位置
	* \param[in]  aReadSize			读取大小
	* \return						操作状态
	*/
	STInt							Read(STUint8* aReadBuffer, STInt aReadPos, STInt aReadSize);

	/**
	* \fn							STInt Size() const;
	* \brief						获取文件大小	
	* \return						文件大小
	*/
	STInt							Size() const;

	/**
	* \fn                       	void Abort();
	* \brief                    	取消
	*/
	void							Abort();

	/**
	* \fn							ISTDataReaderItf::STDataReaderId Id()
	* \brief                    	获取data reader的ID
	* \return						ID
	*/
	ISTDataReaderItf::DataReaderId Id();

	/**
	 *\fn							void SetCachePath(const STChar* aUrl)
	 *\param[in]  aUrl				设置缓存路径
	 */
	void							SetCachePath(const STChar* aPath);

	/**
	 *add by bin.liu
	 *\fn							void SetCachePath(const STChar* aUrl)
	 *\param[in]  aUrl				获得当前已经下载的百分比
	 */
	STInt							GetDownloadPercent();

private:
	static void* 					DownloadThreadProc(void* aPtr);
	void 							DownloadThreadProcL(void* aPtr);

private:
    enum STReadStatus
	{
    	ETTReadStatusFailed = -1,
		ETTReadStatusNotReady = 0,
		ETTReadStatusReading = 2,
		ETTReadStatusCompleted = 3
	};

private:
	STHttpClient*				iHttpClient;
	STHttpCacheFile*			iHttpCacheFile;		/**< 线程间共享，需要加锁 */
	STReadStatus				iReadStatus;		/**< 线程间共享，需要加锁 */
	STSemaphore					iSemaphore;
	STThread					iThreadHandle;
	STThread* const 			iDecodeThreadHandle;
	STChar*						iUrl;
	STChar*						iCachePath;
	STBool						iAborted;
};

#endif
