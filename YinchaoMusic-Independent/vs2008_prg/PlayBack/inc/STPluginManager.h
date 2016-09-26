#ifndef __ST_PLUGIN_MANAGER_H__
#define __ST_PLUGIN_MANAGER_H__
#include "STArray.h"
#include "STPluginItf.h"
#include "STMacrodef.h"
#include "STDataReaderSelector.h"

class STPluginManager
{
public:
	enum MediaFormatId
	{
		EMediaExtIdNone
		, EMediaExtIdAAC
		, EMediaExtIdALAC
		, EMediaExtIdAMR
		, EMediaExtIdAPE
		, EMediaExtIdFLAC
		, EMediaExtIdM4A
		, EMediaExtIdMIDI
		, EMediaExtIdMP3
		, EMediaExtIdOGG
		, EMediaExtIdWAV
		, EMediaExtIdWMA
	};

public:
	STPluginManager();
	~STPluginManager();

public:
	/**
	* \fn				            ISTPluginItf*	SelectPlugin(const STChar* aUrl)
	* \brief				        根据文件路径选择不同的解码器
	* \param[in] aUrl 				Url路径
	* \param[in] aParams 			参数字符串
	* \return						用于解码的Plugin Handle
	*/
	ISTPluginItf*					SelectPlugin(const STChar* aUrl, const STChar* aParams);


private:
	MediaFormatId					GetMediaFormatId(const STChar* aUrl);
	STChar							iTempBuffer[KMaxPathLength];
};

#endif
