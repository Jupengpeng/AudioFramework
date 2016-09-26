#ifndef __ST_MEDIA_PLAYER_ITF_H__
#define __ST_MEDIA_PLAYER_ITF_H__
#include "STInterface.h"
#include "STArray.h"

enum STNotifyMsgId
{
	ENotifyPrepared = 1
	, ENotifyStarted = 2
	, ENotifyCompleted = 3
	, ENotifyPaused = 4
    , ENotifyResumed = 5
	, ENotifyException = 6
	, ENotifyRecordFinished = 7
};

enum STPlayStatus
{
	EStatusPreparing = 1
	, EStatusPrepared = 2
	, EStatusPlaying = 3
	, EStatusPaused = 4
	, EStatusStoped = 5
};

const STInt KMaxWaveFreqSampleNum = 1024;
const STInt KMinWaveFreqSampleNum = 128;

class ISTMediaPlayerItf;
class ISTMediaPlayerObserver
{
public:
	/**
	* \fn                           void PlayerNotifyEvent(STNotifyMsgId aMsg, ISTMediaPlayerItf* aSender, STInt aArg)
	* \brief                        播放开始后，状态改变通知上层
	* \param [in]      aMsg   		操作ID
	* \param [in]	   aSender      发送者
	* \param [in]      aArg   		辅助参数
	*/
	virtual void					PlayerNotifyEvent(STNotifyMsgId aMsg, ISTMediaPlayerItf* aSender, STInt aArg) = 0;
};

class ISTMediaPlayerItf : public ISTInterface
{
public:
	/**
	* \fn                           STUint Duration
	* \brief                        媒体时长
	* \return   					时长，毫秒为单位
	*/
	virtual STUint					Duration() = 0;

	/**
	* \fn                           STInt SetDataSource(const STChar* aUrl)
	* \brief                        同步设置src
	* \param [in]   aUrl			媒体路径
	* \param [in]   aParams			辅助参数
	* \return						返回状态
	*/
	virtual STInt					SetDataSource(const STChar* aUrl, const STChar* aParams) = 0;

	/**
	* \fn                           STInt SetDataSource(const STChar* aUrl)
	* \brief                        异步设置Src
	* \param [in]   aUrl			媒体路径
	* \param [in]   aParams			辅助参数
	* \return						返回状态
	*/
	virtual STInt					SetDataSourceAsync(const STChar* aUrl, const STChar* aParams) = 0;

	/**
	* \fn                           STInt Play()
	* \brief                        启动播放
	* \return						返回状态值
	*/
	virtual STInt					Play() = 0;

	/**
	* \fn                           STInt Stop()
	* \brief                        停止播放
	* \return						返回状态值
	*/
	virtual STInt					Stop() = 0;

	/**
	* \fn                           void Pause()
	* \brief                        Pause
	*/
	virtual void					Pause() = 0;

	/**
	* \fn                           void Resume()
	* \brief                        Resume
	*/
	virtual void					Resume() = 0;

	/**
	* \fn                           STPlayStatus GetPlayStatus();
	* \brief                        获取播放状态
	* \return                       播放状态
	*/
	virtual STPlayStatus			GetPlayStatus() = 0;

	/**
	* \fn                           void Seek(STUint aPosition)
	* \brief                        Seek操作
	* \param [in]   aPosition       单位为毫秒
	*/
	virtual void					Seek(STUint aPosition) = 0;
    
 	/**
	* \fn                           STUint GetPosition()
	* \brief                        获取播放位置
	* \return                       播放位置，单位为毫秒
	*/
	virtual STUint					GetPosition() = 0;

	/**
	* \fn                           STInt GetCurFreqAndWave(STInt16 *aFreq, STInt16 *aWave, STInt aSampleNum, STInt& aChannels);
	* \brief                        获取当前频谱
	* \param[out]    aFreq          频谱数据指针，不需要的话输入NULL
	* \param[out]    aWave          波形数据指针,必须非空
	* \param[in]     aSampleNum     采样点的个数
	* \param[out]    aChannels     	通道数
	* \return                       操作状态
	*/
	virtual STInt                   GetCurFreqAndWave(STInt16 *aFreq, STInt16 *aWave, STInt aSampleNum, STInt& aChannels) = 0;

	/**
	* \fn                           STInt GetCurRecordWave(STInt16 *aWave, STInt aSampleNum, STInt& aChannels);
	* \brief                        获取当前频谱
	* \param[out]    aWave          波形数据指针,必须非空
	* \param[in]     aSampleNum     采样点的个数
	* \param[out]    aChannels     	通道数
	* \return                       操作状态
	*/
	virtual STInt                   GetCurRecordWave(STInt16 *aFreq, STInt16 *aWave, STInt aSampleNum, STInt& aChannels) = 0;


	/**
	* \fn                           STInt Switch2Stream(const STChar* aStreamName);
	* \brief                        切换到某条流
	* \param[in]    aStreamName     指定流的名字
	* \return                       操作状态
	*/
	virtual STInt					Switch2Stream(const STChar* aStreamName) = 0;

	/**
	* \fn                           STChar* GetCurStreamName();
	* \brief                        获取当前流的名称
	* \return                       流的名称
	*/
	virtual const STChar*			GetCurStreamName() = 0;
    
    /**
    * \fn                           void SetAccompanimentVolume(STInt aVolume);
    * \brief                        设置播放声音
    */
    virtual void                    SetAccompanimentVolume(STInt aVolume) = 0;

    /**
    * \fn                           void SetRecorderVolume(STInt aVolume);
    * \brief                        设置录音声音
    */
    virtual void                    SetRecorderVolume(STInt aVolume) = 0;
};

#endif
