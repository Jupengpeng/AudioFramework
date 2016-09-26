#ifndef __ST_AAC_PLUGIN_H__
#define __ST_AAC_PLUGIN_H__
#include "STBasePlugin.h"
#include "STDataReaderItf.h"
#include "faad.h"
#include "STOSConfig.h"
#include "STParamKeys.h"

static const int KStreamBackgourndIdx = 0;
static const int KStreamOriginIdx = KStreamBackgourndIdx + 1;
static const int KStreamMaxCount = KStreamOriginIdx + 1;

class STDataBuffer;

class STAacPlugin : public STBasePlugin
{
public:
	STAacPlugin();
	virtual ~STAacPlugin();

public:
	/**
	* \fn				            STInt InitPlugin(const STChar* aUrl)	
	* \brief				        初始化插件
	* \param						Url路径
	* \param[in] aParams			参数
	* \return						STKErrNone: 成功
	*/
	virtual STInt					InitPlugin(const STChar* aUrl, const STChar* aParams);

	/**
	* \fn				            void ResetPlugin()
	* \brief				        复位插件
	*/
	virtual	void					ResetPlugin();

	/**
	* \fn				            void UnInitPlugin()
	* \brief				        释放插件
	*/
	virtual	void					UnInitPlugin();
	
	/**
	* \fn				            STInt StartReading()
	* \brief				        开始读取PCM数据
	* \return						STKErrNone: 成功
	*/
	virtual	STInt					StartReading();
	
	/**
	* \fn				            STInt Read(STSampleBuffer* aBuffer)
	* \brief				        读取PCM,填充在aBuffer中
	* \param[in] aBuffer			Buffer指针
	* \return						错误码
	*/
	virtual STInt					Read(STSampleBuffer* aBuffer);

	/**
	* \fn				            void Seek(STUint aPos)
	* \brief				        Seek操作
	* \param[in]					Seek到的位置
	*/
	virtual	void					Seek(STUint aPos);
	
	/**
	* \fn                           STInt Switch2Stream(const STChar* aStreamName);
	* \brief                        切换流
	* \param[in]    aStreamName     切换到流的名
	* \return                       操作码
	*/	
	STInt 							Switch2Stream(const STChar *aStreamName);
	
	/**
	* \fn                           const STChar* GetStreamName(STInt aIndex);
	* \brief                        获取流的名称
	* \param[in]    aIndex     		流索引
	* \return                       名称的指针
	*/
	const STChar*					GetStreamName(STInt aIndex);

	/**								void Abort()
	 *								取消操作
	 */
	void							Abort();
private:
	/**
	* \fn				            STInt Decode(STUint aIdx, STSampleBuffer* aBuffer)
	* \brief				   		解码由nIdx指定的音乐，并填充到buffer中
	* \param[in] aIdx		   		音乐的索引，从0开始
	* \param[out]	aBuffer	   		用于填充解码后的数据
	* \return				   		错误码
	*/
	STInt 							Decode(STUint aIdx, STSampleBuffer* aBuffer);


private:
	STInt 							iStreamCount;
	STUint8 *						iReadBuffer;
	ISTDataReaderItf*				iDataReader[KStreamMaxCount];
	STInt							iCurReadPos[KStreamMaxCount];
	
	NeAACDecHandle					iHDecoder[KStreamMaxCount];
	
	STUint8*						iLastRemainBuffer;
	STInt							iLastRemainCount;

	STChar 							iBackgroundName[KMaxPathLength];
	STChar 							iOriginName[KMaxPathLength];
	STInt							iCurStreamIdx;
	STUint							iByteOffset[KStreamMaxCount];
	STChar 							iTempFullNameBuffer[KMaxPathLength];
};

#endif

