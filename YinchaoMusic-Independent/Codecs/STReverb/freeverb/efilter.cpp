/**
 *  Many Types of Filters
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

#include "freeverb/efilter.hpp"
#include "freeverb/fv3_type_float.h"
#include "freeverb/fv3_ns_start.h"

// class iir_1st

FV3_(iir_1st)::FV3_(iir_1st)()
{
  ;
}

FV3_(iir_1st)::FV3_(~iir_1st)()
{
  ;
}

void FV3_(iir_1st)::mute()
{
  y1 = 0;
}

void FV3_(iir_1st)::printconfig()
{
  std::fprintf(stderr, "<< 1st order IIR Filter Coefficients >>\n");
  std::fprintf(stderr, "(in)--+----*b1-->+----------+->(out) \n");
  std::fprintf(stderr, "      |          ^          |        \n");
  std::fprintf(stderr, "      v          |          v        \n");
  std::fprintf(stderr, "  [z^-1]---*b2-->+<--*a2---[z^-1]    \n");
  std::fprintf(stderr, "b1 = %f, b2 = %f\n", b1, b2);
  std::fprintf(stderr, "a1 = 1, a2 = %f\n", a2);
}

void FV3_(iir_1st)::setCoefficients(fv3_float_t _b1, fv3_float_t _b2, fv3_float_t _a2)
{
  b1 = _b1; b2 = _b2; a2 = _a2;
}

/*
  The functions below have been derived from the book of An Audio Cookbook by Christopher Moore.
  First Order Digital Filters--An Audio Cookbook. Application Note AN-11 by Christopher Moore
  http://www.sevenwoodsaudio.com/AN11.pdf
*/

void FV3_(iir_1st)::setLPFA(fv3_float_t fc, fv3_float_t fs)
{
  a2 = std::exp(-1*M_PI*fc/(fs/2.));
  b1 = 1.; b2 = .12;
  fv3_float_t norm = (1-a2)/(b1+b2);
  b1 *= norm; b2 *= norm;
}

void FV3_(iir_1st)::setHPFA(fv3_float_t fc, fv3_float_t fs)
{
  a2 = std::exp(-1*M_PI*fc/(fs/2.));
  b1 = 1.; b2 = -1.;
  fv3_float_t norm = (1+a2)/2.;
  b1 *= norm; b2 *= norm;
}

void FV3_(iir_1st)::setLSFA(fv3_float_t f1, fv3_float_t f2, fv3_float_t fs)
{
  a2 = -1 * std::exp(-1*M_PI*f1/(fs/2.));
  b1 = -1.;
  b2 = std::exp(-1*M_PI*f2/(fs/2.));
}

void FV3_(iir_1st)::setHSFA(fv3_float_t f1, fv3_float_t f2, fv3_float_t fs)
{
  a2 = std::exp(-1*M_PI*f1/(fs/2.));
  b1 = -1.;
  b2 = std::exp(-1*M_PI*f2/(fs/2.));
  fv3_float_t norm = (1-a2)/(b1+b2);
  b1 *= norm; b2 *= norm;
}

void FV3_(iir_1st)::setHPFwLFSA(fv3_float_t fc, fv3_float_t fs)
{
  b1 = -1.;
  b2 = std::exp(-1*M_PI*fc/(fs/2.));
  a2 = -.12;
  fv3_float_t norm = (1-a2)/std::abs(b1+b2);
  b1 *= norm; b2 *= norm;
}

/*
  The following functions are calculated using bilinear transform frequency warping.
  http://www.musicdsp.org/archive.php?classid=3
*/

void FV3_(iir_1st)::setLPFB(fv3_float_t fc, fv3_float_t fs)
{
  fv3_float_t omega_2 = M_PI*fc/fs;
  fv3_float_t tan_omega_2 = std::tan(omega_2);
  b1 = b2 = tan_omega_2/(1+tan_omega_2);
  a2 = (1-tan_omega_2)/(1+tan_omega_2);
}

void FV3_(iir_1st)::setHPFB(fv3_float_t fc, fv3_float_t fs)
{
  fv3_float_t omega_2 = M_PI*fc/fs;
  fv3_float_t tan_omega_2 = std::tan(omega_2);
  b1 = 1/(1+tan_omega_2);
  b2 = -1 * b1;
  a2 = (1-tan_omega_2)/(1+tan_omega_2);
}

void FV3_(iir_1st)::setLPFC(fv3_float_t fc, fv3_float_t fs)
{
  b1 = b2 = fc/(fs+fc);
  a2 = (fs-fc)/(fs+fc);
}

void FV3_(iir_1st)::setHPFC(fv3_float_t fc, fv3_float_t fs)
{
  b1 = fs/(fs+fc);
  b2 = -1 * b1;
  a2 = (fs-fc)/(fs+fc);
}

void FV3_(iir_1st)::setPole(fv3_float_t v)
{
  a2 = v;
  b1 = 1; b2 = 0;
  fv3_float_t norm = 1./(1-std::abs(a2));
  b1 *= norm; b2 *= norm;
}

void FV3_(iir_1st)::setZero(fv3_float_t v)
{
  a2 = 0; b1 = -1.;
  b2 = v;
  fv3_float_t norm = std::abs(b1) + std::abs(b2);
  b1 *= norm; b2 *= norm;
}

void FV3_(iir_1st)::setPoleLPF(fv3_float_t fc, fv3_float_t fs)
{
  fv3_float_t omega = 2.*M_PI*fc/fs;
  fv3_float_t cos_omega = std::cos(omega);
  fv3_float_t coeff = (2-cos_omega)-std::sqrt((2-cos_omega)*(2-cos_omega) - 1);
  a2 = coeff;
  b1 = 1-coeff; b2 = 0;
}

void FV3_(iir_1st)::setPoleHPF(fv3_float_t fc, fv3_float_t fs)
{
  fv3_float_t omega = 2.*M_PI*fc/fs;
  fv3_float_t cos_omega = std::cos(omega);
  fv3_float_t coeff = (2+cos_omega)-std::sqrt((2+cos_omega)*(2+cos_omega) - 1);
  a2 = -1 * coeff;
  b1 = coeff-1; b2 = 0;
}

void FV3_(iir_1st)::setZeroLPF(fv3_float_t fc, fv3_float_t fs)
{
  // fc > fs/4
  fv3_float_t omega = 2.*M_PI*fc/fs;
  fv3_float_t cos_omega = std::cos(omega);
  fv3_float_t coeff = (1-2*cos_omega)-std::sqrt((1-2*cos_omega)*(1-2*cos_omega)-1);
  a2 = 0;
  b1 = 1/(1+coeff);
  b2 = coeff/(1+coeff);
}

void FV3_(iir_1st)::setZeroHPF(fv3_float_t fc, fv3_float_t fs)
{
  // fc < fs/4
  fv3_float_t omega = 2.*M_PI*fc/fs;
  fv3_float_t cos_omega = std::cos(omega);
  fv3_float_t coeff = (1+2*cos_omega)-std::sqrt((1+2*cos_omega)*(1+2*cos_omega)-1);
  a2 = 0;
  b1 = 1/(1+coeff);
  b2 = -1 * coeff/(1+coeff);
}

// class efilter

FV3_(efilter)::FV3_(efilter)()
{
  setLPF(0);
  setHPF(0);
}

FV3_(efilter)::FV3_(~efilter)()
{
  ;
}

void FV3_(efilter)::mute()
{
  lpfL.mute(); lpfR.mute();
  hpfL.mute(); hpfR.mute();
}

void FV3_(efilter)::setLPF(fv3_float_t val)
{
  pole = val;
  lpfL.setPole(val);
  lpfR.setPole(val);
}

void FV3_(efilter)::setHPF(fv3_float_t val)
{
  zero = val;
  hpfL.setZero(val);
  hpfR.setZero(val);
}

fv3_float_t FV3_(efilter)::getLPF()
{
  return pole;
}

fv3_float_t FV3_(efilter)::getHPF()
{
  return zero;
}

// class dccut

FV3_(dccut)::FV3_(dccut)()
{
  gain = .9999f;
  mute();
}

FV3_(dccut)::~FV3_(dccut)()
{
  ;
}

void FV3_(dccut)::mute()
{
  y1 = y2 = 0;
}

void FV3_(dccut)::seta(fv3_float_t val) 
{
  gain = val;
}

fv3_float_t FV3_(dccut)::geta() 
{
  return gain;
}

void FV3_(dccut)::setCutOnFreq(fv3_float_t fc, fv3_float_t fs)
{
  fv3_float_t _fc = 2*fc/fs;
  gain = (std::sqrt(3.) - 2.*std::sin(M_PI*_fc))/(std::sin(M_PI*_fc) + std::sqrt(3.)*std::cos(M_PI*_fc));
}

fv3_float_t FV3_(dccut)::getCutOnFreq()
{
  return  std::atan(std::sqrt(3.)*(1.-gain*gain)/(1.+4.*gain+gain*gain))/M_PI;
}

fv3_float_t FV3_(dccut)::getCutOnFreq(fv3_float_t fs)
{
  return getCutOnFreq()*fs/2.;
}

// class lfo

FV3_(lfo)::FV3_(lfo)()
{
  mute();
}

FV3_(lfo)::~FV3_(lfo)()
{
  ;
}

void FV3_(lfo)::mute()
{
  arc_re = 1; arc_im = 0;
  re = 1; im = 0;
  count = 0; count_max = 1;
}

void FV3_(lfo)::setFreq(fv3_float_t freq, fv3_float_t fs)
{
  this->setFreq(freq/fs);
}

void FV3_(lfo)::setFreq(fv3_float_t fc)
{
  s_fc = fc;
  count_max = 1/fc + 1;
  fv3_float_t theta = 2*M_PI*fc;
  arc_re = std::cos(theta);
  arc_im = std::sin(theta);
}

#include "freeverb/fv3_ns_end.h"
