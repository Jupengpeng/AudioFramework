#ifndef __ST_BASE_PLUGIN_H__
#define __ST_BASE_PLUGIN_H__
#include "STPluginItf.h"
#include "STArray.h"
#include "STSampleBuffer.h"
#include "STMediaInfo.h"
#include "STCritical.h"
class STBasePlugin : public ISTPluginItf
{	
public:
	STBasePlugin();
	virtual ~STBasePlugin();
public:
	/**
	* \fn				            const STMediaInfo&	GetMediaInfo() const
	* \brief				        获取媒体信息
	* \return						媒体信息
	*/
	virtual const STMediaInfo&		GetMediaInfo() const;

	/**
	* \fn				            STReadStatus ReadStatus()
	* \brief				        获取Plugin当前Read状态
	* \return						Plugin当前状态Read的状态
	*/
	virtual	STReadStatus			ReadStatus();

	/**
	* \fn				            void UnInitPlugin()
	* \brief				        释放解码器
	*/
	virtual void					UnInitPlugin();

	/**
	* \fn				            STUint Duration();
	* \brief				        获取歌曲时长
	* \return						歌曲时长
	*/
	virtual	STUint					Duration();
	/**
	* \fn				            STUint Duration();
	* \brief				        获取已经下载的歌曲的百分比
	* \return						
	*/
	virtual	STInt					GetDownloadPercent();

	/**
	* \fn                           STInt Switch2Stream(const STChar* aStreamName);
	* \brief                        切换到某条流
	* \param[in]    aStreamName     指定流的名字
	* \return                       操作状态
	*/
	virtual STInt					Switch2Stream(const STChar* aStreamName);

	/**
	* \fn                           const STChar* GetStreamName(STInt aIndex);
	* \brief                        获取流的名称
	* \param[in]    aIndex     		流索引
	* \return                       名称的指针
	*/
	virtual const STChar*  			GetStreamName(STInt aIndex);

	/**
	* \fn                           void Abort();
	* \brief                       	停止解码操作
	*/
	virtual void					Abort();

protected:
	STMediaInfo						iMediaInfo;
	STReadStatus					iReadStatus;
	STUint							iDuration;
};
#endif
