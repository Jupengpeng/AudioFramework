#ifndef __ST_FILE_READER__H__
#define __ST_FILE_READER__H__
#include <stdio.h>
#include "STDataReaderItf.h"

// CLASSES DECLEARATION
class STFileReader: public ISTDataReaderItf
{   
public:
	STFileReader();
	virtual ~STFileReader();

public:
	/**
	* \fn                       STInt Open(const STChar* aFileName);
	* \brief                    打开文件
	* \param[in]	aUrl	    路径
	* \return					操作状态
	*/
	int							Open(const char* aUrl);

	/**
	* \fn						STInt Close()
	* \brief                    关闭文件
	* \return					操作状态
	*/
	int							Close();

	/**
	* \fn                       STInt Read(STUint8* aReadBuffer, STInt aReadPos, STInt aReadSize);
	* \brief                    读取文件
	* \param[in]	aReadBuffer	存放读出数据的缓冲区
	* \param[in]	aReadPos	读取数据在文件中的偏移位置
	* \param[in]	aReadSize	读取的字节数
	* \return					读取正确时返回实际读取的字节数，否则为错误码
	*/
	int							Read(unsigned char* aReadBuffer, int aReadPos, int aReadSize);

	/**
	* \fn						STInt Size() const;
	* \brief                    查询文件大小
	* \return					文件字节数
	*/
	int							Size() const;

	/**
	* \fn						ISTDataReaderItf::STDataReaderId Id()
	* \brief                    获取data reader的ID
	* \return					ID
	*/

	/**
	*add by bin.liu
	*\fn							void SetCachePath(const STChar* aUrl)
	*\param[in]  aUrl				获得当前已经下载了文件的百分比
	*/
	int							GetDownloadPercent();

	ISTDataReaderItf::DataReaderId Id();

private:
	int							CheckReadInt(int aReadPos, int aIntSize, int& aIntOffset);//读取整数的验证是否超出范围或者将数据读入内存
	int							CheckPreRead(unsigned char* aReadBuffer, int aReadPos, int aReadSize);
	int							ReadFile(unsigned char* aReadBuffer, int aReadPos, int aReadSize);
	void						PreRead(int aReadPos);


private:
	FILE*						iFile;		/**< 操作文件的文件指针*/
	int							iFileSize;	/**<  文件大小*/

	unsigned char*				iPreReadBuffer;/**< 预读文件用的Buffer，为了减少读文件的次数*/
	int							iCurPreReadBufferPos;/**< 预读Buffer数据在文件中的位置*/
};
#endif
