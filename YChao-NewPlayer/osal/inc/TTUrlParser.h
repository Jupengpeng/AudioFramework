/**
* File : TTUrlParser.h  
* Created on : 2011-8-31
* Author : ran.zhao
* Copyright : Copyright (c) 2011 Shuidushi Software Ltd. All rights reserved.
* Description : CTTUrlParser声明文件
*/
#ifndef __TT_URL_PARSER__H__
#define __TT_URL_PARSER__H__

// INCLUDES
#include "TTTypedef.h"

// CLASSES DECLEARATION
class CTTUrlParser
{ 
public:

	/**
	* \fn                       void ParseProtocal(const TTChar* aUrl, TTChar* aProtocal);
	* \brief                    根据Url前缀，解析协议名。
	* \param[in]	aUrl		Url
	* \param[out]	aProtocal	协议名(没有协议名时，返回空字符)
	*/
	static void					ParseProtocal(const TTChar* aUrl, TTChar* aProtocal);

	/**
	* \fn                       void ParseExtension(const TTChar* aUrl, TTChar* aExtension);
	* \brief                    解析Url后缀名。
	* \param[in]	aUrl		Url
	* \param[out]	aExtension	后缀名(没有后缀时，返回空字符)
	*/
	static void					ParseExtension(const TTChar* aUrl, TTChar* aExtension);

	/**
	* \fn                       void ParseShortName(const TTChar* aUrl, TTChar* aShortName);
	* \brief                    解析Url路径中的short name。例如: 输入http://www.google.com/download/1.mp3 --> 输出1.mp3
	* \param[in]	aUrl		Url
	* \param[out]	aShortName	Short name.
	*/
	static void					ParseShortName(const TTChar* aUrl, TTChar* aShortName);

	/**
	* \fn                       void ParseUrl(const TTChar* aUrl, TTChar* aHost, TTChar* aPath, TTInt& aPort);
	* \brief                    从Url中解析出主机名，路径和端口号。
	* \param[in]	aUrl		Url
	* \param[out]	aHost		主机名
	* \param[out]	aPath		路径
	* \param[out]	aPort		端口号
	*/
	static void					ParseUrl(const TTChar* aUrl, TTChar* aHost, TTChar* aPath, TTInt& aPort);

};

#endif
