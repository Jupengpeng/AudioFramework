#ifndef __TT_HTTP_READER__H__
#define __TT_HTTP_READER__H__

// INCLUDES
#include "TTBaseDataReader.h"
#include "TTHttpReaderProxy.h"

// CLASSES DECLEARATION
class CTTHttpReader : public CTTBaseDataReader
{ 
public:
	/**
	* \fn                       CTTHttpReader()
	* \brief                    构造函数
	*/
	CTTHttpReader();

	/**
	* \fn                       ~CTTHttpReader()
	* \brief                    析构函数
	*/
	virtual ~CTTHttpReader();

	/**
	* \fn                       TTInt Open(const TTChar* aUrl);
	* \brief                    打开文件
	* \param[in]	aUrl		路径
	* \return					返回操作状态
	*/
	virtual	TTInt				Open(const TTChar* aUrl);

	/**
	* \fn						TTInt Close()
	* \brief                    关闭文件
	* \return					操作状态
	*/
	virtual	TTInt				Close();

	/**
	* \fn						void CloseConnection()
	* \brief                    关闭网络连接
	*/
	virtual	void				CloseConnection();

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

	virtual TTUint              BandWidth();
    
    /**
     * \fn						CheckOnLineBuffering	()
     * \brief
     */
	virtual void                CheckOnLineBuffering();

	virtual void	            SetDownSpeed(TTInt aFast);
	/**
     * \fn						SetBitrate	()
     * \brief
     */
	virtual void                SetBitrate(TTInt aMediaType, TTInt aBitrate);

	virtual TTInt               GetStatusCode();

	virtual TTUint				GetHostIP();


	virtual void				SetNetWorkProxy(TTBool aNetWorkProxy);


	virtual TTInt                PrepareCache(TTInt aReadPos, TTInt aReadSize, TTInt aFlag);
public:
	/**
	* \fn						TTUint BufferedSize()
	* \brief                    获取缓冲大小
	* \return					缓冲大小
	*/
	virtual	TTUint				BufferedSize();

	virtual TTUint              ProxySize();

	void						SetStreamBufferingObserver(ITTStreamBufferingObserver* aObserver);

private:
	CTTHttpReaderProxy*			iHttpReaderProxy;
};

#endif
