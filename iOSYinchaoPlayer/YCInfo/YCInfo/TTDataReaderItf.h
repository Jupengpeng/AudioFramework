#ifndef __TT_DATA_READER_H__
#define __TT_DATA_READER_H__

#include "TTTypedef.h"
#include "TTInterface.h"
#include "TTOsalConfig.h"

#define TTREADER_CACHE_SYNC		0x00000001
#define TTREADER_CACHE_ASYNC	0x00000002

class ITTDataReader : public ITTInterface
{
public:
	enum TTDataReaderId
	{
		ETTDataReaderIdNone
		,ETTDataReaderIdFile
		,ETTDataReaderIdHttp       
		,ETTDataReaderIdIPodLibraryFile
        ,ETTDataReaderIdExtAudioFile
		,ETTDataReaderIdBuffer
	};
public:

	/**
	* \fn                       TTInt Open(const TTChar* aUrl);
	* \brief                    打开文件
	* \param[in]	aUrl	    文件路径
	* \return					操作状态
	*/
	virtual TTInt				Open(const TTChar* aUrl) = 0;

	/**
	* \fn						TTInt Close()
	* \brief                    关闭文件
	* \return					操作状态
	*/
	virtual TTInt				Close() = 0;

	/**
	* \fn						void CloseConnection()
	* \brief                    关闭网络连接	
	*/
    virtual void                CloseConnection() = 0; 

	/**
	* \fn                       TTInt ReadSync(TTUint8* aReadBuffer, TTInt aReadPos, TTInt aReadSize);
	* \brief                    读取文件
	* \param[in]	aReadBuffer	存放读出数据的缓冲区
	* \param[in]	aReadPos	读取数据在文件中的偏移位置
	* \param[in]	aReadSize	读取的字节数
	* \return					读取正确时返回实际读取的字节数，否则为错误码
	*/
	virtual TTInt				ReadSync(TTUint8* aReadBuffer, TTInt aReadPos, TTInt aReadSize) = 0;

	/**
	* \fn                       TTInt ReadWait(TTUint8* aReadBuffer, TTInt aReadPos, TTInt aReadSize);
	* \brief                    等待读取文件
	* \param[in]	aReadBuffer	存放读出数据的缓冲区
	* \param[in]	aReadPos	读取数据在文件中的偏移位置
	* \param[in]	aReadSize	读取的字节数
	* \return					读取正确时返回实际读取的字节数，否则为错误码
	*/
	virtual TTInt				ReadWait(TTUint8* aReadBuffer, TTInt aReadPos, TTInt aReadSize) = 0;

	/**
	* \fn						TTInt Size() const;
	* \brief                    查询文件大小
	* \return					文件字节数
	*/
	virtual TTInt				Size() const = 0;


	/**
	* \fn						TTUint16 ReadUint16(TTInt aReadPos);
	* \brief                    读取一个16为的无符号整数 按高字节在后面读
	* \param[in]	aReadPos	文件的偏移位置
	* \return					返回整数值，超过范围会Assert
	*/
	virtual TTUint16			ReadUint16(TTInt aReadPos) = 0;// 按高字节在后面读

	/**
	* \fn						TTUint16 ReadUint16BE(TTInt aReadPos);
	* \brief                    读取一个16为的无符号整数,按高字节在前面读
	* \param[in]	aReadPos	文件的偏移位置
	* \return					返回整数值，超过范围会Assert
	*/
	virtual TTUint16			ReadUint16BE(TTInt aReadPos) = 0;// 按高字节在前面读

	/**
	* \fn						TTUint32 ReadUint32(TTInt aReadPos);
	* \brief                    读取一个32为的无符号整数,按高字节在后面读
	* \param[in]	aReadPos	文件的偏移位置
	* \return					返回整数值，超过范围会Assert
	*/
	virtual TTUint32			ReadUint32(TTInt aReadPos) = 0;

	/**
	* \fn						TTUint32 ReadUint32BE(TTInt aReadPos);
	* \brief                    读取一个32为的无符号整数,按高字节在前面读
	* \param[in]	aReadPos	文件的偏移位置
	* \return					返回整数值，超过范围会Assert
	*/
	virtual TTUint32			ReadUint32BE(TTInt aReadPos) = 0;

	/**
	* \fn						TTUint64 ReadUint64(TTInt aReadPos);
	* \brief                    读取一个64为的无符号整数,按高字节在后面读
	* \param[in]	aReadPos	文件的偏移位置
	* \return					返回整数值，超过范围会Assert
	*/
	virtual TTUint64			ReadUint64(TTInt aReadPos) = 0;

	/**
	* \fn						TTUint64 ReadUint64BE(TTInt aReadPos);
	* \brief                    读取一个64为的无符号整数,按高字节在前面读
	* \param[in]	aReadPos	文件的偏移位置
	* \return					返回整数值，超过范围会Assert
	*/
	virtual TTUint64			ReadUint64BE(TTInt aReadPos) = 0;

	/**
	* \fn						TTDataReaderId		Id()
	* \brief                    获取DataReader实例的Id
	* \return					DataReader实例的Id
	*/
	virtual TTDataReaderId		Id() = 0;
    
    /**
     * \fn						CheckOnLineBuffering	()
     * \brief
     */
	virtual void                CheckOnLineBuffering() = 0;

	virtual void	            SetDownSpeed(TTInt aFast) = 0;

	virtual TTUint              BufferedSize() = 0;

	virtual TTUint              BandWidth() = 0;

	virtual TTUint              BandPercent() = 0;

	virtual TTUint              ProxySize() = 0;

	virtual TTInt				GetStatusCode() = 0;
	virtual TTUint				GetHostIP() = 0;


    /**
     * \fn						PrepareCache()
     * \brief
     */
	virtual TTInt               PrepareCache(TTInt aReadPos, TTInt aReadSize, TTInt aFlag) = 0;

	/**
     * \fn						SetBitrate	()
     * \brief
     */
	virtual void                SetBitrate(TTInt aMediaType, TTInt aBitrate) = 0;


	virtual void				SetNetWorkProxy(TTBool aNetWorkProxy) = 0;
};

#endif
