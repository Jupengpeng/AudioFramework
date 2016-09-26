#ifndef __ST_AUTO_RELEASE_POOL_H__
#define __ST_AUTO_RELEASE_POOL_H__

class STAutoReleasePool
{
public:
    static void* InitAutoRelesePool();
    static void UninitAutoReleasePool(void* aAutoreleasePool);   
};

#endif
