#ifndef __ST_AUDIO_OUTPUT_H__
#define __ST_AUDIO_OUTPUT_H__
#include "STBaseAudioOutput.h"
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
class STAudioOutput : public STBaseAudioOutput
{
public:
	STAudioOutput();
	~STAudioOutput();
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
	* \fn                       void Open(STInt aSampleRate, STInt aChannels)
	* \brief                    打开设备
	* \param[in] aSampleRate	采样率
	* \param[in] aChannels		声道数
	* \param[in] aRecordSupported 是否支持录制
	* \return					STKErrNone为成功
	*/
	virtual STInt				Open(STInt aSampleRate, STInt aChannels, STBool aRecordSupported);

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
    * \fn                       void SetAccompanimentVolume(STInt aVolume);
    * \brief                    设置伴奏声音
    */
    virtual void                SetAccompanimentVolume(STInt aVolume);

    /**
    * \fn                       void SetRecorderVolume(STInt aVolume);
    * \brief                    设置录音声音
    */
    virtual void                SetRecorderVolume(STInt aVolume);

public:
	void						Render();

private:
	static void CALLBACK waveOutProc(HWAVEOUT hWaveOut, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
	void			     waveOutProcL();
	void				 writeAudio(HWAVEOUT hWaveOut, LPSTR data, int size);
private:
	STBool						iIsFlushed;
	STInt16*					iCurPCM;

	CRITICAL_SECTION			iCriticalSection;
	WAVEHDR*					iWaveBlocks;
	int							iWaveFreeBlockCount;
	int							iWaveCurrentBlock;
	HWAVEOUT					iHandleWaveOut; /* device handle */
	STBool						iRenderEnable;
};

#endif