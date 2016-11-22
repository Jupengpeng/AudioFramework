//
//  NSString+NSMD5Addition.m
//  NestSound
//
//  Created by 谢豪杰 on 16/6/2.
//  Copyright © 2016年 yinchao. All rights reserved.
//

#import "NSString+NSMD5Addition.h"
#import <CommonCrypto/CommonDigest.h>
@implementation NSString (NSMD5Addition)

-(NSString *)stringToMD5
{
    
    if(self == nil || [self length] == 0)
        return nil;
    
    const char *value = [self UTF8String];
    
    unsigned char outputBuffer[CC_MD5_DIGEST_LENGTH];
    CC_MD5(value, strlen(value), outputBuffer);
    
    NSMutableString *outputString = [[NSMutableString alloc] initWithCapacity:CC_MD5_DIGEST_LENGTH * 2];
    for(NSInteger count = 0; count < CC_MD5_DIGEST_LENGTH; count++){
        [outputString appendFormat:@"%02x",outputBuffer[count]];
    }
    
    return outputString;
}

@end
