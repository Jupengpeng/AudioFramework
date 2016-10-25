/**
 * File : TTPackagePathFetcher.mm
 * Created on : 2012-3-21
 * Author : hu.cao
 * Copyright : Copyright (c) 2010 Shuidushi Software Ltd. All rights reserved.
 * Description : TTPackagePathFetcher  µœ÷Œƒº˛
 */

#import "TTPackagePathFetcher.h"
#import <UIKit/UIApplication.h>
#import <Foundation/NSFileManager.h>
#include <sys/types.h>
#include <sys/sysctl.h>
static TTChar* gCacheFilePath = NULL;

void gSetCacheFilePath(const TTChar* aCacheFilePath)
{
	gCacheFilePath = (TTChar*)aCacheFilePath;
}

const TTChar* gGetCacheFilePath()
{
    if (gCacheFilePath == NULL)
    {
        NSString* tempPath = [NSString stringWithFormat:@"%@%@", NSTemporaryDirectory(), @"cache.tmp"];
        gSetCacheFilePath([tempPath UTF8String]);
    }
    return gCacheFilePath;
}

const bool gGetCacheFileEnble()
{
    return true;
}

@implementation TTPackagePathFetcher

+ (const TTChar*) DocumentsPath
{
    NSArray* pPaths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDirectory, YES);
    NSString* pDocuments = [pPaths objectAtIndex:0];
    return [pDocuments UTF8String];
}

+ (const TTChar*) CacheFilePath
{
    NSString* pTmp = NSTemporaryDirectory();

    return [pTmp UTF8String];
}


@end
