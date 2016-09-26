/**
 * File : TTBackgroundConfig.h
 * Created on : 2011-9-7
 * Author : hu.cao
 * Copyright : Copyright (c) 2010 Shuidushi Software Ltd. All rights reserved.
 * Description : CTTBackgroundConfig declare˛
 */ 
#ifndef __ST_BACKGROUND_CONFIG_H__
#define __ST_BACKGROUND_CONFIG_H__
#include "STMacrodef.h"
#include "STCritical.h"
#include <AudioToolbox/AudioQueue.h>
static STInt const KAudioQueueBufferNum = 3;
static STInt const KAudioQueueBufferSize = 40 * KILO;

class STBackgroundAssetReaderConfig
{
public:
    STBackgroundAssetReaderConfig();
    STInt EnableBackground(const STChar* aPodUrl, STBool aEnable);
    STBool IsEnable();

private:
    STBool        iBackgroundEnable;
    void*         iAsset;
    void*         iAssetReader;
    void*         iAssetReaderOutput;
}; 

class STBackgroundAudioQueueConfig
{
public:
    static STInt EnableBackground(STBool aEnable);
    
private:
    static STInt StartAudioQueue();
    static void AudioQueueCallback(void *aUserData, AudioQueueRef aAudioQueueRef, AudioQueueBufferRef aAudioQueueBufferRef);
private:
    static STBool                   iBackgroundEnable;
    static AudioQueueRef            iAudioQueue;
    static AudioQueueBufferRef      iAudioBuffer[KAudioQueueBufferNum];
};
#endif