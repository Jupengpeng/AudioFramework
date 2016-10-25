/**
 * File : TTUIDeviceHardware.h 
 * Created on : 2011-9-7
 * Author : hu.cao
 * Copyright : Copyright (c) 2011 Shuidushi Software Ltd. All rights reserved.
 * Description : TTUIDeviceHardware∂®“ÂŒƒº˛
 */

#ifndef __TT_UI_DEVICE_HARDWARE_H__
#define __TT_UI_DEVICE_HARDWARE_H__
#import <Foundation/Foundation.h>
@interface TTUIDeviceHardware : NSObject {
@private
    NSString*   iPlatformString;
    Boolean     iIOS4_X;
}
+ (TTUIDeviceHardware*) instance;
- (void) systemInfo;
- (NSString*) platform;
- (Boolean)   IsSystemVersion4X;
- (Boolean)   IsDevice3GS;
- (BOOL) isSystemVersionLargeEqual6;
- (BOOL) isSystemVersion5X;
- (BOOL) isSystemVersionLargeEqual7;
@end
#endif
