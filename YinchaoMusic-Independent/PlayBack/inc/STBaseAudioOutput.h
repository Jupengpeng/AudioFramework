#ifndef __ST_BASE_AUDIO_OUTPUT_H__
#define __ST_BASE_AUDIO_OUTPUT_H__
#include "STAudioOutputItf.h"
#include "STCritical.h"
#include "STArray.h"

class STThread;
class STBaseAudioOutput : public ISTAudioOutputItf
{
public:
	/**
	* \fn                       STBaseAudioOutput()
	* \brief                    构造函数
	*/
	STBaseAudioOutput();

	/**
	* \fn                       ~STBaseAudioOutput()
	* \brief                    析构函数
	*/
	virtual ~STBaseAudioOutput();

	/**
	* \fn                       STInt Open(STInt aSampleRate, STInt aChannels, const STChar* aRecordSavePath)
	* \brief                    打开设备
	* \param[in] aSampleRate	采样率
	* \param[in] aChannels		声道数
	* \param[in] aRecordSavePath 如果非空则支持录制
	* \return					STKErrNone为成功
	*/
	virtual STInt				Open(STInt aSampleRate, STInt aChannels, const STChar* aRecordSavePath);

    /**
     * \fn                      void Stop
     * \brief                   停止播放	
     */
	virtual void				Stop();

	/**
	* \fn                       void RenderBuffer(STSampleBuffer* aBuffer)
	* \brief                    提交Buffer	
	* \param[in] aBuffer		包含Pcm数据的Buffer的指针
	*/

	virtual void				RenderBuffer(STSampleBuffer* aBuffer);

	/**
	* \fn                       void RecycleBuffer()
	* \brief                    回收Buffer
	* \param[in] aForceRecycle  是否强制回收
	* \return					回收Buffer的指针
	*/
	virtual STSampleBuffer*		RecycleBuffer(STBool aForceRecycle = ESTFalse);

	/**
	* \fn                       void Position(STUint& aPosition)
	* \brief                    获取当前播放位置	
	* \param[out] aPosition		当前位置
	*/
	virtual void				Position(STUint& aPosition);
	
	/**
	* \fn                       void SyncPosition(STUint aPosition)
	* \brief                    设置当前位置	
	* \param[in]  aPosition		当前位置
	*/
	virtual void				SyncPosition(STUint aPosition);	

	/**
	* \fn                       void Flush()
	* \brief                    暂停
	*/
	virtual void				Flush();

	/**
    * \fn                       void Switch2Stream(STInt aStreamIdx);
    * \brief                    切换流操作
	* \param[in]  aStreamIdx	目标流的Id
	* \return					返回操作码
    */
	virtual STInt				Switch2Stream(STInt aStreamIdx);

	/**
	* \fn                       STInt GetCurSteramIndex();
	* \brief                    获取当前流Idx
	* \return					流的Idx
	*/
	virtual STInt				GetCurSteramIndex();

protected:
	virtual STSampleBuffer*		GetFilledBuffer();
	virtual STInt				GetMinCachedOffset();

protected:
	STThread* const						iDecodeThreadHandle;
	STCritical							iCritical;
	STSampleBuffer*						iCurSampleBuffer;
	STUint								iSampleRate;
	STUint								iChannels;	
    STPointerArray<STSampleBuffer>		iEmptiedBackgroundBufferArray;
    STPointerArray<STSampleBuffer>		iEmptiedOriginBufferArray;
    STPointerArray<STSampleBuffer>		iFillBufferArray;
    STUint								iCurPosition;
};

#endif
