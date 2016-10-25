/**
* File : TTExtAudioFileReader.h  
* Created on : 2011-12-6
* Author : hu.cao
* Copyright : Copyright (c) 2011 Shuidushi Software Ltd. All rights reserved.
* Description : CTTExtAudioFileReader定义文件
*/
#ifndef __TT_EXT_AUDIO_FILE_READER_H__
#define __TT_EXT_AUDIO_FILE_READER_H__

// INCLUDES
#include "TTBaseDataReader.h"
#import <AudioToolbox/AudioToolbox.h>

static const int KReadSizePerTime = 40 * KILO;

// CLASSES DECLEARATION
class CTTExtAudioFileReader : public CTTBaseDataReader
{ 
public:
    static TTBool               IsSourceValid(const TTChar* aUrl);

public:
    
    TTInt                       GetSampleRate();
    TTInt                       GetChannels();
    TTUint                      Duration();
    TTInt64                     TotalFrame();
    
    TTInt                       Seek(TTUint aFrameIndex);

	/**
	* \fn                       CTTExtAudioFileReader()
	* \brief                    构造函数
	*/
	CTTExtAudioFileReader();

	/**
	* \fn                       ~CTTExtAudioFileReader()
	* \brief                    构造函数
	*/
	virtual ~CTTExtAudioFileReader();

	/**
	* \fn                       TTInt Open(const TTChar* aUrl);
	* \brief                    打开文件
	* \param[in]	aUrl		路径
	* \return					操作状态
	*/
	TTInt						Open(const TTChar* aUrl);

	/**
	* \fn						TTInt Close()
	* \brief                    关闭文件
	* \return					操作状态
	*/
	TTInt						Close();
    
    virtual void                CloseConnection(){};

	/**
	* \fn                       TTInt ReadSync(TTUint8* aReadBuffer, TTInt aReadPos, TTInt aReadSize);
	* \brief                    读取文件
	* \param[in]	aReadBuffer	存放读出数据的缓冲区
	* \param[in]	aReadPos	读取数据在文件中的偏移位置
	* \param[in]	aReadSize	读取的字节数
	* \return					读取正确时返回实际读取的字节数，否则为错误码
	*/
	TTInt						ReadSync(TTUint8* aReadBuffer, TTInt aReadPos, TTInt aReadSize);

	/**
	* \fn						TTInt Size() const;
	* \brief                    查询文件大小
	* \return					文件字节数
	*/
	TTInt						Size() const;

	/**
	* \fn						void ReadAsync(TTUint8* aReadBuffer, TTInt aReadPos, TTInt aReadSize);
	* \brief                    异步读取文件
	* \param[in]	aReadBuffer	存放读出数据的缓冲区
	* \param[in]	aReadPos	读取数据在文件中的偏移位置
	* \param[in]	aReadSize	读取的字节数
	*/
	TTInt						ReadWait(TTUint8* aReadBuffer, TTInt aReadPos, TTInt aReadSize);

	/**
	* \fn						TTDataReaderId		Id()
	* \brief                    获取DataReader实例的Id
	* \return					DataReader实例的Id
	*/
	virtual TTDataReaderId		Id();
    
    virtual void                CheckOnLineBuffering() {};
    
    
    virtual TTUint              BufferedSize() {return 0;};
    
    
    virtual TTUint              ProxySize() {return 0;};
    
    virtual TTUint16 ReadUint16(TTInt aReadPos);
    
    virtual TTUint16 ReadUint16BE(TTInt aReadPos);
    
    virtual TTUint32 ReadUint32(TTInt aReadPos);
    
    virtual TTUint32 ReadUint32BE(TTInt aReadPos);
    
    virtual TTUint64 ReadUint64(TTInt aReadPos);
    
    virtual TTUint64 ReadUint64BE(TTInt aReadPos);
    
    /**
     * \fn						PrepareCache()
     * \brief
     */
    virtual TTInt               PrepareCache(TTInt aReadPos, TTInt aReadSize, TTInt aFlag);


	//virtual void				RunL();
    
private:
    TTInt                       StartReading();
private: 
    ExtAudioFileRef             iCurAudioFile;
    AudioBufferList             iCurBufferList;
    TTInt                       iSampleRate;
    TTInt                       iChannels;
    TTInt                       iDuration;
    TTBool                      iReadStarted;
    TTInt64                     iTotalFrameNum;
};
#endif
