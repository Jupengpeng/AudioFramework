
#include <Foundation/NSAutoreleasePool.h>
#include "STAutoReleasePool.h"
#include "STMacrodef.h"
#include "STCritical.h"

void* STAutoReleasePool::InitAutoRelesePool()
{
    return (void*)([[NSAutoreleasePool alloc] init]);
}

void STAutoReleasePool::UninitAutoReleasePool(void* aAutoreleasePool)
{
    STASSERT(aAutoreleasePool != NULL);
    
    [(NSAutoreleasePool*)aAutoreleasePool release];
}
