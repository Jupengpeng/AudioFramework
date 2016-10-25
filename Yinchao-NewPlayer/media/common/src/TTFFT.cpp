/*      fix_fft.c - Fixed-point Fast Fourier Transform  */
/*
        fix_fft()       perform FFT or inverse FFT
		window()        applies a Hanning window to the (time) input
        fix_loud()      calculates the loudness of the signal, for
                        each freq point. Result is an integer array,
                        units are dB (values will be negative).
        fix_mpy()       perform fixed-point multiplication.
        Sinewave[1024]  sinewave normalized to 32767 (= 1.0).
        Loudampl[100]   Amplitudes for lopudnesses from 0 to -99 dB.


        All data are fixed-point short integers, in which
        -32768 to +32768 represent -1.0 to +1.0. Integer arithmetic
        is used for speed, instead of the more natural floating-point.

        For the forward FFT (time -> freq), fixed scaling is
        performed to prevent arithmetic overflow, and to map a 0dB
        sine/cosine wave (i.e. amplitude = 32767) to two -6dB freq
        coefficients; the one in the lower half is reported as 0dB
        by fix_loud(). The return value is always 0.

        For the inverse FFT (freq -> time), fixed scaling cannot be
        done, as two 0dB coefficients would sum to a peak amplitude of
        64K, overflowing the 32k range of the fixed-point integers.
        Thus, the fix_fft() routine performs variable scaling, and
        returns a value which is the number of bits LEFT by which
        the output must be shifted to get the actual amplitude
        (i.e. if fix_fft() returns 3, each value of fr[] and fi[]
        must be multiplied by 8 (2**3) for proper scaling.
        Clearly, this cannot be done within the fixed-point short
        integers. In practice, if the result is to be used as a
        filter, the scale_shift can usually be ignored, as the
        result will be approximately correctly normalized as is.


        TURBO C, any memory model; uses inline assembly for speed
        and for carefully-scaled arithmetic.
        Written by:  Tom Roberts  11/8/89
        Made portable:  Malcolm Slaney 12/15/94 malcolm@interval.com

                Timing on a Macintosh PowerBook 180.... (using Symantec C6.0)
                        fix_fft (1024 points)             8 ticks
                        fft (1024 points - Using SANE)  112 Ticks
                        fft (1024 points - Using FPU)    11

*/
#include "TTOsalConfig.h"
#ifdef __TT_OS_WINDOWS__
#pragma warning( disable : 4244 ) // conversion from int to short -- possible loss of data
#endif

/* FIX_MPY() - fixed-point multiplication macro.
   This macro is a statement, not an expression (uses asm).
   BEWARE: make sure _DX is not clobbered by evaluating (A) or DEST.
   args are all of type fixed.
   Scaling ensures that 32767*32767 = 32767. */
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include "TTMacrodef.h"
#include "TTTypedef.h"
#include "TTFFT.h"

//TTInt16 gFFTRe[KMaxWaveSample];
//TTInt16 gFFTIm[KMaxWaveSample];

// static const TTUint8 KScale32[32] = {3, 4, 5, 6, 6, 8, 8, 8,
// 									8, 8, 8, 8, 8, 8, 8, 8,
// 									9, 9, 9, 9, 9, 9, 9, 9,
// 									9, 9, 9, 9, 9, 9, 9, 9 };
// static const TTUint8 KScale64[64] = {3, 4, 5, 6, 6, 8, 8, 8,
// 									8, 8, 8, 8, 8, 8, 8, 8,
// 									9, 9, 9, 9, 9, 9, 9, 9,
// 									9, 9, 9, 9, 9, 9, 9, 9,
// 									0, 0, 0, 0, 0, 0, 0, 0,
// 									0, 0, 0, 0, 0, 0, 0, 0,
// 									0, 0, 0, 0, 0, 0, 0, 0,
// 									0, 0, 0, 0, 0, 0, 0, 0};
// 
// static TTUint8 gScale512[512];

#define FIX_MPY(DEST,A,B)       DEST = ((long)(A) * (long)(B))>>15
//static const TTInt KNLoud = 100;
static const TTInt KNWave = 1024;
static const TTInt KLog2NWave = 10;

// static const TTInt16 powValue[KNLoud] = {
// 	0,      4,     14,     30,      54,    82,    118,    162,    210,	  264,     
// 		330,    398,    472,    554,     644,   738,    839,    948,   1052,	 1102,      
// 		1140,	 1192,	 1240,	 1320,    1380,	 1508,	 1648,	 1798,	 1938,	 2100,
// 		2257,   2307,   2364,   2420,	  2480,	 2510,	 2548,	 2728,	 2916,	 3112,
// 		3315,	 3527,	 3746,	 3972,    4208,	 4451,	 4702,	 4862,	 4892,	 4920,
// 		4950,	 5008,	 5056,	 5339,	  5632,	 5935,	 6248,	 6572,	 6907,	 7253,
// 		7610,	 7878,	 7904,	 7998,	  8062,	 8120,	 8194,	 8615,	 9051,	 9511,
// 		9978,	10452,	10960,  11466,	 11902,	11980,	12012,	12068,	12128,	12796,
// 		13421,	14105,	14814,	15500,	 16313,	17104,	17923,	18020,	18134,	18297,
// 		19138,	20447,	21596,	22795,	 24047,	26674,	28197,	29127,	30203,	32767
// };

static const TTInt16 Sinewave[KNWave] = {
	0,    201,    402,    603,    804,   1005,   1206,   1406,
		1607,   1808,   2009,   2209,   2410,   2610,   2811,   3011,
		3211,   3411,   3611,   3811,   4011,   4210,   4409,   4608,
		4807,   5006,   5205,   5403,   5601,   5799,   5997,   6195,
		6392,   6589,   6786,   6982,   7179,   7375,   7571,   7766,
		7961,   8156,   8351,   8545,   8739,   8932,   9126,   9319,
		9511,   9703,   9895,  10087,  10278,  10469,  10659,  10849,
		11038,  11227,  11416,  11604,  11792,  11980,  12166,  12353,
		12539,  12724,  12909,  13094,  13278,  13462,  13645,  13827,
		14009,  14191,  14372,  14552,  14732,  14911,  15090,  15268,
		15446,  15623,  15799,  15975,  16150,  16325,  16499,  16672,
		16845,  17017,  17189,  17360,  17530,  17699,  17868,  18036,
		18204,  18371,  18537,  18702,  18867,  19031,  19194,  19357,
		19519,  19680,  19840,  20000,  20159,  20317,  20474,  20631,
		20787,  20942,  21096,  21249,  21402,  21554,  21705,  21855,
		22004,  22153,  22301,  22448,  22594,  22739,  22883,  23027,
		23169,  23311,  23452,  23592,  23731,  23869,  24006,  24143,
		24278,  24413,  24546,  24679,  24811,  24942,  25072,  25201,
		25329,  25456,  25582,  25707,  25831,  25954,  26077,  26198,
		26318,  26437,  26556,  26673,  26789,  26905,  27019,  27132,
		27244,  27355,  27466,  27575,  27683,  27790,  27896,  28001,
		28105,  28208,  28309,  28410,  28510,  28608,  28706,  28802,
		28897,  28992,  29085,  29177,  29268,  29358,  29446,  29534,
		29621,  29706,  29790,  29873,  29955,  30036,  30116,  30195,
		30272,  30349,  30424,  30498,  30571,  30643,  30713,  30783,
		30851,  30918,  30984,  31049,
		31113,  31175,  31236,  31297,
		31356,  31413,  31470,  31525,  31580,  31633,  31684,  31735,
		31785,  31833,  31880,  31926,  31970,  32014,  32056,  32097,
		32137,  32176,  32213,  32249,  32284,  32318,  32350,  32382,
		32412,  32441,  32468,  32495,  32520,  32544,  32567,  32588,
		32609,  32628,  32646,  32662,  32678,  32692,  32705,  32717,
		32727,  32736,  32744,  32751,  32757,  32761,  32764,  32766,
		32767,  32766,  32764,  32761,  32757,  32751,  32744,  32736,
		32727,  32717,  32705,  32692,  32678,  32662,  32646,  32628,
		32609,  32588,  32567,  32544,  32520,  32495,  32468,  32441,
		32412,  32382,  32350,  32318,  32284,  32249,  32213,  32176,
		32137,  32097,  32056,  32014,  31970,  31926,  31880,  31833,
		31785,  31735,  31684,  31633,  31580,  31525,  31470,  31413,
		31356,  31297,  31236,  31175,  31113,  31049,  30984,  30918,
		30851,  30783,  30713,  30643,  30571,  30498,  30424,  30349,
		30272,  30195,  30116,  30036,  29955,  29873,  29790,  29706,
		29621,  29534,  29446,  29358,  29268,  29177,  29085,  28992,
		28897,  28802,  28706,  28608,  28510,  28410,  28309,  28208,
		28105,  28001,  27896,  27790,  27683,  27575,  27466,  27355,
		27244,  27132,  27019,  26905,  26789,  26673,  26556,  26437,
		26318,  26198,  26077,  25954,  25831,  25707,  25582,  25456,
		25329,  25201,  25072,  24942,  24811,  24679,  24546,  24413,
		24278,  24143,  24006,  23869,  23731,  23592,  23452,  23311,
		23169,  23027,  22883,  22739,  22594,  22448,  22301,  22153,
		22004,  21855,  21705,  21554,  21402,  21249,  21096,  20942,
		20787,  20631,  20474,  20317,  20159,  20000,  19840,  19680,
		19519,  19357,  19194,  19031,  18867,  18702,  18537,  18371,
		18204,  18036,  17868,  17699,  17530,  17360,  17189,  17017,
		16845,  16672,  16499,  16325,  16150,  15975,  15799,  15623,
		15446,  15268,  15090,  14911,  14732,  14552,  14372,  14191,
		14009,  13827,  13645,  13462,  13278,  13094,  12909,  12724,
		12539,  12353,  12166,  11980,  11792,  11604,  11416,  11227,
		11038,  10849,  10659,  10469,  10278,  10087,   9895,   9703,
		9511,   9319,   9126,   8932,   8739,   8545,   8351,   8156,
		7961,   7766,   7571,   7375,   7179,   6982,   6786,   6589,
		6392,   6195,   5997,   5799,   5601,   5403,   5205,   5006,
		4807,   4608,   4409,   4210,   4011,   3811,   3611,   3411,
		3211,   3011,   2811,   2610,   2410,   2209,   2009,   1808,
		1607,   1406,   1206,   1005,    804,    603,    402,    201,
		0,   -201,   -402,   -603,   -804,  -1005,  -1206,  -1406,
		-1607,  -1808,  -2009,  -2209,  -2410,  -2610,  -2811,  -3011,
		-3211,  -3411,  -3611,  -3811,  -4011,  -4210,  -4409,  -4608,
		-4807,  -5006,  -5205,  -5403,  -5601,  -5799,  -5997,  -6195,
		-6392,  -6589,  -6786,  -6982,  -7179,  -7375,  -7571,  -7766,
		-7961,  -8156,  -8351,  -8545,  -8739,  -8932,  -9126,  -9319,
		-9511,  -9703,  -9895, -10087, -10278, -10469, -10659, -10849,
		-11038, -11227, -11416, -11604, -11792, -11980, -12166, -12353,
		-12539, -12724, -12909, -13094, -13278, -13462, -13645, -13827,
		-14009, -14191, -14372, -14552, -14732, -14911, -15090, -15268,
		-15446, -15623, -15799, -15975, -16150, -16325, -16499, -16672,
		-16845, -17017, -17189, -17360, -17530, -17699, -17868, -18036,
		-18204, -18371, -18537, -18702, -18867, -19031, -19194, -19357,
		-19519, -19680, -19840, -20000, -20159, -20317, -20474, -20631,
		-20787, -20942, -21096, -21249, -21402, -21554, -21705, -21855,
		-22004, -22153, -22301, -22448, -22594, -22739, -22883, -23027,
		-23169, -23311, -23452, -23592, -23731, -23869, -24006, -24143,
		-24278, -24413, -24546, -24679, -24811, -24942, -25072, -25201,
		-25329, -25456, -25582, -25707, -25831, -25954, -26077, -26198,
		-26318, -26437, -26556, -26673, -26789, -26905, -27019, -27132,
		-27244, -27355, -27466, -27575, -27683, -27790, -27896, -28001,
		-28105, -28208, -28309, -28410, -28510, -28608, -28706, -28802,
		-28897, -28992, -29085, -29177, -29268, -29358, -29446, -29534,
		-29621, -29706, -29790, -29873, -29955, -30036, -30116, -30195,
		-30272, -30349, -30424, -30498, -30571, -30643, -30713, -30783,
		-30851, -30918, -30984, -31049, -31113, -31175, -31236, -31297,
		-31356, -31413, -31470, -31525, -31580, -31633, -31684, -31735,
		-31785, -31833, -31880, -31926, -31970, -32014, -32056, -32097,
		-32137, -32176, -32213, -32249, -32284, -32318, -32350, -32382,
		-32412, -32441, -32468, -32495, -32520, -32544, -32567, -32588,
		-32609, -32628, -32646, -32662, -32678, -32692, -32705, -32717,
		-32727, -32736, -32744, -32751, -32757, -32761, -32764, -32766,
		-32767, -32766, -32764, -32761, -32757, -32751, -32744, -32736,
		-32727, -32717, -32705, -32692, -32678, -32662, -32646, -32628,
		-32609, -32588, -32567, -32544, -32520, -32495, -32468, -32441,
		-32412, -32382, -32350, -32318, -32284, -32249, -32213, -32176,
		-32137, -32097, -32056, -32014, -31970, -31926, -31880, -31833,
		-31785, -31735, -31684, -31633, -31580, -31525, -31470, -31413,
		-31356, -31297, -31236, -31175, -31113, -31049, -30984, -30918,
		-30851, -30783, -30713, -30643, -30571, -30498, -30424, -30349,
		-30272, -30195, -30116, -30036, -29955, -29873, -29790, -29706,
		-29621, -29534, -29446, -29358, -29268, -29177, -29085, -28992,
		-28897, -28802, -28706, -28608, -28510, -28410, -28309, -28208,
		-28105, -28001, -27896, -27790, -27683, -27575, -27466, -27355,
		-27244, -27132, -27019, -26905, -26789, -26673, -26556, -26437,
		-26318, -26198, -26077, -25954, -25831, -25707, -25582, -25456,
		-25329, -25201, -25072, -24942, -24811, -24679, -24546, -24413,
		-24278, -24143, -24006, -23869, -23731, -23592, -23452, -23311,
		-23169, -23027, -22883, -22739, -22594, -22448, -22301, -22153,
		-22004, -21855, -21705, -21554, -21402, -21249, -21096, -20942,
		-20787, -20631, -20474, -20317, -20159, -20000, -19840, -19680,
		-19519, -19357, -19194, -19031, -18867, -18702, -18537, -18371,
		-18204, -18036, -17868, -17699, -17530, -17360, -17189, -17017,
		-16845, -16672, -16499, -16325, -16150, -15975, -15799, -15623,
		-15446, -15268, -15090, -14911, -14732, -14552, -14372, -14191,
		-14009, -13827, -13645, -13462, -13278, -13094, -12909, -12724,
		-12539, -12353, -12166, -11980, -11792, -11604, -11416, -11227,
		-11038, -10849, -10659, -10469, -10278, -10087,  -9895,  -9703,
		-9511,  -9319,  -9126,  -8932,  -8739,  -8545,  -8351,  -8156,
		-7961,  -7766,  -7571,  -7375,  -7179,  -6982,  -6786,  -6589,
		-6392,  -6195,  -5997,  -5799,  -5601,  -5403,  -5205,  -5006,
		-4807,  -4608,  -4409,  -4210,  -4011,  -3811,  -3611,  -3411,
		-3211,  -3011,  -2811,  -2610,  -2410,  -2209,  -2009,  -1808,
		-1607,  -1406,  -1206,  -1005,   -804,   -603,   -402,   -201,
};

// static const TTInt16 Loudampl[KNWave] = {
// 	32767,  29203,  26027,  23197,  20674,  18426,  16422,  14636,
// 	13044,  11626,  10361,   9234,   8230,   7335,   6537,   5826,
// 	5193,   4628,   4125,   3676,   3276,   2920,   2602,   2319,
// 	2067,   1842,   1642,   1463,   1304,   1162,   1036,    923,
// 	823,    733,    653,    582,    519,    462,    412,    367,
// 	327,    292,    260,    231,    206,    184,    164,    146,
// 	130,    116,    103,     92,     82,     73,     65,     58,
// 	51,     46,     41,     36,     32,     29,     26,     23,
// 	20,     18,     16,     14,     13,     11,     10,      9,
// 	8,      7,      6,      5,      5,      4,      4,      3,
// 	3,      2,      2,      2,      2,      1,      1,      1,
// 	1,      1,      1,      0,      0,      0,      0,      0,
// 	0,      0,      0,      0,
// };

// static const TTInt32 loud2[KNLoud] = {
// 	1073676289, 852815209, 677404729, 538100809, 427414276, 339517476, 269682084, 214212496, 
// 	170145936, 135163876, 107350321,  85266756,  67732900,  53802225,  42732369,  33942276, 
// 	26967249,  21418384,  17015625,  13512976,  10732176,   8526400,   6770404,   5377761, 
// 	4272489,   3392964,   2696164,   2140369,   1700416,   1350244,   1073296,    851929, 
// 	677329,    537289,    426409,    338724,    269361,    213444,    169744,    134689, 
// 	106929,     85264,     67600,     53361,     42436,     33856,     26896,     21316, 
// 	16900,     13456,     10609,      8464,      6724,      5329,      4225,      3364,
// 	2601,      2116,      1681,      1296,      1024,       841,       676,       529, 
// 	400,       324,       256,       196,       169,       121,       100,        81, 
// 	64,        49,        36,        25,        25,        16,        16,         9, 
// 	9,         4,         4,         4,         4,         1,         1,         1, 
// 	1,         1,         1,         0,         0,         0,         0,         0, 
// 	0,         0,         0,         0 
// };

/*
        fix_fft() - perform fast Fourier transform.

        if n>0 FFT is done, if n<0 inverse FFT is done
        fr[n],fi[n] are real,imaginary arrays, INPUT AND RESULT.
        size of data = 2**m
        set inverse to 0=dft, 1=idft
*/
TTInt32 TTFFT::fix_fft(TTInt16 fr[], TTInt16 fi[], TTInt32 m, TTInt32 inverse)
{
	int mr, nn, i, j, l, k, n, scale, shift;
	TTInt16 qr, qi, tr, ti, wr, wi;

	n = 1 << m;

	/* max FFT size = N_WAVE */
	if(n > KNWave)
		return -1;

	mr = 0;
	nn = n - 1;
	scale = 0;

	/* decimation in time - re-order data */
	for(m = 1; m <= nn; ++m) {
		l = n;
		do {
			l >>= 1;
		} while(mr + l > nn);
		mr = (mr & (l - 1)) + l;

		if(mr <= m) continue;
		tr = fr[m];
		fr[m] = fr[mr];
		fr[mr] = tr;
		ti = fi[m];
		fi[m] = fi[mr];
		fi[mr] = ti;
	}

	l = 1;
	k = KLog2NWave - 1;
	while(l < n) {
		if(inverse) {
			/* variable scaling, depending upon data */
			shift = 0;
			for(i = 0; i < n; ++i) {
				j = fr[i];
				if(j < 0)
					j = -j;
				m = fi[i];
				if(m < 0)
					m = -m;
				if(j > 16383 || m > 16383) {
					shift = 1;
					break;
				}
			}
			if(shift)
				++scale;
		} else {
			/* fixed scaling, for proper normalization -
			there will be log2(n) passes, so this
			results in an overall factor of 1/n,
			distributed to maximize arithmetic accuracy. */
			shift = 1;
		}
		/* it may not be obvious, but the shift will be performed
		on each data point exactly once, during this pass. */
		int istep = l << 1;
		for(m = 0; m < l; ++m) {
			j = m << k;
			/* 0 <= j < N_WAVE/2 */
			wr =  Sinewave[j + KNWave/4];
			wi = -Sinewave[j];
			if(inverse)
				wi = -wi;
			if(shift) {
				wr >>= 1;
				wi >>= 1;
			}
			for(i = m; i < n; i += istep) {
				j = i + l;
				tr = fix_mpy(wr, fr[j]) -
					fix_mpy(wi, fi[j]);
				ti = fix_mpy(wr, fi[j]) +
					fix_mpy(wi, fr[j]);
				qr = fr[i];
				qi = fi[i];
				if(shift) {
					qr >>= 1;
					qi >>= 1;
				}
				fr[j] = qr - tr;
				fi[j] = qi - ti;
				fr[i] = qr + tr;
				fi[i] = qi + ti;
			}
		}
		--k;
		l = istep;
	}

	return scale;
}


/*      window() - apply a Hanning window       */
void TTFFT::window(TTInt16 fr[], TTInt32 n)
{
	int i, j, k;

	j = KNWave / n;
	n >>= 1;
	for(i = 0,k = KNWave / 4; i < n; ++i, k += j)
		FIX_MPY(fr[i], fr[i], 16384-(Sinewave[k] >> 1));
	n <<= 1;
	for(k -= j; i<n; ++i, k -= j)
		FIX_MPY(fr[i], fr[i], 16384 - (Sinewave[k] >> 1));
}

/*
        fix_mpy() - fixed-point multiplication
*/
TTInt16 TTFFT::fix_mpy(TTInt16 a, TTInt16 b)
{
	FIX_MPY(a, a, b);
	return a;
}

void TTFFT::WaveformToFreqBin(TTInt16* aFreq, const TTInt16 *aWave, TTInt aChannel, TTInt aWaveSamples)
{
	static TTInt16 sFFTRe[KMaxWaveSample];
	static TTInt16 sFFTIm[KMaxWaveSample];

	const TTInt K_FFT_SIZE = aWaveSamples;
	const TTInt K_FFT_SHIFT = 9;
	TTInt i;
	TTInt16 *pFFTRe = sFFTRe;
	TTInt16 *pFFTIm = sFFTIm;
	memset(pFFTIm, 0, K_FFT_SIZE << 1);//��ʼ���鲿

	if(2 == aChannel)//����ʵ��
	{       
		for (i = 0; i < K_FFT_SIZE; i++) 
		{
			*pFFTRe++ = (*aWave + *(++aWave)) >> 1;
			aWave++;
		}
	}   
	else
	{
		for (i = 0; i < K_FFT_SIZE; i++) 
			*pFFTRe++ = *aWave++;
	}

	pFFTRe = sFFTRe;
	window(pFFTRe, K_FFT_SIZE);
	fix_fft(pFFTRe, pFFTIm, K_FFT_SHIFT, 0);

	double root, squr;
	TTInt16* pDstFreq = aFreq;
	squr = (*pFFTRe) * (*pFFTRe) + (*pFFTIm) * (*pFFTIm);
	root = sqrt(squr );
	*pDstFreq++ = (TTInt16)root;

	pFFTIm++;
	pFFTRe++;
	for(TTInt i = 1; i < K_FFT_SIZE/2; i++) 
	{
		squr = (*pFFTRe) * (*pFFTRe) + (*pFFTIm) * (*pFFTIm);
		root = sqrt(squr);
		*pDstFreq++ = (TTInt16)(2 * root);
		pFFTIm++;
		pFFTRe++;
	}   
}
// end of file