#ifndef __ST_MEDIA_PALYER_H__
#define __ST_MEDIA_PALYER_H__
#include "STTypedef.h"
#include "STMediaPlayerItf.h"
#include "STThread.h"
#include "STMsgQueue.h"
#include "STPlayControl.h"

class STMediaPlayer : public ISTMediaPlayerItf, public ISTPlayControlObserver
{
public: 
	STMediaPlayer(ISTMediaPlayerObserver& aMediaPlayerObserver);
	virtual ~STMediaPlayer();

public:
	/**
	* \fn								 void PrepareComplete();
	* \brief							 启动完成
	*/
	virtual	void						 PrepareComplete();

	/**
	* \fn								 void StartComplete();
	* \brief							 启动完成
	*/
	virtual	void						 StartComplete();

	/**
	* \fn								 void PlayComplete();
	* \brief							 播放完成
	*/
	virtual	void						 PlayComplete();

	/**
	* \fn								 void PauseComplete();
	* \brief							 暂停完成
	*/
	virtual void       					 PauseComplete();
    
    /**
     * \fn							　	 void ResumeComplete();
     * \brief							 继续播放完成
     */
    virtual void       					 ResumeComplete();

	/**
	* \fn							　	 void PlayException(STInt aError);
	* \brief							 播放异常
	* \param [in]   aError				 错误码
	*/
	virtual void       					 PlayException(STInt aError);

	/**
	* \fn							　	 void RecordFinished(STInt aDuration);
	* \brief							 录制完成通知
	* \param [in]   aDuration			 时长
	*/
	virtual void						 RecordFinished(STInt aDuration);

public:

	/**
	* \fn                           STUint Duration
	* \brief                        媒体时长
	* \return   					时长，毫秒为单位
	*/
	STUint							Duration();

	/**
	* \fn                           STInt SetDataSource(const STChar* aUrl)
	* \brief                        同步设置src
	* \param [in]   aUrl			媒体路径
	* \param [in]   aParams			辅助参数
	* \return						返回状态
	*/
	STInt							SetDataSource(const STChar* aUrl, const STChar* aParams);

	/**
	* \fn                           STInt SetDataSource(const STChar* aUrl)
	* \brief                        异步设置Src
	* \param [in]   aUrl			媒体路径
	* \param [in]   aParams			辅助参数
	* \return						返回状态
	*/
	STInt							SetDataSourceAsync(const STChar* aUrl, const STChar* aParams);

	/**
	* \fn                           STInt Play()
	* \brief                        启动播放
	* \return						返回状态值
	*/
	STInt							Play();

	/**
	* \fn                           STInt Stop()
	* \brief                        停止播放
	* \return						返回状态值
	*/
	STInt							Stop();

	/**
	* \fn                           void Pause()
	* \brief                        Pause
	*/
	void							Pause();

	/**
	* \fn                           void Resume()
	* \brief                        Resume
	*/
	void							Resume();

	/**
	* \fn                           STPlayStatus GetPlayStatus();
	* \brief                        获取播放状态
	* \return                       播放状态
	*/
	STPlayStatus					GetPlayStatus();

	/**
	* \fn                           void Seek(STUint aPosition)
	* \brief                        Seek操作
	* \param [in]   aPosition       单位为毫秒
	*/
	void							Seek(STUint aPosition);

	/**
	* \fn                           STUint GetPosition()
	* \brief                        获取播放位置
	* \return                       播放位置，单位为毫秒
	*/
	STUint							GetPosition();

	/**
	* \fn                           STInt GetCurFreqAndWave(STInt16 *aFreq, STInt16 *aWave, STInt aSampleNum, STInt& aChannels);
	* \brief                        获取当前频谱
	* \param[out]    aFreq          频谱数据指针，不需要的话输入NULL
	* \param[out]    aWave          波形数据指针,必须非空
	* \param[in]     aSampleNum     采样点的个数
	* \param[out]    aChannels     	通道数
	* \return                       操作状态
	*/
	virtual STInt                   GetCurFreqAndWave(STInt16 *aFreq, STInt16 *aWave, STInt aSampleNum, STInt& aChannels);

	/**
	* \fn                           STInt GetCurRecordWave(STInt16 *aWave, STInt aSampleNum, STInt& aChannels);
	* \brief                        获取当前频谱
	* \param[out]    aWave          波形数据指针,必须非空
	* \param[in]     aSampleNum     采样点的个数
	* \param[out]    aChannels     	通道数
	* \return                       操作状态
	*/
	virtual STInt                   GetCurRecordWave(STInt16 *aFreq, STInt16 *aWave, STInt aSampleNum, STInt& aChannels);

	/**
	* \fn                           STInt Switch2Stream(const STChar* aStreamName);
	* \brief                        切换到某条流
	* \param[in]    aStreamName     指定流的名字
	* \return                       操作状态
	*/
	STInt							Switch2Stream(const STChar* aStreamName);

	/**
	* \fn                           STChar* GetCurStreamName();
	* \brief                        获取当前流的名称
	* \return                       流的名称
	*/
	const STChar*					GetCurStreamName();

	/**
    * \fn							void SetAccompanimentVolume(STInt aVolume);
    * \brief						设置播放声音
    */
   void								SetAccompanimentVolume(STInt aVolume);

	/**
    * \fn							void SetRecorderVolume(STInt aVolume);
    * \brief						设置录音声音
    */
   void								SetRecorderVolume(STInt aVolume);

#ifdef __ST_OS_ANDROID__
	ISTMediaPlayerObserver*          GetAttachMediaPlayerObserver() {return &iPlayerObserver;}
#endif

private:
	void							PlayThreadProcL(STMediaPlayer* aMediaPlayer);
	static void*				    PlayThreadProc(void* aPtr);
	STInt							SetDataSource(const STChar* aUrl, const STChar* aParams, STBool aSync);

private:
	STChar*							iUrl;
	STChar*							iParams;
	STPlayControl*					iPlayControl;
	ISTMediaPlayerObserver&			iPlayerObserver;
	STThread						iWrokThread;
	STMsgQueue						iMsgQueue;
	STCritical						iCritical;
};
#endif
