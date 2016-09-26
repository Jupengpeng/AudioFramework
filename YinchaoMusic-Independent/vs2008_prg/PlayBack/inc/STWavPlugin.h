#ifndef __ST_WAV_FLUGIN_H__
#define __ST_WAV_FLUGIN_H__
#include "STBasePlugin.h"
#include "STDataReaderItf.h"

class STWavPlugin : public STBasePlugin
{
public:
	STWavPlugin();
	virtual ~STWavPlugin();

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

private:
	ISTDataReaderItf*					iFileReader;
	STInt								iCurReadPos;
};

#endif
