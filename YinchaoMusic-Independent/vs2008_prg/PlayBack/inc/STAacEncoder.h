#ifndef __ST_AAC_ENCODER_H__
#define __ST_AAC_ENCODER_H__
#include <stdio.h>
#include "aacenc_lib.h"
#include "STTypedef.h"
#include "STMacroDef.h"

class STAacEncoder
{
public:
    STAacEncoder();
    ~STAacEncoder();

public:
    /**
	* \fn                       STInt Init(STInt aSampleRate, STInt aChannels, const STChar* aSavePath)
	* \brief                    初始化
	* \param[in] aSampleRate  	采样率
	* \param[in] aChannels  	声道数
	* \param[in] aSavePath  	保存目标地址
	* \return					操作状态码
	*/
    STInt 						Init(STInt aSampleRate, STInt aChannels, const STChar* aSavePath);

    /**
	* \fn                       STInt UnInit()
	* \brief                    UnInit
	*/
    void 						UnInit();

    /**
	* \fn                       STInt Execute(STUint8* aBuffer, STUint32 aSize)
	* \brief                    执行Encode操作
	* \param[in] aBuffer  		Buffer指针
	* \param[in] aSize  		Buffer大小
	* \param[in] aIsEnd			是否最后一帧
	* \return  					操作码
	*/
    STInt 						Execute(STUint8* aBuffer, STUint32 aSize, STBool aIsEnd = ESTFalse);

    /**
	* \fn                       STInt Finish()
	* \brief                    完成编码
	* \return  					返回操作码
	*/
    STInt 						Finish();

    /**
	* \fn                       STInt Duration()
	* \brief                    获取时长
	* \return  					时长（ms）
	*/
	STInt 						Duration();

private:
    STUint8* 				iOutBuffer;
    STInt				    iOutBufferSize;
    FILE* 					iSaveFile;
    HANDLE_AACENCODER 		iHandle;
    STInt					iTotalFrameCount;
    STFloat					iFrameDuration;//每帧时长
};

#endif
