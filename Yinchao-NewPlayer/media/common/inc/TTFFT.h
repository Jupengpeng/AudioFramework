/* Copyright (c) 2004, Nokia. All rights reserved */


#ifndef __TT_FFT_H__
#define __TT_FFT_H__

// INCLUDES
#include <string.h>
#include "TTTypedef.h"

static const TTInt KMinWaveSample = 256;
static const TTInt KMaxWaveSample = 1024;


class TTFFT
{
public:
	static TTInt16 fix_mpy(TTInt16 a, TTInt16 b);
	static void window(TTInt16 fr[], TTInt32 n);
	static TTInt32 fix_fft(TTInt16 fr[], TTInt16 fi[], TTInt32 m, TTInt32 inverse);
	static void WaveformToFreqBin(TTInt16* aFreq, const TTInt16 *aWave, TTInt aChannel, TTInt aWaveSamples);
};


#endif // __TT_FFT_H__

// End of File
