#ifndef __ST_PLUGIN_ITF_H__
#define __ST_PLUGIN_ITF_H__
#include "STTypedef.h"
#include "STInterface.h"
#include "STArray.h"

enum STReadStatus
{
	ESTReadStatusNotReady
	, ESTReadStatusReading
	, ESTReadStatusReadErr
	, ESTReadStatusComplete
};

class STMediaInfo;
class STSampleBuffer;
class ISTPluginItf : public ISTInterface
{
public:
	/**
	* \fn				            const STMediaInfo&	GetMediaInfo() const
	* \brief				        获取媒体信息
	* \return						媒体信息
	*/
	virtual const STMediaInfo&		GetMediaInfo() const = 0;	

	/**
	* \fn				            STInt InitPlugin(const STChar* aUrl)	
	* \brief				        初始化解码器
	* \param[in] aUrl				Url路径
	* \param[in] aParams			参数
	* \return						STKErrNone: 成功
	*/
	virtual STInt					InitPlugin(const STChar* aUrl, const STChar* aParams) = 0;

	/**
	* \fn				            void ResetPlugin()
	* \brief				        复位解码器
	*/
	virtual	void					ResetPlugin() = 0;

	/**
	* \fn				            void UnInitPlugin()
	* \brief				        释放解码器
	*/
	virtual	void					UnInitPlugin() = 0;
	
	/**
	* \fn				            STInt StartReading()
	* \brief				        开始读取PCM数据
	* \return						STKErrNone: 成功
	*/
	virtual	STInt					StartReading() = 0;

	/**
	* \fn				            STReadStatus ReadStatus()
	* \brief				        获取Plugin当前Read状态
	* \return						Plugin当前状态Read的状态
	*/
	virtual	STReadStatus			ReadStatus() = 0;
	
	/**
	* \fn				            STInt Read(STSampleBuffer* aBuffer)
	* \brief				        读取PCM,填充在aBuffer中
	* \param[in] aBuffer			Buffer指针
	* \return						错误码
	*/
	virtual STInt					Read(STSampleBuffer* aBuffer) = 0;

	/**
	* \fn				            void Seek(STUint aPos)
	* \brief				        Seek操作
	* \param[in]					Seek到的位置
	*/
	virtual	void					Seek(STUint aPos) = 0;	

	/**
	* \fn				            STUint Duration();
	* \brief				        获取歌曲时长
	* \return						歌曲时长
	*/
	virtual	STUint					Duration() = 0;

	/**
	* \fn                           STInt Switch2Stream(const STChar* aStreamName);
	* \brief                        切换到某条流
	* \param[in]    aStreamName     指定流的名字
	* \return                       操作状态
	*/
	virtual STInt					Switch2Stream(const STChar* aStreamName) = 0;

	/**
	* \fn                           const STChar* GetStreamName(STInt aIndex);
	* \brief                        获取流的名称
	* \param[in]    aIndex     		流索引
	* \return                       名称的指针
	*/
	virtual const STChar*  			GetStreamName(STInt aIndex) = 0;

	/**								void Abort()
	 *								取消操作
	 */
	virtual void					Abort() = 0;
};
#endif
