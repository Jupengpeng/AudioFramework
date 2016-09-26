#import <Foundation/Foundation.h>
#include "STMediaPlayerWarp.h"
#import <AVFoundation/AVFoundation.h>
#import <UIKit/UIApplication.h>
@interface STMsgObject : NSObject {

}
@property (readonly) STNotifyMsgId iMsg;
@property (readonly) STInt       iError;
- (id) initwithMsg: (STNotifyMsgId) aMsg andError: (STInt) aError;
@end

@interface STMediaPlayerProxy : NSObject <AVAudioSessionDelegate> {

@private
    STMediaPlayerWarp*          iPlayer;    
    NSString*                   iCurUrl;
    STUint64                    iPrePodReaderAccessTime;
    UIBackgroundTaskIdentifier  iBackgroundTaskId;
    STBool                      iBusy;
}

@property (readonly) Boolean       interruptedWhilePlaying;
- (id) init;
- (void) pause;
- (void) resume;

- (void) setPosition : (CMTime) aTime;

- (void) setPlayRangeWithStartTime : (CMTime) aStartTime EndTime : (CMTime) aEndTime;

- (CMTime) getPosition;

- (CMTime) duration;
- (STInt) getCurFreqAndWaveWithFreqBuffer : (STInt16*) aFreqBuffer andWaveBuffer : (STInt16*) aWaveBuffer andSamplenum :(STInt) aSampleNum;
- (STPlayStatus) getPlayStatus;

- (STInt) playWithUrl : (NSString*) aUrl;
- (STInt) stop;
- (STInt) start;

- (NSString*) currentUrl;

- (void) ProcessNotifyEventWithMsg : (STNotifyMsgId) aMsg andError : (STInt) aError;
- (NSArray *) getEqualizerPresetArray;
- (void) setEqualizerPresetWithIdx:(NSInteger)idx;
- (void) enableEffect:(STEffectType) aEffect andParam1:(NSInteger)aParam1 andParam2:(NSInteger)aParam2;
- (void) disableEffect:(STEffectType) aEffect;
@end
