#ifndef __ST_AUDIO_OUTPUT_H__
#define __ST_AUDIO_OUTPUT_H__
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "STBaseAudioOutput.h"
#include "STOpenSLESEngine.h"
#include "STAudioRecorder.h"
#ifdef  __SUPPORT_RECORD__
#include "STAacEncoder.h"
#include "denoise.h"
#include "STReverb.h"
#endif

class STAudioOutput : public STBaseAudioOutput
#ifdef __SUPPORT_RECORD__
, public ISTAudioRecorderCallback
#endif
{
public:
	STAudioOutput();
	virtual ~STAudioOutput();
	
#ifdef __SUPPORT_RECORD__
public://form STAudioRecorderCallback
	/**
	* \fn                       void RecorderError(STInt aErr)
	* \brief                    解码出错
	* \param[in] aErr			错误码
	*/
	void 						RecorderError(STInt aErr);
	
	/**
	* \fn                       void RecorderBufferFilled(STUint8* aBufferPtr, STInt aBufferSize, STUint& aState)
	* \brief                    暂停
	* \param[in] aBufferPtr		录制回调Buffer地址
	* \param[in] aBufferSize	回调Buffer大小
	* \param[in] aByteOffset	字节偏移
	* \param[in] aState
	* \return					返回已经处理的字节数
	*/
	virtual STInt RecorderBufferFilled(STUint8* aBufferPtr, STInt aBufferSize, STInt aByteOffset, STUint& aState);

	STInt	      setAudioImpulsePath(const STChar* path,int flag);

	Denoise*                     iDenoise;

	STReverb*                    m_pSTReverb;

#endif

public:
	/**
	* \fn                       void Pause()
	* \brief                    暂停
	*/
	virtual void				Pause();

	/**
	* \fn                       void Resume()
	* \brief                    继续
	*/
	virtual	void				Resume();

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
	* \fn                       void Close
	* \brief                    关闭设别	
	*/
	virtual void				Close();

	/**
	* \fn                       void Start
	* \brief                    开始播放	
	*/
	virtual void				Start();

	/**
	* \fn                       void Stop
	* \brief                    停止播放	
	*/
	virtual void				Stop();

	/**
	* \fn                       void Flush()
	* \brief                    暂停
	*/
	virtual void				Flush();

	/**
	* \fn                       STBool IsUnderflow();
	* \brief                    后端数据是否用完，主要用于判断是否播放完所有的Buffer	
	* \return					所有的数据用完返回ESTTrue
	*/
	virtual STBool				IsUnderflow();	

	/**
	* \fn						STInt GetCurWave(STInt16* aWave, STInt aSamples, STInt& aChannels);
	* \brief					获取当前播放的数据
	* \param[out] aWave			采样点的数据指针	
	* \param[in]  aSamples		取采样的个数	
	* \param[out] nChannels		音频声道数	
	*/
	virtual STInt				GetCurWave(STInt16* aWave, STInt aSamples, STInt& aChannels);
    
    /**
    * \fn                       void Position(STUint& aPosition)
     * \brief                    获取当前播放位置	
     * \param[out] aPosition	 当前位置
    */
	virtual void				Position(STUint& aPosition);

	/**
	* \fn                       void RenderBuffer(STSampleBuffer* aBuffer)
	* \brief                    提交Buffer
	* \param[in] aBuffer		包含Pcm数据的Buffer的指针
	*/

	virtual void				RenderBuffer(STSampleBuffer* aBuffer);

	/**
	* \fn                       void SetAccompanimentVolume(STInt aVolume);
	* \brief                    设置伴奏音量
	*/
	virtual void                SetAccompanimentVolume(STInt aVolume);

	/**
	* \fn                       void SetRecorderVolume(STInt aVolume);
	* \brief                    设置录音音量
	*/
	virtual void                SetRecorderVolume(STInt aVolume);

	/**
    * \fn                       void Switch2Stream(STInt aStreamIdx);
    * \brief                    切换流操作
	* \param[in]  aStreamIdx	目标流的Id
	* \return					返回操作码
    */
	virtual STInt				Switch2Stream(STInt aStreamIdx);

	/**
	* \fn                       void RecycleBuffer()
	* \brief                    回收Buffer
	* \param[in] aForceRecycle  是否强制回收
	* \return					回收Buffer的指针
	*/
	virtual STSampleBuffer*		RecycleBuffer(STBool aForceRecycle = ESTFalse);

	/**
	* \fn                       STInt GetCurSteramIndex();
	* \brief                    获取当前流Idx
	* \return					流的Idx
	*/
	virtual STInt				GetCurSteramIndex();

#ifdef __SUPPORT_RECORD__
	/**
	* \fn                       STInt GetRecordDuration();
	* \brief                    获取编码文件的时长
	* \return					编码文件的时长(ms)
	*/
	virtual STInt				GetRecordDuration();

	/**
	* \fn                       STInt IsRecorderExisted();
	* \brief                    是否存在录制
	* \return					true表示存在录制
	*/
	virtual STBool				IsRecorderExisted();

	/**
	 * \fn                      STInt GetCurRecordWave(STInt16* aWave, STInt aSamples, STInt& aChannels)
	 * \brief                   获取当前录制的数据
	 * \param[out] aWave		获取的数据
	 * \param[in]  aSamples		采样点的个数
	 * \param[in]  aChannels
	 */
	virtual STInt				GetCurRecordWave(STInt16* aWave, STInt aSamples, STInt& aChannels);
#endif

	/**
	* \fn                       void SyncPosition(STUint aPosition)
	* \brief                    设置当前位置
	* \param[in]  aPosition		当前位置
	*/
	virtual void				SyncPosition(STUint aPosition);

protected:
	virtual STSampleBuffer*		GetFilledBuffer();
	virtual STInt				GetMinCachedOffset();
	
private:
    STInt                        CreateBufferQueueAudioPlayer(STInt sampleRate, STInt channels);
    static void                  AudioCallback(SLAndroidSimpleBufferQueueItf bq, void *context);
    void                         FillBuffer();
    void						 Mix(STInt16* aDstBuffer, STInt16* aRecordBuffer, STInt16* aBackgroundBuffer, STInt aSizeInShort);
	void						 SetOutputVolume(STInt aVolume);
	STInt						 ProcessEncode(STUint8* aBuffer, STInt aSize);
	STInt						 GetPlayPos();

private:
    STOpenSLESEngine*		     iOpenSLESEngine;
    STAudioRecorder*			 iAudioRecorder;
    
    SLObjectItf                  iPlayerObjectItf;
    SLPlayItf                    iPlayerItf;
    SLAndroidSimpleBufferQueueItf iPlayerBufferQueueItf;
    SLVolumeItf                  iPlayerVolumeItf;
    
    STBool                       iFlushed;
    STInt16						 iAccompanimentVolume;
    STInt16						 iRecordVolume;
    STInt						 iCurStreamIdx;
    STInt						 iTotalRenderSize;

#ifdef  __SUPPORT_RECORD__
    STCritical					 iRecordCritical;
    STInt						 iProcessedByteOffset;
    STAacEncoder*                iAacEncoder;

    STUint8*					 iMixBuffer;

    STUint8*					 iRecrodLoopBuffer;//循环数组起始地址
    STUint8*					 iRecordLoopBufferStartPtr;//有效数据起始地址
    STInt						 iRecordLoopBufferValidSize;//有效数据大小
#endif
};

#endif
