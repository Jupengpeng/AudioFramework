/*  FFT impulse fragment square multiplier
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

#include "freeverb/frag.hpp"
#include "freeverb/fv3_type_float.h"
#include "freeverb/fv3_ns_start.h"


#ifdef FRAG_SIMD_CODE
#undef FRAG_SIMD_CODE
#endif
#ifdef FRAG_SIMD_8
#undef FRAG_SIMD_8
#endif
#ifdef FRAG_SIMD_4
#undef FRAG_SIMD_4
#endif
#ifdef FRAG_SIMD_2
#undef FRAG_SIMD_2
#endif
#ifdef FRAG_SIMD_1
#undef FRAG_SIMD_1
#endif

#ifdef LIBFV3_FLOAT
#if   defined(ENABLE_SSE3)||defined(ENABLE_SSE4)
#define FRAG_SIMD_CODE
#define FRAG_SIMD_1
#elif defined(ENABLE_SSE_V2)||defined(ENABLE_SSE2)
#define FRAG_SIMD_CODE
#define FRAG_SIMD_1
#elif defined(ENABLE_SSE)
#define FRAG_SIMD_CODE
#define FRAG_SIMD_4
#elif defined(ENABLE_3DNOW)
#define FRAG_SIMD_CODE
#define FRAG_SIMD_2
#endif
#endif

#ifdef LIBFV3_DOUBLE
#if   defined(ENABLE_SSE4)
#define FRAG_SIMD_CODE
#define FRAG_SIMD_1
#elif defined(ENABLE_SSE2)||defined(ENABLE_SSE3)
#define FRAG_SIMD_CODE
#define FRAG_SIMD_2
#endif
#endif

// class fragfft

FV3_(fragfft)::FV3_(fragfft)()
{
  fragmentSize = 0;
}

FV3_(fragfft)::FV3_(~fragfft)()
{
  freeFFT();
}

long FV3_(fragfft)::getFragmentSize()
{
  return fragmentSize;
}

void FV3_(fragfft)::allocFFT(long size, unsigned fftflags)
		   /*throw(std::bad_alloc)*/
{
#ifdef DEBUG
  std::fprintf(stderr, "fragfft::allocFFT(%ld)\n", size);
#endif
  if(FV3_IR_Min_FragmentSize > size)
    {
      std::fprintf(stderr, "fragfft::allocFFT(size=%ld): fragmentSize(>%d) is too small! \n",
		   size, FV3_IR_Min_FragmentSize);
      /*throw(std::bad_alloc)*/;
    }
  if(size != FV3_(utils)::checkPow2(size))
    {
      std::fprintf(stderr, "fragfft::allocFFT(size=%ld): fragmentSize must be 2^n.\n", size);
      /*throw(std::bad_alloc)*/;
    }
  freeFFT();
  fftOrig.alloc(2*size, 2);
  planOrigL = FFTW_(plan_r2r_1d)(2*size, fftOrig.L, fftOrig.L, FFTW_R2HC, fftflags);
  planOrigR = FFTW_(plan_r2r_1d)(2*size, fftOrig.R, fftOrig.R, FFTW_R2HC, fftflags);
  planRevL = FFTW_(plan_r2r_1d)(2*size, fftOrig.L, fftOrig.L, FFTW_HC2R, fftflags);
  planRevR = FFTW_(plan_r2r_1d)(2*size, fftOrig.R, fftOrig.R, FFTW_HC2R, fftflags);
  fragmentSize = size;
}

void FV3_(fragfft)::freeFFT()
{
  if(fragmentSize == 0) return;
  FFTW_(destroy_plan)(planRevL);
  FFTW_(destroy_plan)(planRevR);
  FFTW_(destroy_plan)(planOrigL);
  FFTW_(destroy_plan)(planOrigR);
  fftOrig.free();
  fragmentSize = 0;
}

void FV3_(fragfft)::R2SA(const fv3_float_t * in, fv3_float_t * out, long n)
{
#if defined(FRAG_SIMD_4)
  out[0] = in[0]; // r0
  out[1] = in[1]; // r1
  out[2] = in[2]; // r2
  out[3] = in[3]; // r3
  out[4+0] = in[n/2]; // r(n/2)
  out[4+1] = in[n-1]; // i1
  out[4+2] = in[n-2]; // i2
  out[4+3] = in[n-3]; // i3
  for(long t = 1;t < n/8;t ++)
    {
      out[8*t+0] = in[4*t];
      out[8*t+1] = in[4*t+1];
      out[8*t+2] = in[4*t+2];
      out[8*t+3] = in[4*t+3];
      out[8*t+4+0] = in[n-4*t];
      out[8*t+4+1] = in[n-4*t-1];
      out[8*t+4+2] = in[n-4*t-2];
      out[8*t+4+3] = in[n-4*t-3];
    }
  return;
#elif defined(FRAG_SIMD_2)
  out[0] = in[0]; // r0
  out[1] = in[1]; // r1
  out[2] = in[n/2]; // r(n/2)
  out[3] = in[n-1]; // i1
  for(long t = 1;t < n/4;t ++)
    {
      out[4*t+0] = in[2*t];
      out[4*t+1] = in[2*t+1];
      out[4*t+2] = in[n-2*t];
      out[4*t+3] = in[n-2*t-1];
    }
  return;
#else
  out[0] = in[0];
  out[1] = in[n/2];
  for(long t = 1;t < n/2;t ++)
    {
      out[2*t+0] = in[t];
      out[2*t+1] = in[n-t];
    }
  return;
#endif
}

void FV3_(fragfft)::R2HC(const fv3_float_t * iL, const fv3_float_t * iR,
			 fv3_float_t * oL, fv3_float_t * oR)
{
  if(fragmentSize == 0) return;
#pragma omp parallel
#pragma omp sections
  {
#pragma omp section
    {
      FV3_(utils)::mute(fftOrig.L+fragmentSize, fragmentSize);
      std::memcpy(fftOrig.L, iL, sizeof(fv3_float_t)*fragmentSize);
      FFTW_(execute)(planOrigL);
      R2SA(fftOrig.L, oL, fragmentSize*2);
    }
#pragma omp section
    {
      FV3_(utils)::mute(fftOrig.R+fragmentSize, fragmentSize);
      std::memcpy(fftOrig.R, iR, sizeof(fv3_float_t)*fragmentSize);
      FFTW_(execute)(planOrigR);
      R2SA(fftOrig.R, oR, fragmentSize*2);
    }
  }
#pragma omp barrier
  return;
}

void FV3_(fragfft)::SA2R(const fv3_float_t * in, fv3_float_t * out, long n)
{
#if defined(FRAG_SIMD_4)
  out[0] = in[0]; // r0
  out[1] = in[1]; // r1
  out[2] = in[2]; // r2
  out[3] = in[3]; // r3
  out[n/2] = in[4+0]; // r(n/2)
  out[n-1] = in[4+1]; // i1
  out[n-2] = in[4+2]; // i2
  out[n-3] = in[4+3]; // i3
  for(long t = 1;t < n/8;t ++)
    {
      out[4*t] = in[8*t+0];
      out[4*t+1] = in[8*t+1];
      out[4*t+2] = in[8*t+2];
      out[4*t+3] = in[8*t+3];
      out[n-4*t] = in[8*t+4+0];
      out[n-4*t-1] = in[8*t+4+1];
      out[n-4*t-2] = in[8*t+4+2];
      out[n-4*t-3] = in[8*t+4+3];
    }
  return;
#elif defined(FRAG_SIMD_2)
  out[0] = in[0]; // r0
  out[1] = in[1]; // r1
  out[n/2] = in[2]; // r(n/2)
  out[n-1] = in[3]; // i1
  for(long t = 1;t < n/4;t ++)
    {
      out[2*t] = in[4*t+0];
      out[2*t+1] = in[4*t+1];
      out[n-2*t] = in[4*t+2];
      out[n-2*t-1] = in[4*t+3];
    }
  return;
#else
  out[0] = in[0];
  out[n/2] = in[1];
  for(long t = 1;t < n/2;t ++)
    {
      out[t] = in[2*t+0];
      out[n-t] = in[2*t+1];
    }
  return;
#endif
}

void FV3_(fragfft)::HC2R(const fv3_float_t * iL, const fv3_float_t * iR,
			 fv3_float_t * oL, fv3_float_t * oR)
{
  if(fragmentSize == 0) return;
#pragma omp parallel
#pragma omp sections
  {
#pragma omp section
    {
      SA2R(iL, fftOrig.L, fragmentSize*2);
      FFTW_(execute)(planRevL);
      for(long i = 0;i < fragmentSize*2;i ++) oL[i] += fftOrig.L[i];
    }
#pragma omp section
    {
      SA2R(iR, fftOrig.R, fragmentSize*2);
      FFTW_(execute)(planRevR);
      for(long i = 0;i < fragmentSize*2;i ++) oR[i] += fftOrig.R[i];
    }
  }
  return;
}

// class frag

FV3_(frag)::FV3_(frag)()
{
  fragmentSize = 0;
  fftImpulse.L = fftImpulse.R = NULL;
}

FV3_(frag)::FV3_(~frag)()
{
  unloadImpulse();
}

void FV3_(frag)::loadImpulse(const fv3_float_t * L, const fv3_float_t * R, long size, long limit, unsigned fftflags)
		/*throw(std::bad_alloc)*/
{
  this->loadImpulse(L,R,size,limit,fftflags,NULL,NULL);
}

void FV3_(frag)::loadImpulse(const fv3_float_t * L, const fv3_float_t * R, long size, long limit, unsigned fftflags,
			     fv3_float_t * preAllocatedL, fv3_float_t * preAllocatedR)
		/*throw(std::bad_alloc)*/
{
#ifdef DEBUG
  std::fprintf(stderr, "frag::loadImpulse(f=%ld,l=%ld)\n", size, limit);
#endif
  if(FV3_IR_Min_FragmentSize > size)
    {
      std::fprintf(stderr, "frag::loadImpulse(f=%ld,l=%ld): fragmentSize(>%d) is too small. \n",
		   size, limit, FV3_IR_Min_FragmentSize);
      /*throw(std::bad_alloc)*/;
    }
  if(size != FV3_(utils)::checkPow2(size))
    {
      std::fprintf(stderr, "frag::loadImpulse(f=%ld,l=%ld): fragmentSize must be 2^n.\n",
		   size, limit);
      /*throw(std::bad_alloc)*/;
    }
  if(size < limit) limit = size;
  unloadImpulse();
  FV3_(fragfft) fragFFT;
  // impulse = [_Re_ impulse...< limit 0...0 (size)][_Im_ 0...0 (size*2)]
  FV3_(slot) impulse;
  impulse.alloc(size, 2);
  
  for(long i = 0;i < limit;i ++)
    {
      impulse.L[i] = L[i] / (fv3_float_t)(size*2);
      impulse.R[i] = R[i] / (fv3_float_t)(size*2);
    }

  //try
   // {
      if(preAllocatedL == NULL||preAllocatedR == NULL)
	allocImpulse(size);
      else
	registerPreallocatedBlock(preAllocatedL, preAllocatedR, size);
      fragFFT.allocFFT(size, fftflags);
   // }
  //catch(std::bad_alloc)
   // {
    //  unloadImpulse();
    //  throw;
   // }
  fragFFT.R2HC(impulse.L, impulse.R, fftImpulse.L, fftImpulse.R);
}

void FV3_(frag)::registerPreallocatedBlock(fv3_float_t * _L, fv3_float_t * _R, long size)
{
  freeImpulse();
  fragmentSize = size;
  fftImpulse.L = _L;
  fftImpulse.R = _R;
}

void FV3_(frag)::allocImpulse(long size)
		/*throw(std::bad_alloc)*/
{
  freeImpulse();
  fragmentSize = size;
  fftImpulse.alloc(2*size, 2);
}

void FV3_(frag)::freeImpulse()
{
  if(fragmentSize == 0) return;
  fftImpulse.free();
  fragmentSize = 0;
}

void FV3_(frag)::unloadImpulse()
{
  freeImpulse();
}

static void MULT_M(const fv3_float_t * iL, const fv3_float_t * fL, fv3_float_t * oL, long n)
#ifdef __GNUC__
  __attribute__((noinline))
#endif
  ;

static void MULT_M(const fv3_float_t * iL, const fv3_float_t * fL, fv3_float_t * oL, long n)
{
#ifdef FRAG_SIMD_CODE
  void * soL = NULL;
#endif
  
#ifdef LIBFV3_FLOAT
#if defined(ENABLE_SSE3)||defined(ENABLE_SSE4)
  __asm__ __volatile__
    ("MULT_M_sse3_single:     \n\t"
     "mov    %0, %4           \n\t" // save %0(oL)
     // float tL0 = oL[0] + iL[0] * fL[0];
     // float tL1 = oL[1] + iL[1] * fL[1];
     "movaps (%1), %%xmm1     \n\t"
     "mulps  (%2), %%xmm1     \n\t"
     "addps  (%0), %%xmm1     \n\t"
     // TODO: Storing float[2] to 64bit MMX register is not a good idea.
     "movdq2q %%xmm1, %%mm0   \n\t" // save float[2] to mm0
     // loop
     "MULT_M_sse3_single_loop:\n\t"
     "prefetchnta 0x80(%1)    \n\t"
     "movaps     (%1), %%xmm0 \n\t" // xmm0,1,3,2
     "movaps 0x10(%1), %%xmm4 \n\t" // xmm4,5,7,6
     "add    $0x20, %1        \n\t"
     "prefetchnta 0x80(%2)    \n\t"
     "movsldup (%2), %%xmm1   \n\t"
     "movshdup (%2), %%xmm3   \n\t"
     "movsldup 0x10(%2), %%xmm5 \n\t"
     "movshdup 0x10(%2), %%xmm7 \n\t"
     "add    $0x20, %2        \n\t"
     "add    $0x20, %0        \n\t"
     "pshufd $0xb1, %%xmm0, %%xmm2 \n\t"
     "pshufd $0xb1, %%xmm4, %%xmm6 \n\t"
     "mulps  %%xmm1, %%xmm0   \n\t"
     "mulps  %%xmm5, %%xmm4   \n\t"
     "prefetcht2 0x80(%0)     \n\t"
     "addps  -0x20(%0), %%xmm0 \n\t"
     "addps  -0x10(%0), %%xmm4 \n\t"
     "mulps  %%xmm3, %%xmm2   \n\t"
     "mulps  %%xmm7, %%xmm6   \n\t"
     "addsubps %%xmm2, %%xmm0 \n\t"
     "addsubps %%xmm6, %%xmm4 \n\t"
     "dec    %3               \n\t"
     "movaps %%xmm0, -0x20(%0)\n\t"
     "movaps %%xmm4, -0x10(%0)\n\t"
     "jne    MULT_M_sse3_single_loop \n\t"
     "MULT_M_sse3_single_save: \n\t"
     // oL[0] = tL0;
     // oL[1] = tL1;
     "movq2dq %%mm0, %%xmm1   \n\t" // restore float[2] from mm0
     // EMMS takes approximately 58 clocks extra.
     "emms                    \n\t"
     "movlps %%xmm1, (%4)     \n\t" // restore float[2]
     :
     : "q"(oL), "q"(iL), "q"(fL), "r"(n/4), "r"(soL)
     : "memory");
  return;
#endif

#if defined(ENABLE_SSE_V2)||defined(ENABLE_SSE2)
  __asm__ __volatile__
    ("MULT_M_sse_v2:          \n\t"
     "mov    %0, %4           \n\t" // save %0 to %4
     // float tL0 = oL[0] + iL[0] * fL[0];
     // float tL1 = oL[1] + iL[1] * fL[1];
     "movaps (%1), %%xmm1     \n\t"
     "mulps  (%2), %%xmm1     \n\t"
     "addps  (%0), %%xmm1     \n\t" // saved in xmm1
     "xor    %0, %0           \n\t" // clear %0
     "mov    $0x80000000, %0  \n\t" // 32bit float -1 mask
     "mov    %0, (%4)         \n\t"
     "xorps  %%xmm2, %%xmm2   \n\t" // pxor xmm,xmm (mmx,sse2)
     "movss  (%4), %%xmm2     \n\t" // movd eax,xmm (sse2)
     "movlhps %%xmm2, %%xmm2  \n\t" // bitmask = xmm2 ([0]=-1,[1]=0,[2]=-1,[3]=0)
     // XMM2 = 00000000 80000000 00000000 80000000h
     "mov    %4, %0           \n\t" // restore %0(oL)
     // loop
     "MULT_M_sse_v2_loop:     \n\t" // bitmask:xmm2
     "prefetchnta 0x80(%1)    \n\t"
     "movaps     (%1), %%xmm7 \n\t" // xmm3,5,7
     "movaps 0x10(%1), %%xmm0 \n\t" // xmm0,4,6
     "prefetchnta 0x80(%2)    \n\t"
     "movaps     (%2), %%xmm3 \n\t"
     "movaps 0x10(%2), %%xmm4 \n\t"
     "add    $0x20, %2        \n\t"
     "movaps %%xmm7, %%xmm5   \n\t"
     "movaps %%xmm0, %%xmm6   \n\t"
     "add    $0x20, %1        \n\t"
     "shufps $0xa0, %%xmm7, %%xmm5 \n\t"
     "shufps $0xa0, %%xmm0, %%xmm6 \n\t"
     "add    $0x20, %0        \n\t"
     "mulps  %%xmm3, %%xmm5   \n\t"
     "mulps  %%xmm4, %%xmm6   \n\t"
     "prefetcht2 0x80(%0)     \n\t"
     "addps  -0x20(%0), %%xmm5 \n\t"
     "addps  -0x10(%0), %%xmm6 \n\t"
     "shufps $0xf5, %%xmm7, %%xmm7 \n\t"
     "shufps $0xf5, %%xmm0, %%xmm0 \n\t"
     "xorps  %%xmm2, %%xmm7   \n\t"
     "xorps  %%xmm2, %%xmm0   \n\t"
     "shufps $0xb1, %%xmm3, %%xmm3 \n\t"
     "shufps $0xb1, %%xmm4, %%xmm4 \n\t"
     "mulps  %%xmm3, %%xmm7   \n\t"
     "mulps  %%xmm4, %%xmm0   \n\t"
     "addps  %%xmm5, %%xmm7   \n\t"
     "addps  %%xmm6, %%xmm0   \n\t"
     "dec    %3               \n\t"
     "movaps %%xmm7, -0x20(%0) \n\t"
     "movaps %%xmm0, -0x10(%0) \n\t"
     "jne    MULT_M_sse_v2_loop \n\t"
     "MULT_M_sse_v2_save:     \n\t"
     // oL[0] = tL0; oL[1] = tL1;
     "movlps %%xmm1, (%4)     \n\t"
     :
     : "q"(oL), "q"(iL), "q"(fL), "r"(n/4), "r"(soL)
     : "memory");
  return;
#endif

#ifdef ENABLE_SSE
  __asm__ __volatile__
    ("MULT_M_sse:             \n\t"
     "mov    %0, %4           \n\t"
     // float tL0 = oL[0] + iL[0] * fL[0];
     "movss  (%1), %%xmm6     \n\t"
     "mulss  (%2), %%xmm6     \n\t"
     "addss  (%0), %%xmm6     \n\t"
     // float tL4 = oL[4] + iL[4] * fL[4];
     "movss  0x10(%1), %%xmm7 \n\t"
     "mulss  0x10(%2), %%xmm7 \n\t"
     "addss  0x10(%0), %%xmm7 \n\t"
     // loop
     "MULT_M_sse_loop:        \n\t"
     "prefetchnta 0x60(%1)    \n\t"
     "movaps (%1), %%xmm0     \n\t"
     "movaps %%xmm0, %%xmm2   \n\t"
     "prefetchnta 0x60(%2)    \n\t"
     "movaps (%2), %%xmm1     \n\t"
     "mulps  %%xmm1, %%xmm0   \n\t"
     "movaps 0x10(%1), %%xmm4 \n\t"
     "movaps %%xmm4, %%xmm3   \n\t"
     "mulps  %%xmm1, %%xmm3   \n\t"
     "add    $0x20, %1        \n\t"
     "movaps 0x10(%2), %%xmm5 \n\t"
     "mulps  %%xmm5, %%xmm4   \n\t"
     "subps  %%xmm4, %%xmm0   \n\t"
     "mulps  %%xmm5, %%xmm2   \n\t"
     "add    $0x20, %2        \n\t"
     "prefetcht2 0x60(%0)     \n\t"
     "addps  (%0), %%xmm0     \n\t"
     "movaps %%xmm0, (%0)     \n\t"
     "addps  %%xmm3, %%xmm2   \n\t"
     "addps  0x10(%0), %%xmm2 \n\t"
     "movaps %%xmm2, 0x10(%0) \n\t"
     "add    $0x20, %0        \n\t"
     "dec    %3               \n\t"
     "jne    MULT_M_sse_loop  \n\t"
     "MULT_M_sse_save:        \n\t"
     // oL[0] = tL0;
     "movss  %%xmm6, (%4)     \n\t"
     // oL[4] = tL4;
     "movss  %%xmm7, 0x10(%4) \n\t"
     :
     : "q"(oL), "q"(iL), "q"(fL), "r"(n/4), "r"(soL)
     : "memory");
  return;
#endif

#ifdef ENABLE_3DNOW
  __asm__ __volatile__
    ("MULT_M_3dnow:           \n\t"
     "mov    %0, %4           \n\t"
     "femms                   \n\t"
     // float tL0 = oL[0] + iL[0] * fL[0];
     "movd  (%1), %%mm6       \n\t"
     "pfmul (%2), %%mm6       \n\t"
     "pfadd (%0), %%mm6       \n\t"
     // float tL2 = oL[2] + iL[2] * fL[2];
     "movd  0x8(%1), %%mm7    \n\t"
     "pfmul 0x8(%2), %%mm7    \n\t"
     "pfadd 0x8(%0), %%mm7    \n\t"
     // loop
     "MULT_M_3dnow_loop:      \n\t"
     "prefetch 0x60(%1)       \n\t"
     "movq  (%1), %%mm0       \n\t"
     "movq  %%mm0, %%mm2      \n\t"
     "prefetch 0x60(%2)       \n\t"
     "movq  (%2), %%mm1       \n\t"
     "pfmul %%mm1, %%mm0      \n\t"
     "movq  0x8(%1), %%mm4    \n\t"
     "movq  %%mm4, %%mm3      \n\t"
     "pfmul %%mm1, %%mm3      \n\t"
     "add   $0x10, %1         \n\t"
     "movq  0x8(%2), %%mm5    \n\t"
     "pfmul %%mm5, %%mm4      \n\t"
     "pfsub %%mm4, %%mm0      \n\t"
     "pfmul %%mm5, %%mm2      \n\t"
     "add   $0x10, %2         \n\t"
     "prefetchw 0x60(%0)      \n\t"
     "pfadd (%0), %%mm0       \n\t"
     "movq  %%mm0, (%0)       \n\t"
     "pfadd %%mm3, %%mm2      \n\t"
     "pfadd 0x8(%0), %%mm2    \n\t"
     "movq  %%mm2, 0x8(%0)    \n\t"
     "add   $0x10, %0         \n\t"
     "dec   %3                \n\t"
     "jne   MULT_M_3dnow_loop \n\t"
     "MULT_M_3dnow_save:      \n\t"
     // oL[0] = tL0;
     "movd  %%mm6, (%4)       \n\t"
     // oL[2] = tL2;
     "movd  %%mm7, 0x8(%4)    \n\t"
     "femms                   \n\t"
     :
     : "q"(oL), "q"(iL), "q"(fL), "r"(n/2), "r"(soL)
     : "memory");
  return;
#endif
#endif

#ifdef LIBFV3_DOUBLE
#ifdef ENABLE_SSE4
  __asm__ __volatile__
    ("MULT_M_sse4:            \n\t"
     // double tL0 = oL[0] + iL[0] * fL[0];
     // double tL1 = oL[1] + iL[1] * fL[1];
     "movapd (%1), %%xmm7     \n\t"
     "mulpd  (%2), %%xmm7     \n\t"
     "addpd  (%0), %%xmm7     \n\t"
     "mov    $0x80, %4        \n\t"
     "pxor   %%xmm6, %%xmm6   \n\t"
     "movd   %4, %%xmm6       \n\t"
     // The bitshift is not supported in SSE :-(
     // ex.
     // pcmpeqd xmm0, xmm0 XMM0 = 0FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFh
     // pslldq  xmm0, 3    XMM0 = 0FFFFFFFFFFFFFFFFFFFFFFFFFF000000h pslldq:sse2
     "pslldq $0xf, %%xmm6  \n\t" // bitmask = xmm6 ([0]=0,[1]=-1)
     // XMM6 = 8000000000000000 0000000000000000h
     "mov    %0, %4           \n\t" // save %0(oL)
     // loop
     "MULT_M_sse4_loop:       \n\t" // bitmask:xmm6
     "prefetchnta 0x80(%1)    \n\t"
     "movapd     (%1), %%xmm0 \n\t" // xmm0,1,2
     "movapd 0x10(%1), %%xmm3 \n\t" // xmm3,4,5
     "add    $0x20, %1        \n\t"
     "prefetchnta 0x80(%2)    \n\t"
     "movapd     (%2), %%xmm1 \n\t"
     "movapd 0x10(%2), %%xmm4 \n\t"
     "add    $0x20, %2        \n\t"
     "movapd %%xmm1, %%xmm2   \n\t"
     "movapd %%xmm4, %%xmm5   \n\t"
     "add    $0x20, %0        \n\t"
     "xorpd  %%xmm6, %%xmm1   \n\t"
     "xorpd  %%xmm6, %%xmm4   \n\t"
     "dppd   $0x31, %%xmm0, %%xmm1 \n\t" // (a,b)#(c,-d)->(ac-bd,0)
     "dppd   $0x31, %%xmm3, %%xmm4 \n\t"
     "shufpd $0x3, %%xmm2, %%xmm2 \n\t" // (c,d)->(d,c)
     "shufpd $0x3, %%xmm5, %%xmm5 \n\t"
     "dppd   $0x32, %%xmm0, %%xmm2 \n\t" // (a,b)#(d,c)->(0,ad+bc)
     "dppd   $0x32, %%xmm3, %%xmm5 \n\t"
     "xorpd  %%xmm1, %%xmm2   \n\t" // (ac-bd,ad+bc)
     "xorpd  %%xmm4, %%xmm5   \n\t"
     "prefetcht2 0x80(%0)     \n\t"
     "addpd  -0x20(%0), %%xmm2 \n\t"
     "addpd  -0x10(%0), %%xmm5 \n\t"
     "dec    %3               \n\t"
     "movapd %%xmm2, -0x20(%0) \n\t"
     "movapd %%xmm5, -0x10(%0) \n\t"
     "jne    MULT_M_sse4_loop \n\t"
     "MULT_M_sse4_save:       \n\t"
     // oL[0] = tL0; oL[1] = tL1;
     "movapd %%xmm7, (%4)     \n\t"
     :
     : "q"(oL), "q"(iL), "q"(fL), "r"(n/2), "r"(soL)
     : "memory");
  return;
#endif

#if defined(ENABLE_SSE2)||defined(ENABLE_SSE3)
  __asm__ __volatile__
    ("MULT_M_sse2:            \n\t"
     "mov    %0, %4           \n\t"
     // double tL0 = oL[0] + iL[0] * fL[0];
     "movsd  (%1), %%xmm6     \n\t"
     "mulsd  (%2), %%xmm6     \n\t"
     "addsd  (%0), %%xmm6     \n\t"
     // double tL2 = oL[2] + iL[2] * fL[2];
     "movsd  0x10(%1), %%xmm7 \n\t"
     "mulsd  0x10(%2), %%xmm7 \n\t"
     "addsd  0x10(%0), %%xmm7 \n\t"
     // loop
     "MULT_M_sse2_loop:       \n\t"
     "prefetchnta 0x60(%1)    \n\t"
     "movapd (%1), %%xmm0     \n\t"
     "movapd %%xmm0, %%xmm2   \n\t"
     "prefetchnta 0x60(%2)    \n\t"
     "movapd (%2), %%xmm1     \n\t"
     "mulpd  %%xmm1, %%xmm0   \n\t"
     "movapd 0x10(%1), %%xmm4 \n\t"
     "movapd %%xmm4, %%xmm3   \n\t"
     "mulpd  %%xmm1, %%xmm3   \n\t"
     "add    $0x20, %1        \n\t"
     "movapd 0x10(%2), %%xmm5 \n\t"
     "mulpd  %%xmm5, %%xmm4   \n\t"
     "subpd  %%xmm4, %%xmm0   \n\t"
     "mulpd  %%xmm5, %%xmm2   \n\t"
     "add    $0x20, %2        \n\t"
     "prefetcht2 0x60(%0)     \n\t"
     "addpd  (%0), %%xmm0     \n\t"
     "movapd %%xmm0, (%0)     \n\t"
     "addpd  %%xmm3, %%xmm2   \n\t"
     "addpd  0x10(%0), %%xmm2 \n\t"
     "movapd %%xmm2, 0x10(%0) \n\t"
     "add    $0x20, %0        \n\t"
     "dec    %3               \n\t"
     "jne    MULT_M_sse2_loop \n\t"
     "MULT_M_sse2_save:       \n\t"
     // oL[0] = tL0;
     "movsd  %%xmm6,     (%4) \n\t"
     // oL[2] = tL2;
     "movsd  %%xmm7, 0x10(%4) \n\t"
     :
     : "q"(oL), "q"(iL), "q"(fL), "r"(n/2), "r"(soL)
     : "memory");
  return;
#endif
#endif
  
#ifndef FRAG_SIMD_CODE
  // normal FPU code
  fv3_float_t tL0 = oL[0] + iL[0] * fL[0];
  fv3_float_t tL1 = oL[1] + iL[1] * fL[1];
  for(long i = 0;i < n;i ++)
    {
      fv3_float_t e = iL[2*i+0];
      fv3_float_t d = iL[2*i+1];
      fv3_float_t f = fL[2*i+0];
      fv3_float_t g = fL[2*i+1];
      oL[2*i+0] += e*f - d*g;
      oL[2*i+1] += e*g + f*d;
    }
  oL[0] = tL0;
  oL[1] = tL1;
  return;
#endif
}

void FV3_(frag)::MULT(const fv3_float_t * iL, const fv3_float_t * iR,
		      fv3_float_t * oL, fv3_float_t * oR)
{
  if(fragmentSize == 0) return;    
#pragma omp parallel
#pragma omp sections
  {
#pragma omp section
    MULT_M(iL, fftImpulse.L, oL, fragmentSize);
#pragma omp section
    MULT_M(iR, fftImpulse.R, oR, fragmentSize);
  }
  return;
}

void FV3_(frag)::getFFT(fv3_float_t * oL, fv3_float_t * oR)
{
  if(fragmentSize == 0) return;    
  std::memcpy(oL, fftImpulse.L, sizeof(fv3_float_t)*fragmentSize*2);
  std::memcpy(oR, fftImpulse.R, sizeof(fv3_float_t)*fragmentSize*2);
}

long FV3_(frag)::getFragmentSize()
{
  return fragmentSize;
}

#include "freeverb/fv3_ns_end.h"
