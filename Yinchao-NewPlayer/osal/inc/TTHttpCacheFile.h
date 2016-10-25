#ifndef __TT_HTTP_FILE_CACHE_H__
#define __TT_HTTP_FILE_CACHE_H__
#include <stdio.h>
#include <string.h>
#include "TTOsalConfig.h"
#include "TTMacrodef.h"
#include "TTTypedef.h"
#include "TTCritical.h"

#define BUFFER_UNIT_SIZE		(32*1024)
#define BUFFER_UNIT_MAXSIZE		(16*32*BUFFER_UNIT_SIZE)

typedef struct
{
	TTInt64		iPosition;
	TTInt		iTotalSize;
	TTInt		iSize;
	TTUint8*	iBuffer;
	TTInt		iFlag;
} TTBufferUnit;

class CTTHttpCacheFile
{
public:

	/**
	* \fn							CTTHttpCacheFile();
	* \brief						构造函数
	*/
	CTTHttpCacheFile();

	/**
	* \fn							~CTTHttpCacheFile();
	* \brief						析构函数
	*/
	~CTTHttpCacheFile();

	/**
	* \fn							TTInt Read(void* aBuffer, TTInt aReadPos, TTInt aReadSize);
	* \brief						读取文件
	* \param[in]	aBuffer			存放读出数据的缓冲区
	* \param[in]	aReadPos		读取数据在文件中的偏移位置
	* \param[in]	aReadSize		读取的字节数
	* \return						读取正确时返回实际读取的字节数，否则为错误码
	*/
	TTInt							Read(void* aBuffer, TTInt aReadPos, TTInt aReadSize);
	
	/**
	* \fn							TTInt Write(void* aBuffer, TTInt aWriteSize);
	* \brief						写文件
	* \param[in]	aBuffer			用于存放要写入文件的数据的Buffer
	* \param[in]	aWriteSize		字节数
	* \return						操作状态
	*/
	TTInt							Write(void* aBuffer, TTInt aWriteSize);


	/**
	* \fn							TTInt ReadBuffer(void* aBuffer, TTInt aReadPos, TTInt aReadSize);
	* \brief						读取文件
	* \param[in]	aBuffer			存放读出数据的缓冲区
	* \param[in]	aReadPos		读取数据在文件中的偏移位置
	* \param[in]	aReadSize		读取的字节数
	* \return						读取正确时返回实际读取的字节数，否则为错误码
	*/
	TTInt							ReadBuffer(TTUint8* aBuffer, TTInt aReadPos, TTInt aReadSize);
	
	/**
	* \fn							TTInt WriteBuffer(void* aBuffer, TTInt aWriteSize);
	* \brief						写文件
	* \param[in]	aBuffer			用于存放要写入文件的数据的Buffer
	* \param[in]	aWriteSize		字节数
	* \return						操作状态
	*/
	TTInt							WriteBuffer(TTUint8* aBuffer, TTInt aWriteSize);
	
	/**
	* \fn							TTInt TotalSize();
	* \brief						获取文件总大小
	* \return						文件总大小
	*/
	INLINE_C TTInt					TotalSize(){return iTotalSize;};

	/**
	* \fn							TTInt CachedSize();
	* \brief						获取已缓存数据的大小
	* \return						已缓存数据的大小
	*/
	TTInt							CachedSize();

	/**
	* \fn							void SetTotalSize(TTInt aTotalSize);
	* \brief						设置文件总大小
	* \param[in]  aTotalSize		文件总大小
	*/
	INLINE_C	void				SetTotalSize(TTInt aTotalSize){iTotalSize = aTotalSize;};
	
	/**
	* \fn							TTInt Open(const TTChar* aUrl);
	* \brief						打开文件，用于检查是否已经缓存了这个文件
	* \param[in]  aUrl				路径
	* \return						操作码
	*/
	TTInt							Open(const TTChar* aUrl);


	/**
	* \fn							TTInt Create(const TTChar* aUrl, TTInt aTotalSize);
	* \brief						创建文件
	* \param[in]  aUrl				Url路径
	* \param[in]  aTotalSize		文件总大小
	* \return						操作码
	*/
	TTInt							Create(const TTChar* aUrl, TTInt aTotalSize);

	/**
	* \fn							TTInt WriteFile();
	* \brief						写文件
	* \return						操作码
	*/
	TTInt							WriteFile(TTInt aCount);

	/**
	* \fn							void Close();
	* \brief						关闭文件
	*/
	void							Close();

private:
	TTInt							InitBufferUnit(int nSize);
	TTInt							UnInitBufferUnit();
		
private:
	FILE*				iFileHandle;
	TTInt				iTotalSize;
	TTInt				iCachedSize;
	TTInt				iIndexEnd;
	TTInt				iCurIndex;
	TTInt				iBufferMode;
	TTBufferUnit**		iBufferUnit;
	TTUint8*			iWriteBuffer;
	TTInt				iBufferCount;
	TTInt				iWriteCount;
	TTInt				iWriteIndex;
	RTTCritical			iCritical;
};
#endif
