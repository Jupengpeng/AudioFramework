/**
* File : TTMediaPlayerProxy.h 
* Created on : 2011-9-1
* Author : hu.cao
* Copyright : Copyright (c) 2011 Shuidushi Software Ltd. All rights reserved.
* Description : CTTMediaPlayerWarp 定义文件
*/
#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import <UIKit/UIApplication.h>

#import "TTMediaPlayerWarp.h"
#import "TTPlayerInterface.h"

extern NSString *NewCallingNotification;
extern NSString *kIsCalling;

@interface TTMsgObject : NSObject {

}
@property (readonly) TTNotifyMsg iMsg;
@property (readonly) TTInt       iError;
- (id) initwithMsg: (TTNotifyMsg) aMsg andError: (TTInt) aError;
@end

@interface TTMediaPlayerProxy : NSObject <AVAudioSessionDelegate, TTPlayerInterface> {

@private
    CTTMediaPlayerWarp*     iPlayer;    
    UIBackgroundTaskIdentifier  backgroundTaskId;
    Boolean                 iUrlUpdated;
    NSString*               iCurUrl;
    Boolean                 iAssetReaderFailOrLongPaused;
    NSTimer*                iPowerDownCountDownTimer;
    NSString*               iGeminiUrl;
    Boolean                 iGeminiDone;
    NSThread*               iGeminiThreadHandle;
    NSLock*                  iCritical;
    TTUint                     iSavedPos;
    
    BOOL                    playerPausedManually;
}

@property (readonly) Boolean       interruptedWhilePlaying;

@end
