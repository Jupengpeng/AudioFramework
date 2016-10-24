#ifndef __ST_SAMPLE_BUFFER_H__
#define __ST_SAMPLE_BUFFER_H__
#include "STTypedef.h"
#include "STMacrodef.h"

class STSampleBuffer
{
public:
	/**
	* 构造函数
	* \param[in] aBufferSize		Buffer 大小
	* \param[in] aDataPtr			数据指针
	* \param[in] aIsFirstBuffer		是否为第一个Buffer
	*/
	STSampleBuffer(STInt aBufferSize, STUint8* aDataPtr, STBool aIsFirstBuffer);
	~STSampleBuffer();

public:
	/**
	* \fn				            STUint GetByteOffset()	
	* \brief				        获取字节偏移
	* \return						时间戳ms
	*/
	STInt							GetByteOffset();

	/**
	* \fn				            void SetByteOffset(STUint aTimeStamp)	
	* \brief				        设置Buffer时间戳
	* \param[in] aTimeStamp			时间戳ms
	*/
	void							SetByteOffset(STInt aTimeStamp);

	/**
	* \fn				            STUint GetPosition()	
	* \brief				        获取有效数据的起始位置
	* \return						有效数据的起始位置
	*/
	STInt							GetPosition();

	/**
	* \fn				            void SetPosition(STUint aPositon)	
	* \brief				        设置有效数据的起始位置
	* \param[in] aPositon			起始位置，相对于DataPtr的数据偏移
	*/
	void							SetPosition(STInt aPositon);
	
	/**
	* \fn				            STBool IsFirstBuffer()	
	* \brief				        是否为第一个Buffer
	* \return						第一个Buffer返回ESTrue
	*/
	STBool							IsFirstBuffer();
	
	/**
	* \fn				            void Reset()	
	* \brief				        清空时间戳和起始位置
	*/
	void							Reset();
	
	/**
	* \fn				            void Ptr()	
	* \brief				        获取缓冲区的指针
	*/
	STUint8*						Ptr();

	/**
	* \fn				            void Size()	
	* \brief				        获取缓冲区大小
	*/
	STInt							Size();

	/**
	* \fn				            void ValidSize()	
	* \brief				        获取有效数据的长度，总数据大小 - iPosition
	*/
	STInt							ValidSize();

	/**
	* \fn				            void SetStreamIndex(STInt aIndex)	
	* \brief				        设置流索引号
	* \param[in]	aIndex			流的索引号						
	*/
	void							SetStreamIndex(STInt aIndex);
	
	/**
	* \fn				            STInt GetStreamIndex()	
	* \brief				        设置流索引号
	* \return						流的索引号						
	*/
	STInt							GetStreamIndex();

private:	
	STUint8* const					iDataPtr;//内存起始指针
	const STInt						iSize;//buffer可以表示的内存大小即从ptr至ptr+iSize的大小
	STInt							iPosition;//相对启始ptr的数据偏移。
	STInt							iByteOffset;//该buffer在整个文件流的数据偏移？
	const STBool					iIsFirstBuffer;
	STInt							iStreamIndex;//buffer 所在的数据索引
};

#endif
