#ifndef __TT_BASE_DATA_READER__H__
#define __TT_BASE_DATA_READER__H__

// INCLUDES
#include <stdio.h>
#include "TTMacrodef.h"
#include "TTTypedef.h"
#include "TTDataReaderItf.h"
//#include "TTActive.h"

class CTTBaseDataReader : public ITTDataReader
{ 
public:

	/**
	* \fn                       CTTBaseDataReader(TTBool aAsyncReaderEnable = ETTTrue))
	* \brief                    构造函数
	* \param[in]  aAsyncReaderEnable 是否使用异步读取，这时候会使用到AO机制。注意。
	*/
	CTTBaseDataReader();

	/**
	* \fn                       ~CTTBaseDataReader()
	* \brief                    构造函数
	*/
	virtual ~CTTBaseDataReader();

	/**
	* \fn						void CloseConnection()
	* \brief                    关闭网络连接
	*/
	virtual void				CloseConnection(){};
    
    /**
     * \fn						CheckOnLineBuffering()
     * \brief
     */
	virtual void                CheckOnLineBuffering() {};

	virtual void	            SetDownSpeed(TTInt aFast) {};

	/**
     * \fn						SetBitrate	()
     * \brief
     */
	virtual void                SetBitrate(TTInt aMediaType, TTInt aBitrate);

	virtual TTUint              BandWidth();

	virtual TTUint              BandPercent();


	virtual void				SetNetWorkProxy(TTBool aNetWorkProxy);

	virtual TTInt               GetStatusCode();
	virtual TTUint				GetHostIP();

	virtual TTUint              ProxySize();


protected:
	TTInt						iAudioBitrate;
	TTInt						iVideoBitrate;

	TTBool						iCancel;
	TTBool						iUseProxy;
};
#endif