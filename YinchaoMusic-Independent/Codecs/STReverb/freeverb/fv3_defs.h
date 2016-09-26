/*  Freeverb3
 *
 *  Copyright (C) 2006-2010 Teru KAMOGASHIRA
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _FV3_DEFS_H
#define _FV3_DEFS_H

#include <cmath>

#ifndef isfinite
#define isfinite(v) std::isfinite(v)
#endif
#ifndef isnormal
#define isnormal(v) std::isnormal(v)
#endif
#ifndef fpclassify
#define fpclassify(v) std::fpclassify(v)
#endif
#define STRINGIZEx(x) #x
#define STRINGIZE(x) STRINGIZEx(x)

#ifdef DEBUG
#define UNDENORMAL(v)							\
  if((fpclassify(v)!=FP_NORMAL)&&(fpclassify(v)!=FP_ZERO))		\
    {std::fprintf(stderr, "^[" STRINGIZE(v) "=%d]",fpclassify(v));v=0;}
#else
#define UNDENORMAL(v)							\
  if(fpclassify(v) != FP_NORMAL&&fpclassify(v) != FP_ZERO){v=0;}
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327
#endif
#ifndef M_LN2
#define M_LN2 0.69314718055994530942
#endif
#ifndef LN_2_2
#define LN_2_2 0.34657359
#endif

#if defined(ENABLE_SSE)||defined(ENABLE_SSE_V2)||defined(ENABLE_SSE2)||defined(ENABLE_SSE3)||defined(ENABLE_SSE4)
#define FV3_PTR_ALIGN_BYTE 16
#elif defined(ENABLE_AVX)
#define FV3_PTR_ALIGN_BYTE 32
#else
#define FV3_PTR_ALIGN_BYTE 16
#endif

#define FV3_IR_DEFAULT     (0U)
#define FV3_IR_MUTE_DRY    (1U << 1)
#define FV3_IR_MUTE_WET    (1U << 2)
#define FV3_IR_SKIP_FILTER (1U << 3)
#define FV3_IR_MONO2STEREO (1U << 4)
#define FV3_IR_SKIP_INIT   (1U << 5)
/* OBSOLETE #define FV3_IR_ZERO_LATENCY (1U << 6) */
#define FV3_IR_SWAP_LR     (1U << 7)
/* SIMD size */
#define FV3_IR_Min_FragmentSize 16
#define FV3_IR2_DFragmentSize 16384
#define FV3_IR3_DFragmentSize 1024
#define FV3_IR3_DefaultFactor 16

#ifdef __cplusplus
extern "C" {
#endif
  enum
    {
      FV3_W_BLACKMAN = 1,
      FV3_W_HANNING  = 2,
      FV3_W_HAMMING  = 3,
      FV3_W_KAISER   = 4,
      FV3_W_COSRO    = 5,
      FV3_W_SQUARE   = 6,
    };
  enum
    {
      FV3_3BSPLIT_IR_IR2 = 0,
      FV3_3BSPLIT_IR_IR3 = 1,
    };
  enum
    {
      FV3_SRC_SINC_BEST_QUALITY        =   0,
      FV3_SRC_SINC_MEDIUM_QUALITY      =   1,
      FV3_SRC_SINC_FASTEST             =   2,
      FV3_SRC_ZERO_ORDER_HOLD          =   3,
      FV3_SRC_LINEAR                   =   4,
      FV3_SRC_SINC_SLOW_BEST_QUALITY   =  10,
      FV3_SRC_SINC_SLOW_MEDIUM_QUALITY =  11,
      FV3_SRC_LPF_IIR_1                =  50,
      FV3_SRC_LPF_IIR_2                =  51,
    } ;
#ifdef __cplusplus
}
#endif

#endif
