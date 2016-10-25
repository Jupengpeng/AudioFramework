#ifndef __ST_FFT_H__
#define __ST_FFT_H__

#include "STTypedef.h"
#include <string.h>

class STFFT
{
public:
	static STInt16 fix_mpy(STInt16 a, STInt16 b);
	static void window(STInt16 fr[], STInt32 n);
	static STInt32 fix_fft(STInt16 fr[], STInt16 fi[], STInt32 m, STInt32 inverse);
	static void DoFFT(STInt16* aFreq, const STInt16 *aWave, STInt aChannel, STInt aSampleNum);
};
#endif