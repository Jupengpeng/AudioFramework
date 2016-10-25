/**
 * File : TTAutoReleasePool.mm
 * Created on : 2011-9-7
 * Author : hu.cao
 * Copyright : Copyright (c) 2010 Shuidushi Software Ltd. All rights reserved.
 * Description : TTAutoReleasePool  µœ÷Œƒº˛
 */

#include <Foundation/NSAutoreleasePool.h>
#include "TTAutoReleasePool.h"
#include "TTMacrodef.h"
#include "TTCritical.h"

static NSAutoreleasePool* gAutoReleasePool = NULL;

void TTAutoReleasePool::InitAutoRelesePool()
{
    TTASSERT(gAutoReleasePool == NULL);
    gAutoReleasePool = [[NSAutoreleasePool alloc] init];
}

void TTAutoReleasePool::UninitAutoReleasePool()
{
    TTASSERT(gAutoReleasePool != NULL);
    [gAutoReleasePool release];
    gAutoReleasePool = NULL;
}
