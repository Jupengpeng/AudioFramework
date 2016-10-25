/**
 * File : TTPackagePathFetcher.h 
 * Created on : 2012-3-21
 * Author : hu.cao
 * Copyright : Copyright (c) 2011 Shuidushi Software Ltd. All rights reserved.
 * Description : TTPackagePathFetcher∂®“ÂŒƒº˛
 */

#ifndef __TT_PACKAGE_PATH_FETCHER_H__
#define __TT_PACKAGE_PATH_FETCHER_H__
#import <Foundation/Foundation.h>
#include "TTMacrodef.h"
#include "TTTypedef.h"
@interface TTPackagePathFetcher : NSObject {
@private
}
+ (const TTChar*) DocumentsPath;
+ (const TTChar*) CacheFilePath;
@end
#endif
