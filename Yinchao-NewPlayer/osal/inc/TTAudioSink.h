/**
* File : TTSink.h
* Created on : 2011-3-16
* Author : hu.cao
* Copyright : Copyright (c) 2011 Shuidushi Software Ltd. All rights reserved.
* Description : Engine����Ҫ�������ļ�����ͬƽ̨ʹ�ò�ͬ��ͷ�ļ�
*/

#ifndef __TT_SINK_H__
#define __TT_SINK_H__

#include "TTOSALConfig.h"

#ifdef __TT_OS_WINDOWS__
#include "Wins/TTAudioSink.h"

#elif (defined __TT_OS_SYMBIAN__)
#include "Symbian/TTAudioSink.h"

#elif (defined __TT_OS_ANDROID__)
#include "Android/TTAudioSink.h"

#elif (defined __TT_OS_IOS__)
#include "IOS/TTAudioSink.h"

#endif

#endif
