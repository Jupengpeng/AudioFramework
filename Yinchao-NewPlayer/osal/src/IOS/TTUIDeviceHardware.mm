/**
 * File : TTUIDeviceHardware.mm
 * Created on : 2011-9-7
 * Author : hu.cao
 * Copyright : Copyright (c) 2010 Shuidushi Software Ltd. All rights reserved.
 * Description : TTUIDeviceHardware  µœ÷Œƒº˛
 */

#import "TTUIDeviceHardware.h"
//#import <UIKit/UIApplication.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include "TTLog.h"
#import <UIKit/UIKit.h>
static TTUIDeviceHardware* gUIDeviceHardware = NULL; 

static const NSString* kIPHONE_1_G = @"iPhone1,1";
static const NSString* kIPHONE_3_G = @"iPhone1,2";
static const NSString* kIPHONE_3_GS = @"iPhone2,1";
static const NSString* kIPHONE_4 = @"iPhone3,1";

static const NSString* kIPOD_1G = @"iPod1,1";
static const NSString* kIPOD_2G = @"iPod2,1";
static const NSString* kIPOD_3G = @"iPod3,1";
static const NSString* kIPOD_4G = @"iPod4,1";

static const NSString* kIPAD = @"iPad1,1";

static NSString *systemVersion5X = @"5";

@implementation TTUIDeviceHardware

+ (TTUIDeviceHardware*) instance
{
    if (gUIDeviceHardware == NULL)
    {
        gUIDeviceHardware = [[TTUIDeviceHardware alloc] init];
    }
    
    return gUIDeviceHardware;
}

- (id) init
{
    [self systemInfo];
    
    iPlatformString = nil;
    iPlatformString = [[self platform] retain];
    return self;
}

- (void)dealloc{
    [iPlatformString release];
    iPlatformString = nil;
    
    [super dealloc];
}

- (void) systemInfo
{  
    NSLogDebug(@"system name:%@", [[UIDevice currentDevice] systemName]);
    NSLogDebug(@"system version:%@", [[UIDevice currentDevice] systemVersion]);
    NSLogDebug(@"system model:%@", [[UIDevice currentDevice] model]);
    NSLogDebug(@"system localModel:%@", [[UIDevice currentDevice] localizedModel]);
    
    if (NSClassFromString(@"ACAccount"))
    {
        iIOS4_X = false;
        NSLogDebug(@"ACAccount exists");
    }
    else
    {
        iIOS4_X = true;
        NSLogDebug(@"ACAccount not exists");
    }
}

- (NSString*) platform
{
    size_t size;
    sysctlbyname("hw.machine", NULL, &size, NULL, 0);
    char* machine = (char*)malloc(size);
    sysctlbyname("hw.machine", machine, &size, NULL, 0);
    NSString* platform = [NSString stringWithCString:machine encoding:NSUTF8StringEncoding];
    free(machine);
    
    NSLogDebug(@"platform:%@", platform);
    
    return platform;
}

- (Boolean) IsSystemVersion4X
{
    return iIOS4_X;
}

- (BOOL) isSystemVersionLargeEqual6{
    return [[[UIDevice currentDevice] systemVersion] integerValue] >= 6;
}

- (BOOL) isSystemVersion5X{
    return [[[UIDevice currentDevice] systemVersion] hasPrefix:systemVersion5X];
}

- (BOOL) isSystemVersionLargeEqual7 {
    return [[[UIDevice currentDevice] systemVersion] integerValue] >= 7;
}

- (Boolean) IsDevice3GS
{
    return [kIPHONE_3_GS isEqualToString:iPlatformString];
}

@end
