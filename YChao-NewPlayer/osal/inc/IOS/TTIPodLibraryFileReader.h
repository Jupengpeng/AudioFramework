/**
* File : TTIPodLibraryFileReader.h  
* Created on : 2011-8-31
* Author : hu.cao
* Copyright : Copyright (c) 2011 Shuidushi Software Ltd. All rights reserved.
* Description : CTTIPodLibraryFileReader定义文件
*/
#ifndef __TT_IPOD_LIBRARY_FILE_READER_H__
#define __TT_IPOD_LIBRARY_FILE_READER_H__

// INCLUDES
#include "TTBaseDataReader.h"

// CLASSES DECLEARATION
class CTTIPodLibraryFileReader : public CTTBaseDataReader
{ 
public:
    static TTBool               IsSourceValid(const TTChar* aUrl);

public:
    
    TTInt                       GetSampleRate();
    TTInt                       GetChannels();
    TTUint                      Duration();
    
    TTInt                       Seek(TTUint aTime);

	/**
	* \fn                       CTTIPodLibraryFileReader()
	* \brief                    构造函数
	*/
	CTTIPodLibraryFileReader();

	/**
	* \fn                       ~CTTIPodLibraryFileReader()
	* \brief                    构造函数
	*/
	virtual ~CTTIPodLibraryFileReader();

	/**
	* \fn                       TTInt Open(const TTChar* aUrl);
	* \brief                    打开文件
	* \param[in]	aUrl		路径
	* \return					操作状态
	*/
	virtual TTInt			   Open(const TTChar* aUrl);

	/**
	* \fn						TTInt Close()
	* \brief                    关闭文件
	* \return					操作状态
	*/
	virtual  TTInt				Close();
    
    virtual void                CloseConnection(){};

	/**
	* \fn                       TTInt ReadSync(TTUint8* aReadBuffer, TTInt aReadPos, TTInt aReadSize);
	* \brief                    读取文件
	* \param[in]	aReadBuffer	存放读出数据的缓冲区
	* \param[in]	aReadPos	读取数据在文件中的偏移位置
	* \param[in]	aReadSize	读取的字节数
	* \return					读取正确时返回实际读取的字节数，否则为错误码
	*/
	virtual TTInt				 ReadSync(TTUint8* aReadBuffer, TTInt aReadPos, TTInt aReadSize);

	/**
	* \fn						TTInt Size() const;
	* \brief                    查询文件大小
	* \return					文件字节数
	*/
	virtual TTInt				  Size() const;

	/**
	* \fn						void ReadAsync(TTUint8* aReadBuffer, TTInt aReadPos, TTInt aReadSize);
	* \brief                    异步读取文件
	* \param[in]	aReadBuffer	存放读出数据的缓冲区
	* \param[in]	aReadPos	读取数据在文件中的偏移位置
	* \param[in]	aReadSize	读取的字节数
	*/
	virtual TTInt				  ReadWait(TTUint8* aReadBuffer, TTInt aReadPos, TTInt aReadSize);

	/**
	* \fn						TTDataReaderId		Id()
	* \brief                    获取DataReader实例的Id
	* \return					DataReader实例的Id
	*/
	virtual TTDataReaderId		Id();
    
    virtual TTUint16 ReadUint16(TTInt aReadPos);
    
    virtual TTUint16 ReadUint16BE(TTInt aReadPos);
    
    virtual TTUint32 ReadUint32(TTInt aReadPos);
    
    virtual TTUint32 ReadUint32BE(TTInt aReadPos);
    
    virtual TTUint64 ReadUint64(TTInt aReadPos);
    
    virtual TTUint64 ReadUint64BE(TTInt aReadPos);
    
    
    virtual void                CheckOnLineBuffering() {};
    
    
    virtual TTUint              BufferedSize() {return 0;};
    
    
    virtual TTUint              ProxySize() {return 0;};
    
    virtual TTInt               PrepareCache(TTInt aReadPos, TTInt aReadSize, TTInt aFlag) ;
    
protected:
    void                        PreSampleRefRelease();
    TTInt                       SetStartReadPos(TTUint aTime);
    
private: 
    void*                       iAsset;
    void*                       iAssetReader;
    void*                       iAssetReaderOutput;
    void*                       iCurSampleBufferRef;
    void*                       iAudioArray;
    TTInt                       iDuration;
    TTBool                      iReadStarted;
    TTInt                       iSampleRate;
    TTInt                       iChannels;
    TTInt64                     iTotalReadSize;
};
#endif
