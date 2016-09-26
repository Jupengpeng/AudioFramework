#ifndef __ST_SUDIO_OUTPUT_ITF_H__
#define __ST_SUDIO_OUTPUT_ITF_H__
#include "STInterface.h"
#include "STMediaPlayerItf.h"
#include "STArray.h"

class STSampleBuffer;

class ISTAudioOutputItf : public ISTInterface
{
public:
	/**
	* \fn                       void Flush()
	* \brief                    暂停
	*/
	virtual void				Flush() = 0;

	/**
	* \fn                       void Pause()
	* \brief                    暂停
	*/
	virtual void				Pause() = 0;

	/**
	* \fn                       void Resume()
	* \brief                    继续
	*/
	virtual	void				Resume() = 0;

	/**
	* \fn                       STInt Open(STInt aSampleRate, STInt aChannels, const STChar* aRecordSavePath)
	* \brief                    打开设备
	* \param[in] aSampleRate	采样率
	* \param[in] aChannels		声道数
	* \param[in] aRecordSavePath 如果非空则支持录制
	* \return					STKErrNone为成功
	*/
	virtual STInt				Open(STInt aSampleRate, STInt aChannels, const STChar* aRecordSavePath) = 0;

	/**
	* \fn                       void Close
	* \brief                    关闭设别	
	*/
	virtual void				Close() = 0;

	/**
	* \fn                       void Start
	* \brief                    开始播放	
	*/
	virtual void				Start() = 0;

	/**
	* \fn                       void Stop
	* \brief                    停止播放	
	*/
	virtual void				Stop() = 0;

	/**
	* \fn                       void RenderBuffer(STSampleBuffer* aBuffer)
	* \brief                    提交Buffer	
	* \param[in] aBuffer		包含Pcm数据的Buffer的指针
	*/
	virtual void				RenderBuffer(STSampleBuffer* aBuffer) = 0;

	/**
	* \fn                       void RecycleBuffer()
	* \brief                    回收Buffer	
	* \param[in] aForceRecycle  是否强制回收
	* \return					回收Buffer的指针
	*/
	virtual STSampleBuffer*		RecycleBuffer(STBool aForceRecycle = ESTFalse) = 0;

	/**
	* \fn                       void Position(STUint& aPosition)
	* \brief                    获取当前播放位置	
	* \param[out] aPosition		当前位置
	*/
	virtual void				Position(STUint& aPosition) = 0;
	
	/**
	* \fn                       void SyncPosition(STUint aPosition)
	* \brief                    设置当前位置	
	* \param[in]  aPosition		当前位置
	*/
	virtual void				SyncPosition(STUint aPosition) = 0;	

	/**
	* \fn                       STBool IsUnderflow();
	* \brief                    后端数据是否用完，主要用于判断是否播放完所有的Buffer	
	* \return					所有的数据用完返回ESTTrue
	*/
	virtual STBool				IsUnderflow() = 0;	

	/**
	* \fn						STInt GetCurWave(STInt16* aWave, STInt aSamples, STInt& aChannels);
	* \brief					获取当前播放的数据
	* \param[out] aWave			采样点的数据指针	
	* \param[in]  aSamples		取采样的个数	
	* \param[out] nChannels		音频声道数	
	*/
	virtual STInt				GetCurWave(STInt16* aWave, STInt aSamples, STInt& aChannels) = 0;

	/**
    * \fn                       void SetAccompanimentVolume(STInt aVolume);
    * \brief                    设置播放声音
    */
    virtual void                SetAccompanimentVolume(STInt aVolume) = 0;

    /**
    * \fn                       void SetRecorderVolume(STInt aVolume);
    * \brief                    设置录音声音
    */
    virtual void                SetRecorderVolume(STInt aVolume) = 0;

	/**
    * \fn                       void Switch2Stream(STInt aStreamId);
    * \brief                    切换流操作
	* \param[in]  aStreamIdx	目标流的Id
	* \return					返回操作码
    */
	virtual STInt				Switch2Stream(STInt aStreamId) = 0;

	/**
	* \fn                       STInt GetCurSteramIndex();
	* \brief                    获取当前流Index
	* \return					流的Index
	*/
	virtual STInt				GetCurSteramIndex() = 0;

#ifdef __SUPPORT_RECORD__
	/**
	* \fn                       STInt GetRecordDuration();
	* \brief                    获取编码文件的时长
	* \return					编码文件的时长(ms)
	*/
	virtual STInt				GetRecordDuration() = 0;


	/**
	* \fn                       STInt IsRecorderExisted();
	* \brief                    是否存在录制
	* \return					true表示存在录制
	*/
	virtual STBool				IsRecorderExisted() = 0;

	/**
	 * \fn                      STInt GetCurRecordWave(STInt16* aWave, STInt aSamples, STInt& aChannels)
	 * \brief                   获取当前录制的数据
	 * \param[out] aWave		获取的数据
	 * \param[in]  aSamples		采样点的个数
	 * \param[in]  aChannels
	 */
	virtual STInt				GetCurRecordWave(STInt16* aWave, STInt aSamples, STInt& aChannels) = 0;
#endif
};

#endif
