#include "STSysTime.h"
#import "STMediaPlayerProxy.h"
#import <AVFoundation/AVFoundation.h>
#import <AudioToolbox/AudioToolbox.h>
#import <UIKit/UIApplication.h>


static const STInt64 KMax_PodReaderRestart_Interval = 180000000;//单位微秒，60s 

@implementation STMsgObject

@synthesize iMsg;
@synthesize iError;

- (id) initwithMsg : (STNotifyMsgId) aMsg andError: (STInt) aError
{
    self = [super init];
    if(self)
    {
        iMsg = aMsg;
        iError = aError;
    }
    
    return self;
}

@end

@interface STMediaPlayerProxy()
- (void) notifyProcessProcL : (id) aMsgObject;
- (void) setupAudioSession;
- (void) processPodReaderRestart;
- (void) extendBackgroundTask;
@end

@implementation STMediaPlayerProxy

@synthesize interruptedWhilePlaying;

- (id) init
{   
    self = [super init];
    if(self)
    {
        [self setupAudioSession];
        
        iPlayer = new STMediaPlayerWarp((void*)self);
        
        interruptedWhilePlaying = false;   
        
        iCurUrl = NULL;
        
        iPrePodReaderAccessTime = GetTimeOfDay();
        
        iBackgroundTaskId = UIBackgroundTaskInvalid;
        
        iBusy = NO;
    }
    
    return self;
}

- (void)dealloc
{  
    [super dealloc];
    [iCurUrl release];
    SAFE_DELETE(iPlayer);
}

//void audioRouteChangeListenerCallback(void *inClientData,AudioSessionPropertyID	inID,UInt32 inDataSize,const void *inData)
//{
//    NSLog(@"audioRouteChangeListenerCallback");
//    
//    STMediaPlayerProxy* player = (STMediaPlayerProxy*)inClientData;
//    if (inID == kAudioSessionProperty_AudioRouteChange)
//    {
//        CFDictionaryRef routeDict = (CFDictionaryRef)inData;
//        NSNumber* reasonValue = (NSNumber*)CFDictionaryGetValue(routeDict, CFSTR(kAudioSession_AudioRouteChangeKey_Reason));
//        
//        int reason = [reasonValue intValue];
//        NSLog(@"Audio Route Change Reason,%d", reason);
//        if (reason == kAudioSessionRouteChangeReason_OldDeviceUnavailable)
//        {
//            gSlientImmediately = ESTTrue;
//            [player pause];
//            gRouteChangePaused = ESTTrue;
//        }
//        else if(reason == kAudioSessionRouteChangeReason_NewDeviceAvailable)
//        {
//            gSlientImmediately = ESTFalse;
//            gRouteChangePaused = ESTFalse;
//        }
//        
//        //        else if(reason == kAudioSessionRouteChangeReason_NewDeviceAvailable)
//        //        {
//        //            if(MusicPlayerStatusPlaying == musicPlayer.playbackStatus)
//        //            {
//        //                NSLogDebug(@"musicPlayer,mark 2");
//        //                [musicPlayer play];
//        //            }
//        //        }
//    }
    
//}

- (void) setupAudioSession
{
    NSError *sessionError = nil;
    [[AVAudioSession sharedInstance] setCategory:AVAudioSessionCategoryPlayback error:&sessionError];
    
    if(sessionError)
    {
        NSLog(@"%@", sessionError);
    }
    else
    {   sessionError = nil;
        [[AVAudioSession sharedInstance] setActive:YES withFlags:kAudioSessionSetActiveFlag_NotifyOthersOnDeactivation error:&sessionError];
        if(sessionError)
        {
            NSLog(@"%@", sessionError);
        }      
        
//        AudioSessionAddPropertyListener(kAudioSessionProperty_AudioRouteChange, audioRouteChangeListenerCallback, self);
    }   
    
    [[AVAudioSession sharedInstance] setDelegate:self];    
    [[AVAudioSession sharedInstance] setActive:YES error:&sessionError];
}

- (STInt) start
{
    STASSERT(iPlayer != NULL);
    [self processPodReaderRestart];
    return iPlayer->Play();
}

- (void) pause
{
    STASSERT(iPlayer != NULL);
    iPlayer->Pause();
}

- (void) extendBackgroundTask
{
    UIApplication* application = [UIApplication sharedApplication];
    if(UIBackgroundTaskInvalid != iBackgroundTaskId)
    {
        [application endBackgroundTask:iBackgroundTaskId];
        iBackgroundTaskId = UIBackgroundTaskInvalid;
    }
    iBackgroundTaskId = [application beginBackgroundTaskWithExpirationHandler:^{
        [application endBackgroundTask:iBackgroundTaskId];
        iBackgroundTaskId = UIBackgroundTaskInvalid;
    }];
}

- (void) resume
{
    STASSERT(iPlayer != NULL);
    [self processPodReaderRestart];
    iPlayer->Resume();       
}

- (void) setPosition : (CMTime) aTime
{
    STASSERT(iPlayer != NULL);
    STUint64 tmp = aTime.value;
    tmp *= 1000;
    STUint32 nMillSecPos = tmp / aTime.timescale;
    iPlayer->SetPosition(nMillSecPos);    
}

- (void) setPlayRangeWithStartTime : (CMTime) aStartTime EndTime : (CMTime) aEndTime
{    
    STASSERT(iPlayer != NULL);
    STUint64 tmp = aStartTime.value;
    tmp *= 1000;
    STUint32 nStartMillSecPos = tmp / aStartTime.timescale;
    
    tmp = aEndTime.value;
    tmp *= 1000;
    STUint32 nEndMillSecPos = tmp / aEndTime.timescale;
    
    iPlayer->SetPlayRange(nStartMillSecPos, nEndMillSecPos);
}

- (CMTime) getPosition
{
    STASSERT(iPlayer != NULL);
    return CMTimeMake(iPlayer->GetPosition(), 1000);
}

- (CMTime) duration
{
    STASSERT(iPlayer != NULL);
    return CMTimeMake(iPlayer->Duration(), 1000);
}

- (STInt) getCurFreqAndWaveWithFreqBuffer : (STInt16*) aFreqBuffer andWaveBuffer : (STInt16*) aWaveBuffer andSamplenum :(STInt) aSampleNum
{
    return iPlayer->GetCurFreqAndWave(aFreqBuffer, aWaveBuffer, aSampleNum);
}

- (STPlayStatus) getPlayStatus
{
    STASSERT(iPlayer != NULL);
    return iPlayer->GetPlayStatus();
}

- (void) ProcessNotifyEventWithMsg : (STNotifyMsgId) aMsg andError: (STInt) aError
{
    STMsgObject* pMsgObject = [[[STMsgObject alloc] initwithMsg:aMsg andError:aError] autorelease];
    
    if (aMsg == ENotifyPrepare || aMsg == ENotifyPause) 
    {
        iPrePodReaderAccessTime = GetTimeOfDay();
        [self extendBackgroundTask];
    }
    
    [self performSelectorOnMainThread:@selector(notifyProcessProcL:) withObject:pMsgObject waitUntilDone:NO];
}

- (void) notifyProcessProcL : (id) aMsgObject;
{
    if (iBusy)
    {
        iBusy = NO;
        
        [self playWithUrl:iCurUrl];
    }
    else
    {
        STMsgObject* pMsg = (STMsgObject *)aMsgObject;
        
        if (pMsg != NULL)
        {
            [[NSNotificationCenter defaultCenter] postNotificationName:@"PlayerNotifyEvent" object:pMsg];
        }
    }
}

- (STInt) playWithUrl : (NSString*) aUrl
{
    STASSERT(iPlayer != NULL);
    
    STInt nErr = iPlayer->SetDataSourceAsync([aUrl UTF8String]);

    [iCurUrl autorelease];
    iCurUrl = [aUrl retain];
    
    iBusy = nErr != STKErrNone;
    
    return nErr;
}

- (NSString*) currentUrl
{
    return iCurUrl;
}

- (STInt) stop
{   
    STASSERT(iPlayer != NULL);
    return iPlayer->Stop();
}

- (void) beginInterruption
{
    NSLog(@"beginInterruption");
    if (iPlayer->GetPlayStatus() == EStatusPlaying)
    {
        interruptedWhilePlaying = ESTTrue;
        [self pause];      
    }
    else
    {
        interruptedWhilePlaying = ESTFalse;
    }
}

- (void) endInterruptionWithFlags:(NSUInteger)flags
{
    NSLog(@"endInterruptionWithFlags:%d", flags);
    if(interruptedWhilePlaying && (flags & kAudioSessionInterruptionType_ShouldResume))
    {        
        NSError* error = nil;
        [[AVAudioSession sharedInstance] setActive:YES error:&error];
        if(error)
        {
            NSLog(@"Error : Can't reactivate audio session,%@", error);
        }
        else
        {
            [self resume]; 
        }
    }
    
    interruptedWhilePlaying = ESTFalse;
}

- (void) processPodReaderRestart
{
    if (GetTimeOfDay() - iPrePodReaderAccessTime > KMax_PodReaderRestart_Interval) 
    {
        NSLog(@"!!!!!!!!!!!!!Pod Reader ReStart!!!!!!!!!!!!!!!");
        iPlayer->SetPosition(iPlayer->GetPosition());
    }
}

- (NSArray *) getEqualizerPresetArray
{
    NSMutableArray* pArray = [[[NSMutableArray alloc] init] autorelease];
    STPointerArray<STChar> tArray;
    iPlayer->GetEqualizerPresetArray(tArray);
    for (int nIdx = 0; nIdx < tArray.Count(); nIdx++) 
    {
        [pArray addObject:[NSString stringWithUTF8String:tArray[nIdx]]];
    }
    
    tArray.ResetAndDestroy();
    tArray.Close();
    
    return pArray;
}

- (void) setEqualizerPresetWithIdx:(NSInteger)idx
{
    iPlayer->SetEqualizerPresetIdx(idx);
}

- (void) enableEffect:(STEffectType) aEffect andParam1:(NSInteger)aParam1 andParam2:(NSInteger)aParam2
{
    iPlayer->EnableEffect(aEffect, aParam1, aParam2);
}

- (void) disableEffect:(STEffectType) aEffect
{
    iPlayer->DisableEffect(aEffect);
}

@end