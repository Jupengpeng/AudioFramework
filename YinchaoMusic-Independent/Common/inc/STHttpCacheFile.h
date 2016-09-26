/**
* File : TTHttpCacheFile.h  
* Created on : 2012-3-19
* Author : hu.cao
* Copyright : Copyright (c) 2011 Shuidushi Software Ltd. All rights reserved.
* Description : STHttpCacheFile定义文件,用于管理文件文件的存储于读取
*/
#ifndef __ST_HTTP_FILE_CACHE_H__
#define __ST_HTTP_FILE_CACHE_H__
#include <stdio.h>
#include "STOSConfig.h"
#include "STMacrodef.h"
#include "STTypedef.h"
#include "STCritical.h"

class STHttpCacheFile
{
public:

	/**
	* \fn							STHttpCacheFile();
	* \brief						构造函数
	*/
	STHttpCacheFile();

	/**
	* \fn							~STHttpCacheFile();
	* \brief						析构函数
	*/
	~STHttpCacheFile();

	/**
	* \fn							STInt Read(void* aBuffer, STInt aReadPos, STInt aReadSize);
	* \brief						读取文件
	* \param[in]	aBuffer			存放读出数据的缓冲区
	* \param[in]	aReadPos		读取数据在文件中的偏移位置
	* \param[in]	aReadSize		读取的字节数
	* \return						读取正确时返回实际读取的字节数，否则为错误码
	*/
	STInt							Read(void* aBuffer, STInt aReadPos, STInt aReadSize);
	
	/**
	* \fn							STInt Write(void* aBuffer, STInt aWriteSize);
	* \brief						写文件
	* \param[in]	aBuffer			用于存放要写入文件的数据的Buffer
	* \param[in]	aWriteSize		字节数
	* \return						操作状态
	*/
	STInt							Write(void* aBuffer, STInt aWriteSize);
	
	/**
	* \fn							STInt TotalSize();
	* \brief						获取文件总大小
	* \return						文件总大小
	*/
	STInt							TotalSize();

	/**
	* \fn							STInt CachedSize();
	* \brief						获取已缓存数据的大小
	* \return						已缓存数据的大小
	*/
	STInt							CachedSize();

		/**
	* \fn							STInt CachedSize();
	* \brief						获取已缓存数据的大小
	* \return						已缓存数据的大小
	*/
	STInt							CachedPercent();

	/**
	* \fn							void SetTotalSize(STInt aTotalSize);
	* \brief						设置文件总大小
	* \param[in]  aTotalSize		文件总大小
	*/
	void							SetTotalSize(STInt aTotalSize);
	
	/**
	* \fn							STInt Open(const STChar* aUrl);
	* \brief						打开文件，用于检查是否已经缓存了这个文件
	* \param[in]  aUrl				路径
	* \return						操作码
	*/
	STInt							Open(const STChar* aUrl);


	/**
	* \fn							STInt Create(const STChar* aUrl);
	* \brief						创建文件
	* \param[in]  aUrl				Url路径
	* \return						操作码
	*/
	STInt							Create(const STChar* aUrl);

	/**
	* \fn							void Close();
	* \brief						关闭文件
	*/
	void							Close();

		
private:
	FILE*				iFileHandle;
	STInt				iTotalSize;
	STInt				iCachedSize;
	STCritical			iCritical;           
};
#endif
