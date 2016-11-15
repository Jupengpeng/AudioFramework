/**
* File : TTMediaPlayerItf.h 
* Created on : 2011-3-11
* Author : hu.cao
* Copyright : Copyright (c) 2011 Shuidushi Software Ltd. All rights reserved.
* Description : ITTMediaPlayer ???????
*/

#ifndef __TT_MEDIA_PLAYER_ITF_H__
#define __TT_MEDIA_PLAYER_ITF_H__

#include "TTOsalConfig.h"
#include "TTMacrodef.h"
#include "TTTypedef.h"
#include "TTInterface.h"
#include "TTNetWorkConfig.h"

class ITTAudioEffect;
class ITTEqualizer;

enum TTNotifyMsg
{
	ENotifyNone = 0
	, ENotifyPrepare = 1
	, ENotifyPlay = 2
	, ENotifyComplete = 3
	, ENotifyPause = 4
	, ENotifyClose = 5
	, ENotifyException = 6
	, ENotifyUpdateDuration = 7
#ifdef __TT_OS_IOS__
    , ENotifyAssetReaderFail = 8
    , ENotifyStop = 9
#endif
	, ENotifyTimeReset = 10
	, ENotifySeekComplete = 11
	, ENotifyAudioFormatChanged = 12
	, ENotifyVideoFormatChanged = 13
	, ENotifyBufferingStart = 16
	, ENotifyBufferingDone = 17
	, ENotifyDNSDone = 18
	, ENotifyConnectDone = 19
	, ENotifyHttpHeaderReceived = 20
    , ENotifyPrefetchStart = 21
	, ENotifyPrefetchCompleted = 22
	, ENotifyCacheCompleted = 23
	, ENotifyMediaStartToOpen = 24
	, ENotifyMediaFirstFrame = 25
	, ENotifyMediaPreOpenStart = 26
	, ENotifyMediaPreOpenSucess = 27
	, ENotifyMediaPreOpenFailed = 28

	, ENotifyMediaChangedStart = 50
	, ENotifyMediaChangedSucess = 51
	, ENotifyMediaChangedFailed = 52

	, ENotifyMediaPreOpen = 80
	, ENotifyMediaPreRelease = 81
	, ENotifyMediaOpenLoaded = 82
	
	, ENotifySCMediaStartToOpen = 100
	, ENotifySCMediaStartToPlay = 101
	, ENotifySCMediaOpenSeek = 102
	, ENotifySCMediaPlaySeek = 103

	, ENotifyPlayerInfo = 200
};

enum TTPlayStatus
{
	EStatusStarting = 1
	, EStatusPlaying = 2
	, EStatusPaused = 3
	, EStatusStoped = 4
	, EStatusPrepared = 5
};

enum TTDecoderType
{
	EDecoderDefault = 0
	, EDecoderSoft = 1
};

enum TTSourceFlag
{
	ESourceDefault = 0x0
	, ESourceBuffer = 0x1
	, ESourcePreOpen = 0x2
	, ESourceOpenLoaded = 0x4
	, ESourceChanged = 0x8
};

class ITTMediaPlayerObserver
{
public:
	/**
	* \fn                           void PlayerNotifyEvent(TTNotifyMsg aMsg, TTInt aArg1, TTInt aArg2, const TTChar* aArg3)
	* \brief                        ??????????????????
	* \param [in]      aMsg   		????ID
	* \param [in]	   aArg1        ????1
	* \param [in]	   aArg2		????2
	* \param [in]	   aArg3		????3
	*/
	virtual void PlayerNotifyEvent(TTNotifyMsg aMsg, TTInt aArg1, TTInt aArg2, const TTChar* aArg3) = 0;
};

class ITTMediaPlayer : public ITTInterface
{
public:
	/**
	* \fn                           TTUint Duration
	* \brief                        ??????
	* \return   					????????????λ
	*/
	virtual TTUint		            Duration() = 0;

    /**
	* \fn                           TTUint Size()
	* \brief                        ???????С
	* \return   					????С????λ???
	*/
	virtual TTUint		            Size() = 0;

    /**
    * \fn                           const TTChar* Url()
    * \brief                        ??????????
    * \return   					?????
    */
    virtual const TTChar*           Url() = 0;
    
	/**
	* \fn                           TTInt SetDataSourceSync(const TTChar* aUrl)
	* \brief                        ??????Src
	* \param [in]   aUrl			???·??
	* \return						??????
	*/
	virtual TTInt					SetDataSourceSync(const TTChar* aUrl, TTInt nFlag = 0) = 0;

	/**
	* \fn                           TTInt SetDataSourceAsync(const TTChar* aUrl)
	* \brief                        ??????src
	* \param [in]   aUrl			???·??
	* \return						??????
	*/
	virtual TTInt					SetDataSourceAsync(const TTChar* aUrl, TTInt nFlag = 0) = 0;

	/**
	* \fn                           TTInt SetParam(TTInt nID, void* aParam)
	* \brief                        ???ò???
	* \param [in]   nID				????ID???????????
	* \param [in]   aParam			?????
	* \return						??????
	*/
	virtual TTInt					SetParam(TTInt nID, void* aParam) = 0;

	/**
	* \fn                           TTInt GetParam(TTInt nID, void* aParam)
	* \brief                        ???????
	* \param [in]   nID				????ID???????????
	* \param [in]   aParam			?????
	* \return						??????
	*/
	virtual TTInt					GetParam(TTInt nID, void* aParam) = 0;

	/**
	* \fn                           void Play()
	* \brief                        ????????
	* \return						??????
	*/
	virtual TTInt					Play() = 0;

	/**
	* \fn                           void Pause()
	* \brief                        Pause
	*/
	virtual void					Pause(TTBool aFadeOut = ETTFalse)= 0;

	/**
	* \fn                           void Resume()
	* \brief                        Resume
	*/
	virtual void                    Resume(TTBool aFadeIn = ETTFalse)= 0;
	/**
	* \fn                           void Stop()
	* \brief                        ??????
	* \param [in]   aSync			??????stop??default????
	* \return                       ??????
	*/
	virtual TTInt                   Stop(TTBool aSync = false) = 0;

	/**
	* \fn                           void SetVolume(TTInt aLVolume, TTInt aRVolume);
	* \brief                        ????????
	* \param [in]   aLVolume         ?????
	* \param [in]   aRVolume         ?????
	*/
	virtual void                    SetVolume(TTInt aLVolume, TTInt aRVolume) = 0;

	/**
	* \fn                           TInt GetVolume();
	* \brief                        ???????
	* \return                       ?????
	*/
	virtual TTInt                   GetVolume() = 0;

	/**
	* \fn                           void SetView(TTInt aLVolume, TTInt aRVolume);
	* \brief                        ????????
	* \param [in]   aLVolume         ?????
	* \param [in]   aRVolume         ?????
	*/
	virtual void                    SetView(void* aView) = 0;

	/**
	* \fn                           TTPlayStatus GetPlayStatus();
	* \brief                        ???????????
	* \return                       ??
	*/
	virtual TTPlayStatus            GetPlayStatus() = 0;

	/**
	* \fn                           void SetPosition(TTUint aPosition)
	* \brief                        ???ò???λ??
	* \param [in]   aPosition       λ?????????????λ
	* \param [in]   aOption	        ?????????????seek??????seek
	*/
	virtual TTInt64                 SetPosition(TTInt64 aPosition, TTInt aOption = 0) = 0;

	/**
	* \fn                           TTUint GetPosition()
	* \brief                        ??????????
	* \return                       λ?????????????λ
	*/
	virtual TTUint64				GetPosition() = 0;

	/**
	* \fn                           SetVolumeRamp(TTUint aFadeIn, TTUint aFadeOut)
	* \brief                        ?????????????????
	* \param [in]   aFadeIn         ???????
	* \param [in]   aFadeOut        ???????
	*/
	//virtual void                  SetVolumeRamp(TTUint aFadeIn,  TTUint aFadeOut) = 0;

	/**
	* \fn                           void SetPlayWindow(TTInt aReaptedNum, TTUint aStart, TTUint aEnd)
	* \brief                        ??????????????????????
	* \param [in]   aReaptedNum     ????????
	* \param [in]   aStart          ??????????
	* \param [in]   aEnd            ??????????
	*/
	//virtual void                  SetPlayWindow(TTInt aReaptedNum, TTUint aStart, TTUint aEnd) = 0;

	/**
	* \fn                           void RepeatCancel()
	* \brief                        ??????????
	*/
	//virtual void                  RepeatCancel() = 0;

	/**
	* \fn                           TTInt GetCurrentFreqAndWave(TTInt16 *aFreq, TTInt16 *aWave, TTInt aSampleNum);
	* \brief                        ?????????
	* \param[out]    aFreq          ??????????????????????NULL
	* \param[out]    aWave          ???????????,??????
	* \param[in]     aSampleNum     ??????????
	* \return                       ??????
	*/
	virtual TTInt                   GetCurrentFreqAndWave(TTInt16 *aFreq, TTInt16 *aWave, TTInt aSampleNum) = 0;

	/**
	* \fn							void SetPlayRange(TTUint aStartTime, TTUint aEndTime);
	* \brief						???ò????Χ
	* \param[in]	aStartTime		?????Χ????????λ????
	* \param[in]	aEndTime		?????Χ?????????λ????
	*/
	virtual void					SetPlayRange(TTUint aStartTime, TTUint aEndTime) = 0;

	/**
	* \fn							TTUint BufferedSize()
	* \brief						????????С
	* \return						?????С(??λ:???)
	*/
	virtual	TTUint					BufferedSize() = 0;


	/**
	* \fn							TTUint BandWidth()
	* \brief						???????????????С
	* \return						?????С(??λ:???)
	*/
	virtual	TTUint					BandWidth() = 0;

	/**
	* \fn							TTUint BandPercent()
	* \brief						??????????????
	* \return						?????С(??λ:???)
	*/
	virtual	TTUint					BandPercent() = 0;

	/**
	* \fn							TTInt BufferedPercent(TTInt& aBufferedPercent)
	* \brief						?????????????
	* \param[out]	aBufferedPercent????
	* \return						??????
	*/
	virtual TTInt					BufferedPercent(TTInt& aBufferedPercent) = 0;

	/**
	* \fn							void SetActiveNetWorkType(TTActiveNetWorkType aNetWorkType)
	* \brief						????????????????
	* \param[in]	aNetWorkType	????????????
	*/
	virtual void					SetActiveNetWorkType(TTActiveNetWorkType aNetWorkType) = 0;


	/**
	* \fn							void SetNetWorkProxy(TTBool aNetWorkProxy)
	* \brief						??????????????
	* \param[in]	aNetWorkProxy	??????????
	*/
	virtual void					SetNetWorkProxy(TTBool aNetWorkProxy) = 0;

	/**
	* \fn							void SetDecoderType(TTDecoderType aDecoderType)
	* \brief						???????????
	* \param[in]	aDecoderType	????????????
	*/
	virtual void					SetDecoderType(TTDecoderType aDecoderType) = 0;

#ifdef __TT_OS_IOS__
    /**
     * \fn						void SetBalanceChannel(float aVolume)
     * \brief                          set balane channel volume
     * \param[in] aVolume  ,value range: [-1,+1]
     */
    virtual void                    SetBalanceChannel(float aVolume) = 0;
    
    virtual void                    SetRotate() = 0;
#endif

	/**
	* \fn                           void SetCacheFilePath(const TTChar* aCacheFilePath)
	* \brief                        ?????????????????????·??
	* \param[in] aCacheFilePath     ???·??
	*/
	virtual void					SetCacheFilePath(const TTChar* aCacheFilePath) = 0;

		/**
	* \fn							void Decode(const TTChar* aUrl, TTInt size)
	* \brief					    ???????????
	* \param[in] aUrl			    ????url	
	* \param[in] size			    ????????pcm????size	
	* \return						??????		
	*/
	virtual	TTInt					Decode(const TTChar* aUrl, TTInt size)= 0;

	/**
	* \fn							TTInt GetPCMData();
	* \brief						???????????
	* \return						??????????		
	*/
	virtual TTUint8*				GetPCMData()= 0;

	/**
	* \fn							TTInt GetPCMDataSize();
	* \brief						?????????????
	* \return						???????????		
	*/
	virtual TTInt				    GetPCMDataSize()= 0;

	/**
	* \fn							void GetChannels();
	* \brief			            ?????????
	* \return						?????	
	*/
	virtual TTInt				    GetPCMDataChannle()= 0;

	/**
	* \fn							void GetSamplerate();
	* \brief			            ?????????
	* \return						???????	
	*/
	virtual TTInt				    GetPCMDataSamplerate()= 0;

	/**
	* \fn							void CancelGetPCM();
	* \brief			            ???????	
	*/
	virtual void					CancelGetPCM()= 0;

	/**
	* \fn							void SetTimeRange(TTInt start, TTInt end, TTInt decodeStartPos)
	* \brief						???ò????Χ
	* \param[in] start			    ????	
	* \param[in] end			    ?????	
	* \param[in] decodeStartPos		???????λ???	
	*/
	virtual void					SetTimeRange(TTInt start, TTInt end, TTInt decodeStartPos)= 0;


#ifdef __TT_OS_ANDROID__
	/**
	* \fn							void SaveAudioTrackJClass(void* pJclass)
	* \brief						????JNI??????
	* \param[in]	pJclass	        java?? AudioTrack class
	*/
	virtual	void					SaveAudioTrackJClass(void* pJclass) = 0;

    /**
     * \fn							void GetAudiotrackClass();
     * \brief						???java?? AudioTrack class,???ITTMediaPlayer??????
	 * \return                      AudioTrack class pointer
     */
	virtual void*					GetAudiotrackClass() = 0;


	virtual void 				    SetMaxOutPutSamplerate(TTInt aSampleRate) =0;

	/**
	* \fn							void SaveVideoTrackJClass(void* pJclass)
	* \brief						????JNI??????
	* \param[in]	pJclass	        java?? VideoTrack class
	*/
	virtual	void					SaveVideoTrackJClass(void* pJclass) = 0;

    /**
     * \fn							void GetVideotrackClass();
     * \brief						???java?? VideoTrack class,???ITTMediaPlayer??????
	 * \return                      VideoTrack class pointer
     */
	virtual void*					GetVideotrackClass() = 0;

#endif

};


class CTTMediaPlayerFactory
{

public:

	/**
	* \fn                               ITTMediaPlayer* NewL(ITTMediaPlayerObserver* aPlayerObserver)
	* \brief                            ????ITTMediaPlayer??????
	* \param[in]	aPlayerObserver		MediaPlayer?????
	* \return                           ITTMediaPlayer?????????
	*/
	static ITTMediaPlayer*              NewL(ITTMediaPlayerObserver* aPlayerObserver);
};

#endif
