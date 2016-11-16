#ifndef __ST_DATA_READER_ITF__H__
#define __ST_DATA_READER_ITF__H__

class ISTDataReaderItf
{   
public:
	enum DataReaderId
	{
		EDataReaderIdNone
		, EDataReaderIdLoacal
		, EDataReaderIdHttp
	};


public:
	/**
	* \fn                       STInt Open(const char* aFileName);
	* \brief                    打开文件
	* \param[in]	aUrl	    路径
	* \return					操作状态
	*/
	virtual int					Open(const char* aUrl) = 0;

	/**
	* \fn						STInt Close()
	* \brief                    关闭文件
	* \return					操作状态
	*/
	virtual int					Close() = 0;

	/**
	* \fn                       STInt Read(unsigned char* aReadBuffer, int aReadPos, int aReadSize);
	* \brief                    读取文件
	* \param[in]	aReadBuffer	存放读出数据的缓冲区
	* \param[in]	aReadPos	读取数据在文件中的偏移位置
	* \param[in]	aReadSize	读取的字节数
	* \return					读取正确时返回实际读取的字节数，否则为错误码
	*/
	virtual int					Read(unsigned char* aReadBuffer, int aReadPos, int aReadSize) = 0;

	/**
	* \fn						STInt Size() const;
	* \brief                    查询文件大小
	* \return					文件字节数
	*/
	virtual int					Size() const = 0;

	/**
	* \fn						STDataReaderId Id()
	* \brief                    获取data reader的ID
	* \return					ID
	*/
	virtual DataReaderId		Id() = 0;

	/**							void Abort()
	*							取消操作
	*/
	virtual void				Abort(){};
};

#endif
