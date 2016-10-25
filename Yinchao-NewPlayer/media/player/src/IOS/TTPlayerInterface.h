//
//  TTPlayerInterface.h
//  Player
//
//  Created by chaoyang.zhang on 14-1-13.
//
//

#include "TTTypedef.h"

#import <Foundation/Foundation.h>

enum TTPlayStatus;
enum TTNotifyMsg;
enum TTActiveNetWorkType;

@protocol TTPlayerInterface <NSObject>

@required

- (void) pause;
- (void) resume;
- (void) resumeAndSetMixOthers;
- (void) inactiveAudioSession;

- (void) setPosition : (CMTime) aTime;

- (void) setPlayRangeWithStartTime : (CMTime) aStartTime EndTime : (CMTime) aEndTime;

- (CMTime) getPosition;

- (CMTime) duration;
- (TTInt) getCurFreqAndWaveWithFreqBuffer : (TTInt16*) aFreqBuffer andWaveBuffer : (TTInt16*) aWaveBuffer andSamplenum :(TTInt) aSampleNum;
- (TTPlayStatus) getPlayStatus;

- (TTInt) playWithUrl : (NSString*) aUrl;
- (TTInt) stop;
- (TTInt) start;

- (void) ProcessNotifyEventWithMsg : (TTNotifyMsg) aMsg andError : (TTInt) aError;
- (TTInt) ConfigGeminiWithUrl : (NSString*) aUrl;
- (void) SetActiveNetWorkType : (TTActiveNetWorkType) aType;
- (void) SetCacheFilePath: (NSString *) path;

- (void) stopPlayerWhenNoMusic;//when no song plays, stop player
+ (void) activateAudioDevicesWhenAppLaunchFinished;//in order to avoid init audio unit failed, first activate audio devices

- (TTInt) BufferedPercent;
- (TTUint) fileSize;
- (TTUint) bufferedFileSize;

- (void) SetView : (UIView*) aView;
- (void) SetEffectBackgroundHandle :(bool)aBackground;
- (void) VideoBackgroundHandle;
- (void) VideoForegroundHandle : (UIView*) aView;
- (TTInt) BandWidth ;
- (void) SetRotate ;

@optional

- (void) setBalanceChannel:(float)aVolume;
@end
