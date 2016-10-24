#ifndef __ST_PLAY_CONTROL_H__
#define __ST_PLAY_CONTROL_H__

#include "STMsgQueue.h"
#include "STMediaPlayerItf.h"
#include "STPluginManager.h"
#include "STAudioOutputItf.h"

enum STPlayerCmdMsgId
{
	ECmdMsgIdDestory
	, ECmdMsgIdOpen
	, ECmdMsgIdStart
	, ECmdMsgIdPause
	, ECmdMsgIdResume
	, ECmdMsgIdStop
	, ECmdMsgIdSeek
	, ECmdMsgIdSwitchStream
	, ECmdMsgIdGetCurStreamName
};

class ISTPluginItf;
class STSampleBufferManager;

class ISTPlayControlObserver //这个接口由播放线程直接回调，故运行在播放线程中
{
public:

	/**
	* \fn								 void PrepareComplete();
	* \brief							 启动完成
	*/
	virtual	void						 PrepareComplete() = 0;

	/**
	* \fn								 void StartComplete();
	* \brief							 启动完成
	*/
	virtual	void						 StartComplete() = 0;


	/**
	* \fn								 void PlayComplete();
	* \brief							 播放完成
	*/
	virtual	void						 PlayComplete() = 0;

	/**
	* \fn								 void PauseComplete();
	* \brief							 暂停完成
	*/
	virtual void       					 PauseComplete() = 0;
    
    /**
     * \fn							　	 void ResumeComplete();
     * \brief							 继续播放完成
     */
    virtual void       					 ResumeComplete() = 0;

	/**
	* \fn							　	 void PlayException(STInt aError);
	* \brief							 播放异常
	* \param [in]   aError				 错误码
	*/
	virtual void       					 PlayException(STInt aError) = 0;

	/**
	* \fn							　	 void RecordFinished(STInt aDuration);
	* \brief							 录制完成通知
	* \param [in]   aDuration			 时长
	*/
	virtual void						 RecordFinished(STInt aDuration) = 0;
};


class STPlayControl : public ISTMsgHandle
{
public:

	/**
	* \fn								 CTTPlayControlSTPlayControlITTPlayControlObserver& aObserver);
	* \brief							 构造函数
	* \param[in]	aObserver			 回调引用
	*/
	STPlayControl(ISTPlayControlObserver& aObserver);

	/**
	* \fn								 ~STPlayControl();
	* \brief							 析构函数
	*/
	~STPlayControl();


public:
	/**
	* \fn								 STBool IsAllJobToDone();
	* \brief							 进行解码相关的工作
	* \return							 无任何事情做返回ESTTure
	*/
	STBool								 IsAllJobToDone();

	/**
	* \fn								 STBool IsToDestory();
	* \brief							 是否退出解码线程
	* \return							 true 表示退出
	*/
	STBool								 IsToDestory();		

	/**
	* \fn								 STPlayStatus GetPlayStatus();
	* \brief							 获取播放状态
	* \return							 播放状态	
	*/
	STPlayStatus						 GetPlayStatus();

	/**
	* \fn								 STUint Position();
	* \brief							 获取播放位置
	* \return							 播放位置，单位毫秒
	*/
	STUint								 Position();

	/**
	* \fn								 STInt GetCurWave(STInt16* aWave, STInt aSamples, STInt& aChannels);
	* \brief							 获取当前播放的数据
	* \param[in] aWave					 采样点的数据指针	
	* \param[in]  aSamples				 取采样的个数	
	* \param[out] nChannels				 音频声道数	
	* \return							 返回状态
	*/
	STInt								 GetCurWave(STInt16* aWave, STInt aSamples, STInt& aChannels);
	
	/**
	* \fn								 STInt GetCurWave(STInt16* aWave, STInt aSamples, STInt& aChannels);
	* \brief							 获取当前录制的数据
	* \param[in] aWave					 采样点的数据指针
	* \param[in]  aSamples				 取采样的个数
	* \param[out] nChannels				 音频声道数
	* \return							 返回状态
	*/
	STInt 								 GetCurRecordWave(STInt16* aWave, STInt aSamples, STInt& aChannels);

	/**
	* \fn								 void SetPlayStatus(STPlayStatus aStatus)
	* \brief							 设置播放状态
	* \param[in] aStatus				 当前播放状态	
	*/
	void								 SetPlayStatus(STPlayStatus aStatus);

	/**
	* \fn								 STUint Duration();
	* \brief							 获取时长
	* \return							 时长值，单位毫秒	
	*/
	STUint								 Duration();

	/**
    * \fn								 void SetAccompanimentVolume(STInt aVolume);
    * \brief							 设置播放声音
    */
    void								 SetAccompanimentVolume(STInt aVolume);

	/**
    * \fn								 void SetRecorderVolume(STInt aVolume);
    * \brief							 设置录音声音
    */
	void								 SetRecorderVolume(STInt aVolume);

	/**									 void Abort()
	 *									 取消操作
	 */
	void								 Abort();

private:
	/**
	* \fn								 STInt Open(const STChar* aUrl);
	* \brief							 Open
	* \param[in] aUrl					 数据源路径	
	* \param[in] aParams			     相关参数
	* \return							 操作状态
	*/
	STInt								 Open(const STChar* aUrl, const STChar* aParams);

	/**
	* \fn								 void Close();
	* \brief							 Close
	*/
	void								 Close();

	/**
	* \fn								 void Pause();
	* \brief							 Pause
	*/
	void								 Pause();
	
	/**
	* \fn								 void Resume();
	* \brief							 Resume
	*/
	void								 Resume();
	
	/**
	* \fn								 void Start();
	* \brief							 Start
	*/
	void								 Start();

	/**
	* \fn								 void Stop();
	* \brief							 Stop
	*/
	void								 Stop();

	/**
	* \fn								 void Seek(STUint aPosition)
	* \brief						  	 Seek操作
	* \param [in]   aPosition			 单位为毫秒
	*/
	void								 Seek(STUint aPosition);

private://ISTMsgHandle
	
	/**
	* \fn								 void HandleMsg(STMsg& aMsg);
	* \brief							 线程通信消息处理函数
	* \param[in] aMsg					 消息引用
	*/
	virtual void						 HandleMsg(STMsg& aMsg);

	/**
	* \fn                           	 STInt Switch2Stream(const STChar* aStreamName);
	* \brief                        	 切换到某条流
	* \param[in]    aStreamName    	 	 指定流的名字
	* \return                       	 操作状态
	*/	
	STInt 								 Switch2Stream(const STChar *aStreamName);
private:
	void								 Flush();		
	STInt								 FillAudioOutput();

private:
	ISTPlayControlObserver&				 iPlayControlObserver;
	STBool								 iIsToDestory;
	STCritical							 iCritical;
	STPlayStatus						 iPlayStatus;
	ISTPluginItf*						 iCurPlugin;
	STPluginManager*					 iPluginManager;
	ISTAudioOutputItf*					 iAudioOutput;
	STSampleBufferManager*				 iSampleBufferManager;
	STChar								 iTempBuffer[KMaxPathLength];
	STChar								 iTempFullPathBuffer[KMaxPathLength];
};

#endif
