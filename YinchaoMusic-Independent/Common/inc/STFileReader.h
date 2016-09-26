#ifndef __ST_FILE_READER__H__
#define __ST_FILE_READER__H__
#include <stdio.h>
#include "STTypedef.h"
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
	STInt						Open(const STChar* aUrl);
	
	/**
	* \fn						STInt Close()
	* \brief                    关闭文件
	* \return					操作状态
	*/
	STInt						Close();
	
	/**
	* \fn                       STInt Read(STUint8* aReadBuffer, STInt aReadPos, STInt aReadSize);
	* \brief                    读取文件
	* \param[in]	aReadBuffer	存放读出数据的缓冲区
	* \param[in]	aReadPos	读取数据在文件中的偏移位置
	* \param[in]	aReadSize	读取的字节数
	* \return					读取正确时返回实际读取的字节数，否则为错误码
	*/
	STInt						Read(STUint8* aReadBuffer, STInt aReadPos, STInt aReadSize);

	/**
	* \fn						STInt Size() const;
	* \brief                    查询文件大小
	* \return					文件字节数
	*/
	STInt						Size() const;

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
	STInt							GetDownloadPercent();

	ISTDataReaderItf::DataReaderId Id();

private:
	STInt						CheckReadInt(STInt aReadPos, STInt aIntSize, STInt& aIntOffset);//读取整数的验证是否超出范围或者将数据读入内存
	STInt						CheckPreRead(STUint8* aReadBuffer, STInt aReadPos, STInt aReadSize);
	STInt						ReadFile(STUint8* aReadBuffer, STInt aReadPos, STInt aReadSize);
	void						PreRead(STInt aReadPos);


private:
	FILE*						iFile;		/**< 操作文件的文件指针*/
	STInt						iFileSize;	/**<  文件大小*/

	STUint8*					iPreReadBuffer;/**< 预读文件用的Buffer，为了减少读文件的次数*/
	STInt						iCurPreReadBufferPos;/**< 预读Buffer数据在文件中的位置*/
};
#endif
