/**
* File : TTHttpClient.h  
* Created on : 2012-3-19
* Author : hu.cao
* Copyright : Copyright (c) 2011 Shuidushi Software Ltd. All rights reserved.
* Description : STHttpClient定义文件,用于管理与服务器的交互
*/
#ifndef __ST_HTTP_CLIENT_H__
#define __ST_HTTP_CLIENT_H__

#include "STtypedef.h"
#include "STMacrodef.h"
#include "STOSConfig.h"
#include "STCritical.h"

#if defined __ST_OS_WINDOWS__
#include <WinSock.h>
#endif

static const STInt KMaxLineLength = 2048;
class STHttpClient
{
public:
	/**
	* \fn							STHttpClient();
	* \brief						构造函数
	*/
	STHttpClient();

	/**
	* \fn							~STHttpClient();
	* \brief						析构函数
	*/
	~STHttpClient();
	
	/**
	* \fn							STInt Read(STChar* aDstBuffer, STInt aSize);
	* \brief						从当前位置读取流
	* \param[in] aDstBuffer			用于填充数据的Buffer		
	* \param[in] aSize			    读取的数据大小	
	* \return						读取的字节数或者错误码
	*/
	STInt							Read(STChar* aDstBuffer, STInt aSize);

	/**
	* \fn							STInt Recv(STChar* aDstBuffer, STInt aSize);
	* \brief						从服务器接收数据
	* \param[in] aDstBuffer			用于接收数据的Buffer		
	* \param[in] aSize			    接收的数据大小	
	* \return						接收的字节数或者错误码
	*/
	STInt							Recv(STChar* aDstBuffer, STInt aSize);

	/**
	* \fn							STInt Send(STChar* aDstBuffer, STInt aSize);
	* \brief						向服务器发送数据
	* \param[in] aSendBuffer		发送数据的Buffer		
	* \param[in] aSize			    发送的数据大小	
	* \return						操作状态
	*/
	STInt							Send(const STChar* aSendBuffer, STInt aSize);

	/**
	* \fn							STInt Connect(STChar* aUrl);
	* \brief						连接服务器
	* \param[in] aUrl				资源路径
	* \param[in] aOffset			读取偏移
	* \return						操作状态
	*/
	STInt							Connect(const STChar* aUrl, STInt aOffset = 0);

	/**
	* \fn							STInt Disconnect();
	* \brief						断开连接		
	* \return						操作状态
	*/
	STInt							Disconnect();

	/**
	* \fn							STInt ContentLength();
	* \brief						获取资源文件大小		
	* \return						资源文件大小(byte)
	*/
	STInt							ContentLength(){ return iContentLength ;};

	/**								void Abort()
	 *								取消操作
	 */
	void							Abort();

private:
	STInt				ReceiveLine(STChar* aLine, STInt aSize);
	STInt				ParseHeader(STInt32& aResponseStatus);
	STBool				IsRedirectStatusCode(STInt aResStatus);
	STInt				GetHeaderValueByKey(const STChar* aKey, STChar* aBuffer, STInt aBufferLen);
	void				SetReceiveTimeout(STInt aSecond);
#if 0
	STInt				WaitSocketReadBuffer(timeval timeout);
#endif

private:
	enum State {
	        DISCONNECTED,
	        CONNECTING,
	        CONNECTED
	    };

	State		iState;
	STChar*		iUrl;
	STInt		iSocketHandle;
	STInt		iContentLength;
	STBool		iAborted;
	STChar		iLineBuffer[KMaxLineLength];
	STChar		iHeaderValueBuffer[KMaxLineLength];
	STCritical  iCritical;
};

#endif
