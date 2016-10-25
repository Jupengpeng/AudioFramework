#ifndef __TT_FILE_READER__H__
#define __TT_FILE_READER__H__

#include "TTBaseDataReader.h"

// CLASSES DECLEARATION
class CTTFileReader : public CTTBaseDataReader
{ 
public:    
    static TTBool              IsSourceValid(const TTChar* aUrl);
    
public:

	/**
	* \fn                       CTTFileReader(TTBool aAsyncReaderEnable)
	* \brief                    构造函数
	* \param[in]  aAsyncReaderEnable 是否使用异步读取，这时候会使用到AO机制。注意。
	*/
	CTTFileReader();
	
	/**
	* \fn                       ~CTTFileReader()
	* \brief                    构造函数
	*/
	virtual ~CTTFileReader();

	/**
	* \fn                       TTInt Open(const TTChar* aFileName);
	* \brief                    打开文件
	* \param[in]	aUrl	    路径
	* \return					操作状态
	*/
	virtual	TTInt				Open(const TTChar* aUrl);
	
	/**
	* \fn						TTInt Close()
	* \brief                    关闭文件
	* \return					操作状态
	*/
	virtual	TTInt				Close();
	
	/**
	* \fn                       TTInt ReadSync(TTUint8* aReadBuffer, TTInt aReadPos, TTInt aReadSize);
	* \brief                    读取文件
	* \param[in]	aReadBuffer	存放读出数据的缓冲区
	* \param[in]	aReadPos	读取数据在文件中的偏移位置
	* \param[in]	aReadSize	读取的字节数
	* \return					读取正确时返回实际读取的字节数，否则为错误码
	*/
	virtual	TTInt				ReadSync(TTUint8* aReadBuffer, TTInt aReadPos, TTInt aReadSize);

	/**
	* \fn						TTInt Size() const;
	* \brief                    查询文件大小
	* \return					文件字节数
	*/
	virtual	TTInt				Size() const;

	virtual	TTUint				BufferedSize();

	/**
	* \fn						TTInt ReadWait(TTUint8* aReadBuffer, TTInt aReadPos, TTInt aReadSize);
	* \brief                    等待读取文件
	* \param[in]	aReadBuffer	存放读出数据的缓冲区
	* \param[in]	aReadPos	读取数据在文件中的偏移位置
	* \param[in]	aReadSize	读取的字节数
	*/
	virtual	TTInt				ReadWait(TTUint8* aReadBuffer, TTInt aReadPos, TTInt aReadSize);

	/**
	* \fn						TTUint16 ReadUint16(TTInt aReadPos);
	* \brief                    读取一个16为的无符号整数 按高字节在后面读
	* \param[in]	aReadPos	文件的偏移位置
	* \return					返回整数值，超过范围会Assert
	*/
	virtual TTUint16			ReadUint16(TTInt aReadPos);// 按高字节在后面读

	/**
	* \fn						TTUint16 ReadUint16BE(TTInt aReadPos);
	* \brief                    读取一个16为的无符号整数,按高字节在前面读
	* \param[in]	aReadPos	文件的偏移位置
	* \return					返回整数值，超过范围会Assert
	*/
	virtual TTUint16			ReadUint16BE(TTInt aReadPos);// 按高字节在前面读

	/**
	* \fn						TTUint32 ReadUint32(TTInt aReadPos);
	* \brief                    读取一个32为的无符号整数,按高字节在后面读
	* \param[in]	aReadPos	文件的偏移位置
	* \return					返回整数值，超过范围会Assert
	*/
	virtual TTUint32			ReadUint32(TTInt aReadPos);

	/**
	* \fn						TTUint32 ReadUint32BE(TTInt aReadPos);
	* \brief                    读取一个32为的无符号整数,按高字节在前面读
	* \param[in]	aReadPos	文件的偏移位置
	* \return					返回整数值，超过范围会Assert
	*/
	virtual TTUint32			ReadUint32BE(TTInt aReadPos);

	/**
	* \fn						TTUint64 ReadUint64(TTInt aReadPos);
	* \brief                    读取一个64为的无符号整数,按高字节在后面读
	* \param[in]	aReadPos	文件的偏移位置
	* \return					返回整数值，超过范围会Assert
	*/
	virtual TTUint64			ReadUint64(TTInt aReadPos);

	/**
	* \fn						TTUint64 ReadUint64BE(TTInt aReadPos);
	* \brief                    读取一个64为的无符号整数,按高字节在前面读
	* \param[in]	aReadPos	文件的偏移位置
	* \return					返回整数值，超过范围会Assert
	*/
	virtual TTUint64			ReadUint64BE(TTInt aReadPos);

	/**
	* \fn						TTDataReaderId		Id()
	* \brief                    获取DataReader实例的Id
	* \return					DataReader实例的Id
	*/
	virtual TTDataReaderId		Id();

	virtual TTInt                PrepareCache(TTInt aReadPos, TTInt aReadSize, TTInt aFlag);


//protected:
//	virtual void				RunL();


private:

	TTInt						CheckReadInt(TTInt aReadPos, TTInt aIntSize, TTInt& aIntOffset);//读取整数的验证是否超出范围或者将数据读入内存
	TTInt						CheckPreRead(TTUint8* aReadBuffer, TTInt aReadPos, TTInt aReadSize);
	TTInt						Read(TTUint8* aReadBuffer, TTInt aReadPos, TTInt aReadSize);
	void						PreRead(TTInt aReadPos);


private:

	FILE*						iFile;		/**< 操作文件的文件指针*/
	TTInt						iFileSize;	/**<  文件大小*/

	//TTInt						iAsyncReadSize;
	//TTUint8*					iAsyncReadBuffer;
	//TTInt						iAsyncReadPos;

	TTUint8*					iPreReadBuffer;/**< 预读文件用的Buffer，为了减少读文件的次数*/
	TTInt						iCurPreReadBufferPos;/**< 预读Buffer数据在文件中的位置*/
};
#endif
