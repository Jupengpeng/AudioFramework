/**
 *  BiQuad filter
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

#include "freeverb/biquad.hpp"
#include "freeverb/fv3_type_float.h"
#include "freeverb/fv3_ns_start.h"

FV3_(biquad)::FV3_(biquad)()
{
  ;
}

FV3_(biquad)::~FV3_(biquad)()
{
  ;
}

void FV3_(biquad)::printconfig()
{
  std::fprintf(stderr, "<< BiQuad Filter Coefficients >>\n");
  std::fprintf(stderr, "(in)--+----*b0-->+----------+->(out) \n");
  std::fprintf(stderr, "      |          ^          |        \n");
  std::fprintf(stderr, "      v          |          v        \n");
  std::fprintf(stderr, "  [z^-1]---*b1-->+<-*(-a1)-[z^-1]    \n");
  std::fprintf(stderr, "      |          ^          |        \n");
  std::fprintf(stderr, "      v          |          v        \n");
  std::fprintf(stderr, "  [z^-1]---*b2-->+<-*(-a2)-[z^-1]    \n");
  std::fprintf(stderr, "b0 = %f, b1 = %f, b2 = %f\n", b0, b1, b2);
  std::fprintf(stderr, "a1 = %f, a2 = %f\n", a1, a2);
}

void FV3_(biquad)::mute()
{
  i1 = i2 = o1 = o2 = 0;
  t0 = t1 = t2 = 0;
}

void FV3_(biquad)::setCoefficients(fv3_float_t _b0, fv3_float_t _b1, fv3_float_t _b2, fv3_float_t _a1, fv3_float_t _a2)
{
  b0 = _b0; b1 = _b1; b2 = _b2; a1 = _a1; a2 = _a2;
}

/* The majority of the definitions and helper functions below have been
   derived from the source code of Steve Harris's SWH plugins.
   Biquad filter (adapted from lisp code by Eli Brandt, http://www.cs.cmu.edu/~eli/) 
   See the Cookbook formulae for audio EQ biquad filter coefficients
   by Robert Bristow-Johnson <rbj@audioimagination.com> for more details.
*/

static inline fv3_float_t BQ_LIMIT(fv3_float_t v,fv3_float_t l,fv3_float_t u){return ((v)<(l)?(l):((v)>(u)?(u):(v)));}

void FV3_(biquad)::setLPF(fv3_float_t fc, fv3_float_t bw, fv3_float_t fs)
{
  fv3_float_t omega = 2.0 * M_PI * fc/fs;
  fv3_float_t sn = std::sin(omega);
  fv3_float_t cs = std::cos(omega);
  fv3_float_t alpha = sn * std::sinh(M_LN2 / 2.0 * bw * omega / sn);
  fv3_float_t a0r = 1.0 / (1.0 + alpha);
#if 0
  b0 = (1 - cs) /2;
  b1 = 1 - cs;
  b2 = (1 - cs) /2;
  a0 = 1 + alpha;
  a1 = -2 * cs;
  a2 = 1 - alpha;
#endif
 b0 = a0r * (1.0 - cs) * 0.5;
 b1 = a0r * (1.0 - cs);
 b2 = a0r * (1.0 - cs) * 0.5;
 a1 = -1 * a0r * (2.0 * cs);
 a2 = -1 * a0r * (alpha - 1.0);
}

void FV3_(biquad)::setHPF(fv3_float_t fc, fv3_float_t bw, fv3_float_t fs)
{
  fv3_float_t omega = 2.0 * M_PI * fc/fs;
  fv3_float_t sn = std::sin(omega);
  fv3_float_t cs = std::cos(omega);
  fv3_float_t alpha = sn * std::sinh(M_LN2 / 2.0 * bw * omega / sn);
  float a0r = 1.0 / (1.0 + alpha);
#if 0
  b0 = (1 + cs) /2;
  b1 = -(1 + cs);
  b2 = (1 + cs) /2;
  a0 = 1 + alpha;
  a1 = -2 * cs;
  a2 = 1 - alpha;
#endif
 b0 = a0r * (1.0 + cs) * 0.5;
 b1 = a0r * -(1.0 + cs);
 b2 = a0r * (1.0 + cs) * 0.5;
 a1 = -1 * a0r * (2.0 * cs);
 a2 = -1 * a0r * (alpha - 1.0);
}

void FV3_(biquad)::setEQP(fv3_float_t fc, fv3_float_t gain, fv3_float_t bw, fv3_float_t fs)
{
  fv3_float_t w = 2.0f * M_PI * BQ_LIMIT(fc, 1.0f, fs/2.0f) / fs;
  fv3_float_t cw = cosf(w);
  fv3_float_t sw = sinf(w);
  fv3_float_t J = pow(10.0f, gain * 0.025f);
  fv3_float_t g = sw * sinhf(LN_2_2 * BQ_LIMIT(bw, 0.0001f, 4.0f) * w / sw);
  fv3_float_t a0r = 1.0f / (1.0f + (g / J));
  b0 = (1.0f + (g * J)) * a0r;
  b1 = (-2.0f * cw) * a0r;
  b2 = (1.0f - (g * J)) * a0r;
  a1 = b1;
  a2 = -1 * ((g / J) - 1.0f) * a0r;
}

void FV3_(biquad)::setLSF(fv3_float_t fc, fv3_float_t gain, fv3_float_t slope, fv3_float_t fs)
{
  fv3_float_t w = 2.0f * M_PI * BQ_LIMIT(fc, 1.0, fs/2.0) / fs;
  fv3_float_t cw = cosf(w);
  fv3_float_t sw = sinf(w);
  fv3_float_t A = powf(10.0f, gain * 0.025f);
  fv3_float_t b = sqrt(((1.0f + A * A) / BQ_LIMIT(slope, 0.0001f, 1.0f)) - ((A - 1.0f) * (A - 1.0)));
  fv3_float_t apc = cw * (A + 1.0f);
  fv3_float_t amc = cw * (A - 1.0f);
  fv3_float_t bs = b * sw;
  fv3_float_t a0r = 1.0f / (A + 1.0f + amc + bs);
  b0 = a0r * A * (A + 1.0f - amc + bs);
  b1 = a0r * 2.0f * A * (A - 1.0f - apc);
  b2 = a0r * A * (A + 1.0f - amc - bs);
  a1 = -1 * a0r * 2.0f * (A - 1.0f + apc);
  a2 = -1 * a0r * (-A - 1.0f - amc + bs);
}

void FV3_(biquad)::setHSF(fv3_float_t fc, fv3_float_t gain, fv3_float_t slope, fv3_float_t fs)
{
  fv3_float_t w = 2.0f * M_PI * BQ_LIMIT(fc, 1.0, fs/2.0) / fs;
  fv3_float_t cw = cosf(w);
  fv3_float_t sw = sinf(w);
  fv3_float_t A = powf(10.0f, gain * 0.025f);
  fv3_float_t b = sqrt(((1.0f + A * A) / BQ_LIMIT(slope, 0.0001f, 1.0f)) - ((A - 1.0f) * (A - 1.0f)));
  fv3_float_t apc = cw * (A + 1.0f);
  fv3_float_t amc = cw * (A - 1.0f);
  fv3_float_t bs = b * sw;
  fv3_float_t a0r = 1.0f / (A + 1.0f - amc + bs);
  b0 = a0r * A * (A + 1.0f + amc + bs);
  b1 = a0r * -2.0f * A * (A - 1.0f + apc);
  b2 = a0r * A * (A + 1.0f + amc - bs);
  a1 = -1 * a0r * -2.0f * (A - 1.0f - apc);
  a2 = -1 * a0r * (-A - 1.0f + amc + bs);
}

#include "freeverb/fv3_ns_end.h"
