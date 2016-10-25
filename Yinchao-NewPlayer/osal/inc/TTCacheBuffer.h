#ifndef __TT_CACHE_BUFFER_H__
#define __TT_CACHE_BUFFER_H__
#include <stdio.h>
#include <string.h>
#include "TTOsalConfig.h"
#include "TTMacrodef.h"
#include "TTTypedef.h"
#include "TTCritical.h"

#define PAGE_BUFFER_COUNT	20
#define PAGE_BUFFER_SIZE	1024*1024

typedef struct
{
	TTInt64		iPosition;
	TTInt		iTotalSize;
	TTInt		iSize;
	TTUint8*	iBuffer;
	TTInt		iFlag;
} TTBufferPage;

class CTTCacheBuffer
{
public:

	/**
	* \fn							CTTCacheBuffer();
	* \brief						构造函数
	*/
	CTTCacheBuffer();

	/**
	* \fn							~CTTCacheBuffer();
	* \brief						析构函数
	*/
	~CTTCacheBuffer();

	/**
	* \fn							TTInt Read(TTChar* aBuffer, TTInt aReadPos, TTInt aReadSize);
	* \brief						读取文件
	* \param[in]	aBuffer			存放读出数据的缓冲区
	* \param[in]	aReadPos		读取数据在文件中的偏移位置
	* \param[in]	aReadSize		读取的字节数
	* \return						读取正确时返回实际读取的字节数，否则为错误码
	*/
	TTInt							Read(TTUint8* aBuffer, TTInt aReadPos, TTInt aReadSize);
	
	/**
	* \fn							TTInt Write(TTChar* aBuffer, TTInt aWriteSize);
	* \brief						写文件
	* \param[in]	aBuffer			用于存放要写入文件的数据的Buffer
	* \param[in]	aWriteSize		字节数
	* \return						操作状态
	*/
	TTInt							Write(TTUint8* aBuffer, TTInt aWritePos, TTInt aWriteSize);
	
	/**
	* \fn							TTInt TotalSize();
	* \brief						获取文件总大小
	* \return						文件总大小
	*/
	INLINE_C TTInt					TotalSize(){return iTotalSize;};

	INLINE_C TTInt					TotalCount(){return iCachedCount;};

	/**
	* \fn							TTInt CachePoistion();
	* \brief						获取已缓存数据的大小
	* \return						已缓存数据的大小
	*/
	TTInt							CachePoistion(TTInt64& aCacheBegin, TTInt64& aCacheEnd);

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
	TTInt							Open();
	/**
	* \fn							void Close();
	* \brief						关闭文件
	*/
	void							Close();

private:
	void							InitPage();
	void							UnInitPage();
		
private:
	TTInt64				iTotalSize;
	TTInt64				iCachedCount;
	RTTCritical			iCritical;

	TTInt				iIndexStart; 
	TTInt				iIndexEnd;
	TTBufferPage		iPage[PAGE_BUFFER_COUNT];
};
#endif
