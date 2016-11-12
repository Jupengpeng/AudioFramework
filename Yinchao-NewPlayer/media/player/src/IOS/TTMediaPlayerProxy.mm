/**
* File : TTMediaPlayerProxy.mm
* Created on : 2011-9-22
* Author : hu.cao
* Copyright : Copyright (c) 2011 Shuidushi Software Ltd. All rights reserved.
* Description : TTMediaPlayerProxy µœ÷
*/

#import "TTMediaPlayerProxy.h"
#import <AVFoundation/AVFoundation.h>
#import <AudioToolbox/AudioToolbox.h>
#import <UIKit/UIApplication.h>
#import <CoreTelephony/CTCallCenter.h>
#import <CoreTelephony/CTCall.h>

#include "TTLog.h"
#include "TTUIDeviceHardware.h"
#include "TTBackgroundConfig.h"
#include <TargetConditionals.h>

#if TARGET_RT_BIG_ENDIAN
#   define FourCC2Str(fourcc) (const char[]){*((char*)&fourcc), *(((char*)&fourcc)+1), *(((char*)&fourcc)+2), *(((char*)&fourcc)+3),0}
#else
#   define FourCC2Str(fourcc) (const char[]){*(((char*)&fourcc)+3), *(((char*)&fourcc)+2), *(((char*)&fourcc)+1), *(((char*)&fourcc)+0),0}
#endif


NSString *NewCallingNotification = @"NewCallingNotification";
NSString *kIsCalling = @"kIsCalling";

Boolean gIsIOS4X = ETTFalse;
static TTBool gRouteChangePaused = ETTFalse;
extern TTBool gSlientImmediately;
TTInt  gAudioEffectLowDelay = 1;

static BOOL callingBegin = NO;

static TTBackgroundAssetReaderConfig gGeminiLeft;
static TTBackgroundAssetReaderConfig gGeminiRight;
static TTPlayStatus playerStatusBeforeCalling = EStatusPaused;


@implementation TTMsgObject

@synthesize iMsg;
@synthesize iError;

- (id) initwithMsg : (TTNotifyMsg) aMsg andError: (TTInt) aError
{
    iMsg = aMsg;
    iError = aError;
    
    return self;
}

@end

@interface TTMediaPlayerProxy()

@property (nonatomic, retain) CTCallCenter *phoneCall;

- (void)dealloc;
- (TTInt) setDataSource : (NSString*) aUrl;
- (void) notifyProcessProcL : (id) aMsgObject;
- (void) backgroundIssueProcess : (TTMsgObject*) aMsg;
- (TTMsgObject *) playIssueProcess:(TTMsgObject *)aMsg;
- (void) setupAudioSession;
- (void) onTimer;
- (void) startCountDown;
- (void) stopCountDown;
- (void) geminiProcL : (NSObject*) aObj;
@end

@implementation TTMediaPlayerProxy

@synthesize interruptedWhilePlaying;

- (id) init                                                             
{   
    gIsIOS4X = [[TTUIDeviceHardware instance] IsSystemVersion4X];
    NSLogDebug(@"gIsIOS4X:%d", gIsIOS4X);
    
    [self setupAudioSession];
    iPlayer = new CTTMediaPlayerWarp((void*)self);
        
    iUrlUpdated = false;
    
    iAssetReaderFailOrLongPaused =false;
    
    iCurUrl = NULL;
    
    iPowerDownCountDownTimer = NULL;
        
    interruptedWhilePlaying = false;   
        
    iGeminiUrl = NULL;
        
    if (gIsIOS4X)
    {
        iCritical = [[NSLock alloc] init];
        iGeminiDone = ETTFalse;
        iGeminiThreadHandle = [[NSThread alloc] initWithTarget:self selector:@selector(geminiProcL:) object:nil];
    }
    else
    {
        iCritical = NULL;
        iGeminiDone = ETTTrue;
        iGeminiThreadHandle = NULL;
    }
    
    return self;
}

- (void)dealloc
{    
    iGeminiDone = ETTTrue;
    
    self.phoneCall = nil;
    
    if (iCurUrl != NULL)
    {
        [iCurUrl release];
        iCurUrl = NULL;
    }
        
    if (iGeminiThreadHandle != NULL)
    {
        [iGeminiThreadHandle release];
        iGeminiThreadHandle = NULL;
    }
    
    if (iCritical != NULL)
    {
        [iCritical release];
    }
    
    if (iGeminiUrl != NULL)
    {
        [iGeminiUrl release];
    }
    
    [self stopCountDown];
    
        
    SAFE_DELETE(iPlayer);
    
    [super dealloc];
}

void audioRouteChangeListenerCallback(void *inClientData,AudioSessionPropertyID	inID,UInt32 inDataSize,const void *inData)
{
    NSLogDebug(@"audioRouteChangeListenerCallback");
    
    TTMediaPlayerProxy* player = (TTMediaPlayerProxy*)inClientData;
    if (inID == kAudioSessionProperty_AudioRouteChange)
    {
        CFDictionaryRef routeDict = (CFDictionaryRef)inData;
        NSNumber* reasonValue = (NSNumber*)CFDictionaryGetValue(routeDict, CFSTR(kAudioSession_AudioRouteChangeKey_Reason));
        
        int reason = [reasonValue intValue];
        NSLogDebug(@"Audio Route Change Reason,%d", reason);
        if (reason == kAudioSessionRouteChangeReason_OldDeviceUnavailable)
        {
            gSlientImmediately = ETTTrue;
            [player pause];
            gRouteChangePaused = ETTTrue;
            playerStatusBeforeCalling = EStatusPaused;
        }
        else if(reason == kAudioSessionRouteChangeReason_NewDeviceAvailable)
        {
            gSlientImmediately = ETTFalse;
            gRouteChangePaused = ETTFalse;
        }else if(reason == kAudioSessionRouteChangeReason_Override)
        {
            if ([player isOtherMusicPlaying]) {
                [player pause];
                return;
            }
            
            if ([player isUnderPlaybackCategory]){
                double delayInSeconds = 1.0;
                dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, (int64_t)(delayInSeconds * NSEC_PER_SEC));
                dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
                    if (player.interruptedWhilePlaying) {
                        [player playWhenInterruptionFinished];
                    }
                });
            }
        }
    }
    
}

- (void)audioRouteChanged:(UInt32)reason withRouteDescription:(CFDictionaryRef)routeDict{
    static BOOL audioSessionCategoryChanged = NO;
    if (callingBegin) {
        return;
    }
    
    if (reason == kAudioSessionRouteChangeReason_CategoryChange) {
        if (!audioSessionCategoryChanged && interruptedWhilePlaying) {
            audioSessionCategoryChanged = YES;
            
            if (iPlayer->GetPlayStatus() == EStatusPlaying){
                [self pause];
            }
        }else if (audioSessionCategoryChanged && interruptedWhilePlaying){
            audioSessionCategoryChanged = NO;
            
            //reset when interrupted by recorded app, in this situation endInterruptionWithFlags:(NSUInteger)flags
            //never called.
            interruptedWhilePlaying = NO;
            
            if (![self isOtherMusicPlaying]) {
                AudioSessionSetActive(YES);
                if ([self isMixWithOthers]) {
                    [self resumeAndSetMixOthers];
                }
            }else {
                [self setAudioSessionMixWithOther:0];
            }
        }
    }else if (reason == kAudioSessionRouteChangeReason_Override && ![[TTUIDeviceHardware instance] isSystemVersionLargeEqual6]){
        if (!audioSessionCategoryChanged && interruptedWhilePlaying) {
            audioSessionCategoryChanged = YES;
            if ([[TTUIDeviceHardware instance] isSystemVersion5X]) {
                [self pause];
            }
        }
    }
}

//determine when player is inpterrupted by other video recorded app
//@para begin  which label the state of this interrupte, it have two states:begin and end.
//the begin state respresents that the audio session just changed to record mode, while
//the end state respresents the audio session just recorved from record mode.
- (BOOL)isInterruptedByRecordSoftware:(CFDictionaryRef)routeDict state:(BOOL)begin{
    BOOL isInterrupted = NO;
    
    CFDictionaryRef previousRouteDes = (CFDictionaryRef)CFDictionaryGetValue(routeDict, kAudioSession_AudioRouteChangeKey_PreviousRouteDescription);
    CFDictionaryRef currentRouteDes = (CFDictionaryRef)CFDictionaryGetValue(routeDict, kAudioSession_AudioRouteChangeKey_CurrentRouteDescription);
    
    CFArrayRef previousInputs = (CFArrayRef)CFDictionaryGetValue(previousRouteDes,  kAudioSession_AudioRouteKey_Inputs);
    CFArrayRef currentInputs = (CFArrayRef)CFDictionaryGetValue(currentRouteDes,  kAudioSession_AudioRouteKey_Inputs);
    NSLog(@"previos inputs:%@",previousInputs);
    NSLog(@"current inputs:%@",currentInputs);
    
    if (begin) {
        if (CFArrayGetCount(previousInputs) == 0 && CFArrayGetCount(currentInputs)) {
            isInterrupted = YES;
        }
    }else {
        if (CFArrayGetCount(previousInputs) && CFArrayGetCount(currentInputs) == 0) {
            isInterrupted = YES;
        }
    }
    
    NSLog(@"isInterrupted:%d",isInterrupted);
    return isInterrupted;
}

- (void) setupAudioSession
{
    NSError *sessionError = nil;
    [[AVAudioSession sharedInstance] setCategory:AVAudioSessionCategoryPlayback error:&sessionError];
    
    if(sessionError)
    {
        NSLogDebug(@"%@", sessionError);
    }
    else
    {   sessionError = nil;
        
        if (iOSVersion>6.0) {
            [[AVAudioSession sharedInstance] setActive:YES withOptions:kAudioSessionSetActiveFlag_NotifyOthersOnDeactivation error:&sessionError];
        }else{
            [[AVAudioSession sharedInstance] setActive:YES withFlags:kAudioSessionSetActiveFlag_NotifyOthersOnDeactivation error:&sessionError];
        }
        
        if(sessionError)
        {
            NSLogDebug(@"%@", sessionError);
        }      
        
        AudioSessionAddPropertyListener(kAudioSessionProperty_AudioRouteChange, audioRouteChangeListenerCallback, self);
    }   
      
    if (gIsIOS4X)
    {
        [self setAudioSessionMixWithOther:1];
    }
    
    [[AVAudioSession sharedInstance] setDelegate:self];    
    [[AVAudioSession sharedInstance] setActive:YES error:&sessionError];
    
    if(sessionError)
    {
        NSLogDebug(@"%@", sessionError);
    }
    
    //handle calling coming or call others
    [self handlePhoneComing];
}

- (BOOL)setAudioSessionMixWithOther:(UInt32)mixWithOthers{
    OSStatus tStatus = AudioSessionSetProperty (kAudioSessionProperty_OverrideCategoryMixWithOthers,
                                                sizeof (mixWithOthers),
                                                &mixWithOthers
                                                );
    NSLogDebug(@"AudioSession Config tStatus:%d",(int)tStatus);
    
    return !tStatus;
}

- (BOOL)isMixWithOthers{
    UInt32 mixWithOthers = 0;
    UInt32 dataSize = sizeof(mixWithOthers);
    OSStatus result = AudioSessionGetProperty(kAudioSessionProperty_OverrideCategoryMixWithOthers, &dataSize, &mixWithOthers);
    
    NSLogDebug(@"AudioSession mix with others:%lu result:%ld",mixWithOthers,result);
    return mixWithOthers;
}

- (BOOL)isOtherMusicPlaying{
    UInt32 otherAudioIsPlay = 0;
    UInt32 dataSize = sizeof(otherAudioIsPlay);
    OSStatus result = AudioSessionGetProperty(kAudioSessionProperty_OtherAudioIsPlaying, &dataSize,&otherAudioIsPlay);
    NSLog(@"resut:%ld other is play:%ld",result,otherAudioIsPlay);
    
    return otherAudioIsPlay;
}

- (BOOL)isUnderPlaybackCategory{
    UInt32 audioSessionCategory = 0;
    UInt32 playBackAudioSession = kAudioSessionCategory_MediaPlayback;
    UInt32 dataSize = sizeof(audioSessionCategory);
    OSStatus result = AudioSessionGetProperty(kAudioSessionProperty_AudioCategory, &dataSize,&audioSessionCategory);
    BOOL isUnderPlayback = NO;
    
    NSString *currentAudioSessionString = @(FourCC2Str(audioSessionCategory));
    NSString *mediaPlaybackAudioSessionString = @(FourCC2Str(playBackAudioSession));
    if ([currentAudioSessionString isEqual:mediaPlaybackAudioSessionString]) {
        isUnderPlayback = YES;
    }
    printf("resut:%ld is under playback category:%hhd\n",result,isUnderPlayback);
    
    return isUnderPlayback;
}

- (void)setPlayBackAudioSession:(BOOL)active {
    [self isUnderPlaybackCategory];
    [[AVAudioSession sharedInstance] setCategory:AVAudioSessionCategoryPlayback error:nil];
    [[AVAudioSession sharedInstance] setActive:active error:nil];
}

- (void)handlePhoneComing{
    if (self.phoneCall == nil) {
        self.phoneCall = [[[CTCallCenter alloc] init] autorelease];
        self.phoneCall.callEventHandler = ^(CTCall *call){
            NSLog(@"call:%@",call);
            if (call.callState == CTCallStateDisconnected){
                if (playerStatusBeforeCalling == EStatusPlaying) {
                    [self resumeAndSetMixOthers];
                }
                
                callingBegin = NO;
                playerStatusBeforeCalling = EStatusPaused;
            }else if (call.callState == CTCallStateDialing || call.callState == CTCallStateIncoming) {
                playerStatusBeforeCalling = [self getPlayStatus];
                [self pause];
                
                callingBegin = YES;
            }
            
            //send calling notification
            dispatch_async(dispatch_get_main_queue(), ^{
                NSDictionary *dictionary = [NSDictionary dictionaryWithObject:[NSNumber numberWithBool:callingBegin] forKey:kIsCalling];
                [[NSNotificationCenter defaultCenter] postNotification:[NSNotification notificationWithName:NewCallingNotification object:self userInfo:dictionary]];
            });
        };
    }
}

- (TTInt) start
{
    [self setPlayBackAudioSession:YES];
    playerPausedManually = NO;
    
    NSLogDebug(@"Proxy start");
    gSlientImmediately = ETTFalse;
    TTASSERT(iPlayer != NULL);
    return iPlayer->Play();
}

- (void) pause
{
    playerPausedManually = YES;
    
    NSLog(@"Proxy pause");
    TTASSERT(iPlayer != NULL);
    iPlayer->Pause();
}

- (void) inactiveAudioSession
{
    [self setPlayBackAudioSession:NO];
}

- (void) resume
{
    [self setPlayBackAudioSession:YES];
    playerPausedManually = NO;

    NSLogDebug(@"Proxy resume");
    TTASSERT(iPlayer != NULL);
    if (iAssetReaderFailOrLongPaused)
    {
        iAssetReaderFailOrLongPaused = false;
        iPlayer->SetPosition(iPlayer->GetPosition());        
    }
    
    gSlientImmediately = ETTFalse;
    
    iPlayer->Resume();       
}

- (void)resumeAndSetMixOthers{
    if (!gIsIOS4X) {
        [self setAudioSessionMixWithOther:0];
    }
    
    [self resume];
}

- (void) setPosition : (CMTime) aTime
{
    NSLogDebug(@"Proxy setPostion");
    TTASSERT(iPlayer != NULL);
    TTUint64 ntmp = aTime.value;
    ntmp *= 1000;
    TTUint32 nMillSecPos = ntmp / aTime.timescale;
    iPlayer->SetPosition(nMillSecPos);    
}

- (void) setPlayRangeWithStartTime : (CMTime) aStartTime EndTime : (CMTime) aEndTime
{    
    TTASSERT(iPlayer != NULL);
    TTUint64 ntmp = aStartTime.value;
    ntmp *= 1000;
    TTUint32 nStartMillSecPos = ntmp / aStartTime.timescale;
    
    ntmp = aEndTime.value;
    ntmp *= 1000;
    TTUint32 nEndMillSecPos = ntmp / aEndTime.timescale;
    
    NSLogDebug(@"Proxy setPlayRange:%ld--%ld", nStartMillSecPos, nEndMillSecPos);
    if (nEndMillSecPos > nStartMillSecPos)
    {
        iPlayer->SetPlayRange(nStartMillSecPos, nEndMillSecPos);
    }
}

- (CMTime) getPosition
{
    TTASSERT(iPlayer != NULL);
    return CMTimeMake(iPlayer->GetPosition(), 1000);
}

- (CMTime) duration
{
    TTASSERT(iPlayer != NULL);
    return CMTimeMake(iPlayer->Duration(), 1000);
}

- (TTInt) getCurFreqAndWaveWithFreqBuffer : (TTInt16*) aFreqBuffer andWaveBuffer : (TTInt16*) aWaveBuffer andSamplenum :(TTInt) aSampleNum
{
    return iPlayer->GetCurFreqAndWave(aFreqBuffer, aWaveBuffer, aSampleNum);
}

- (TTPlayStatus) getPlayStatus
{
    TTASSERT(iPlayer != NULL);
    return iPlayer->GetPlayStatus();
}

- (void) ProcessNotifyEventWithMsg : (TTNotifyMsg) aMsg andError: (TTInt) aError
{
    TTMsgObject* pMsgObject = [[TTMsgObject alloc] initwithMsg : aMsg andError : aError];
    [self performSelectorOnMainThread:@selector(notifyProcessProcL:) withObject:pMsgObject waitUntilDone:NO];
}

- (void) notifyProcessProcL : (id) aMsgObject;
{
    [self backgroundIssueProcess:(TTMsgObject *)aMsgObject];
    TTMsgObject* pMsg = [self playIssueProcess:(TTMsgObject *)aMsgObject];
    
    if (pMsg != NULL)
    {
        [[NSNotificationCenter defaultCenter] postNotificationName:@"PlayerNotifyEvent"     object:pMsg];
        [pMsg release];
    }
}
 
- (TTMsgObject *) playIssueProcess : (TTMsgObject *)aMsg
{    
    TTMsgObject* pMsgObject = aMsg;
    TTInt nMsgId = pMsgObject.iMsg;
    TTInt errorCode = pMsgObject.iError;
    if (nMsgId == ENotifyStop)
    {
        if (iUrlUpdated)//change mediaItem
        {
            if (TTKErrNone != [self setDataSource : iCurUrl])
            {
                iPlayer->Stop();
            }
            
            [pMsgObject release];
            pMsgObject = NULL;
        }
        //cmd stop
    }        
    else if (nMsgId == ENotifyPrepare)
    {
        if (iUrlUpdated)
        {
            iPlayer->Stop();
            [pMsgObject release];
            pMsgObject = NULL;
        }        
    } 
    else if (nMsgId == ENotifyAssetReaderFail)
    {
        if (gIsIOS4X){
            [self pause];
            iAssetReaderFailOrLongPaused = true;
        }else if (errorCode == TTKErrOperationInterrupted) {
            iAssetReaderFailOrLongPaused = true;
            
            //resume from readerfail right now
            [self playWhenInterruptionFinished];
        }else {
            [pMsgObject release];
            pMsgObject = [[TTMsgObject alloc] initwithMsg:ENotifyException andError:TTKErrNotSupported];
        }
    }    
    /*if (nMsgId == ENotifyStop || nMsgId == ENotifyComplete || nMsgId == ENotifyException)
    {
        [self startCountDown];
    }
    else
    {
        [self stopCountDown];
    }*/
    
    if (nMsgId == ENotifyPause)
    {
        [self startCountDown];
    }
    else
    {
        [self stopCountDown];
    }
    
    return pMsgObject;
}

- (void) backgroundIssueProcess : (TTMsgObject*) aMsg
{
    switch (aMsg.iMsg)
    {
        case ENotifyPause:
            if (!playerPausedManually) {
                [self beginBackgroundTask];
            } else {
                [self endBackgroundTask];
            }
        default:
            break;
    }
}

- (TTInt) playWithUrl : (NSString*) aUrl
{
    NSLogDebug(@"Proxy playWithUrl:%@\n", aUrl);

    playerPausedManually = NO;
    iSavedPos = 0;

    [self beginBackgroundTask];
    [self setPlayBackAudioSession:YES];
    
    TTASSERT(iPlayer != NULL);
    if (iCurUrl != NULL)
    {
        [iCurUrl release];
        iCurUrl = NULL;
    }
    
    
    iCurUrl = [[NSString alloc] initWithString : aUrl];
    iUrlUpdated = false;
    iPlayer->SetDataSourceAsync([iCurUrl UTF8String]);
    

    return TTKErrNone;
}

- (TTInt) setDataSource : (NSString*) aUrl
{
    TTASSERT((iCurUrl != NULL) && (iPlayer != NULL));
    if (iPlayer->GetPlayStatus() == EStatusStoped)
    {
        iUrlUpdated = false;
        TTInt nErr = iPlayer->SetDataSourceAsync([aUrl UTF8String]);
        TTASSERT(nErr == TTKErrNone);
        
        return TTKErrNone;
    }
    
    return TTKErrInUse;
}

- (TTInt) stop
{
    NSLogDebug(@"Proxy stop");
    TTASSERT(iPlayer != NULL);
    return iPlayer->Stop();
}

- (void)stopPlayerWhenNoMusic
{
    playerPausedManually = NO;
    
    [self endBackgroundTask];
}

+ (void) activateAudioDevicesWhenAppLaunchFinished
{
    TTBackgroundAudioQueueConfig::EnableBackground(YES);
}

- (void) beginInterruption
{
    [[AVAudioSession sharedInstance] setActive:NO error:nil];

    if (!callingBegin && iPlayer->GetPlayStatus() == EStatusPlaying){
        NSLogDebug(@"player is running when interruption coming");
        
        [self pause];
        interruptedWhilePlaying = ETTTrue;
    } else {
        interruptedWhilePlaying = ETTFalse;
    }
    
    NSLogDebug(@"beginInterruption %d",interruptedWhilePlaying);
}


- (void) endInterruptionWithFlags:(NSUInteger)flags
{
    NSLogDebug(@"endInterruptionWithFlags:%d %d", flags, interruptedWhilePlaying);
    if(interruptedWhilePlaying && (flags & kAudioSessionInterruptionType_ShouldResume))
    {
        [self playWhenInterruptionFinished];
    }else if (interruptedWhilePlaying || playerStatusBeforeCalling == EStatusPlaying) {
        playerStatusBeforeCalling = EStatusPaused;
        callingBegin = NO;
        
        [self playWhenInterruptionFinished];
    }
    
    gRouteChangePaused = ETTFalse;
    interruptedWhilePlaying = false;
}

- (void)playWhenInterruptionFinished
{
    NSError* error = nil;
    [[AVAudioSession sharedInstance] setActive:YES error:&error];
    if(error)
    {
        NSLogDebug(@"Error : Can't reactivate audio session,%@", error);
    }
    else
    {
        [self resumeAndSetMixOthers];
    }
}

- (void) endBackgroundTask
{
    UIApplication* application = [UIApplication sharedApplication];
    if(UIBackgroundTaskInvalid != backgroundTaskId)
    {
        NSLog(@"BackgroundTask end");
        [application endBackgroundTask:backgroundTaskId];
        backgroundTaskId = UIBackgroundTaskInvalid;
    }
}

- (void) beginBackgroundTask
{
    UIApplication* application = [UIApplication sharedApplication];
    if(UIBackgroundTaskInvalid == backgroundTaskId)
    {
        NSLog(@"BackgroundTask start");
        backgroundTaskId = [application beginBackgroundTaskWithExpirationHandler:^{
            NSLog(@"BackgroundTask time out");
            
            [application endBackgroundTask:backgroundTaskId];
            backgroundTaskId = UIBackgroundTaskInvalid;
            
            [self beginBackgroundTask];
        }];
    }
}

- (void) onTimer
{
    NSLogDebug(@"onTimer");
    TTASSERT(iPlayer != NULL);
    if (iPlayer->GetPlayStatus() == EStatusPaused)
    {
        iAssetReaderFailOrLongPaused = true;   
    }
}

- (void) startCountDown
{
    [self stopCountDown];    
    TTASSERT(iPowerDownCountDownTimer == NULL);
    iPowerDownCountDownTimer = [NSTimer scheduledTimerWithTimeInterval:(100) target:self selector:@selector(onTimer) userInfo:nil repeats:NO];
    [iPowerDownCountDownTimer retain];
    
    NSLogDebug(@"CountDown started!");
}

- (void) stopCountDown
{
    if (iPowerDownCountDownTimer != NULL)
    {        
        [iPowerDownCountDownTimer invalidate];
        [iPowerDownCountDownTimer release];
        iPowerDownCountDownTimer = NULL;
        
        NSLogDebug(@"CountDown stopped!");
    }    
}

- (TTInt) ConfigGeminiWithUrl : (NSString*) aUrl
{
    NSLogDebug(@"ConfigGeminiWithUrl:%@!", aUrl);
    TTInt nErr = TTKErrNone;
    if (gIsIOS4X && aUrl != NULL)
    {        
        if (![iGeminiThreadHandle isExecuting])
        {
            if (gGeminiLeft.IsEnable())
            {
                nErr = gGeminiRight.EnableBackground([aUrl UTF8String], ETTTrue);
                gGeminiRight.EnableBackground(NULL, ETTFalse);
            }
            else
            {
                nErr = gGeminiLeft.EnableBackground([aUrl UTF8String], ETTTrue);
                
                if ((!gGeminiRight.IsEnable()) && nErr == TTKErrNone)
                {
                    TTASSERT(iGeminiUrl == NULL);
                    iGeminiUrl = [aUrl retain];
                    [iGeminiThreadHandle start];
                }
                else
                {
                    gGeminiLeft.EnableBackground(NULL, ETTFalse);                
                }
            }
        }
        else
        {   
            [iCritical lock];
            TTASSERT(iGeminiUrl != NULL);
            [iGeminiUrl release];
            iGeminiUrl = [aUrl retain];
            [iCritical unlock];
        } 
    }
        
    return nErr;
}


- (void) geminiProcL : (NSObject*) aObj
{
    NSAutoreleasePool *pool =[[NSAutoreleasePool alloc] init];
    
    while (ETTTrue)
    {
        NSTimeInterval tInterval = 60;
        if (iGeminiDone)
        {            
            gGeminiLeft.EnableBackground(NULL, ETTFalse);
            gGeminiRight.EnableBackground(NULL, ETTFalse);          
            
            break;
        }
        else
        {
            TTInt nErr = TTKErrNone;
            if (gGeminiLeft.IsEnable())
            {
                [iCritical lock];
                nErr = gGeminiRight.EnableBackground([iGeminiUrl UTF8String], ETTTrue);
                [iCritical unlock];
                gGeminiLeft.EnableBackground(NULL, ETTFalse);                
                NSLogDebug(@"switch to Right:%d\n", nErr);
            }    
            else
            {
                [iCritical lock];
                nErr = gGeminiLeft.EnableBackground([iGeminiUrl UTF8String], ETTTrue);
                [iCritical unlock];
                gGeminiRight.EnableBackground(NULL, ETTFalse);
                NSLogDebug(@"switch to Left:%d\n", nErr);
            }            
            
            if (nErr != TTKErrNone) 
            {
                tInterval = 0.5;
            }
        }
        
        [NSThread sleepForTimeInterval:tInterval];
    }
    
    [pool release];
}

- (TTInt) BufferedPercent
{
    if (iPlayer != NULL) 
    {
        return iPlayer->BufferedPercent();
    }
    return TTKErrNotReady;
}

- (TTUint) fileSize
{
    if (iPlayer != NULL) {
        return iPlayer->fileSize();
    }
    return TTKErrNotReady;
}

- (TTUint) bufferedFileSize
{
    if (iPlayer != NULL) {
        return iPlayer->bufferedFileSize();
    }
    return TTKErrNotReady;
}

- (void) SetActiveNetWorkType : (TTActiveNetWorkType) aType
{
    TTASSERT(iPlayer != NULL);
    iPlayer->SetActiveNetWorkType(aType);
}

- (void) SetCacheFilePath: (NSString *) path
{
    TTASSERT(iPlayer != NULL && path != NULL);
    iPlayer->SetCacheFilePath([path UTF8String]);
}

- (void) setBalanceChannel:(float)aVolume
{
    iPlayer->SetBalanceChannel(aVolume);
}

- (void) SetView : (UIView*) aView
{
    iPlayer->SetView(aView);
}

- (void) SetEffectBackgroundHandle :(bool)aBackground
{
    if (aBackground) {
        gAudioEffectLowDelay = 0;
    }
    else
        gAudioEffectLowDelay = 1;
}

- (void) VideoBackgroundHandle
{
    if (iPlayer) {
        iPlayer->Pause();
        if (iPlayer->GetPlayStatus() != EStatusStoped) {
            iSavedPos = iPlayer->GetPosition();
        }

        iPlayer->SetView(NULL);
    }
}

- (void) VideoForegroundHandle : (UIView*) aView
{
    if (iPlayer && iPlayer->GetPlayStatus() != EStatusStoped) {
        iPlayer->SetView(aView);
        iPlayer->SetPosition(iSavedPos, 1);
    }
}

- (void) SetRotate
{
    if (iPlayer){
        if(EStatusPaused == iPlayer->GetPlayStatus())
            iPlayer->SetRotate();
    }
}

@end
