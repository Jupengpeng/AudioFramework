/**
 *  Enhanced Simple Tank Reverb
 *
 *  Copyright (C) 1977 David Griesinger
 *  Copyright (C) 1997 Jon Dattorro
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

#include "freeverb/strev.hpp"
#include "freeverb/fv3_type_float.h"
#include "freeverb/fv3_ns_start.h"

const long FV3_(strev)::allpCo[] = {142, 107, 379, 277, 672, 908, 1800, 2656,};
const long FV3_(strev)::delayCo[] = {4453, 3720, 4217, 3163,};
const long FV3_(strev)::idxLCo[] = {266, 2974, 1913, 1996, 1990, 187, 1066,};
const long FV3_(strev)::idxRCo[] = {353, 3627, 1228, 2673, 2111, 335, 121,};
const long FV3_(strev)::allpM_EXCURSION = 32;

FV3_(strev)::FV3_(strev)()
	       /*throw(std::bad_alloc)*/
{
  setCurrentFs(48000);
  setdiffusion1(0.7);
  setdiffusion2(0.5);
  setidiffusion1(0.75);
  setidiffusion2(0.62);
}

FV3_(strev)::FV3_(~strev)()
{
  freeWave();
}

void FV3_(strev)::mute()
{
  FV3_(revbase)::mute();
  dccut1.mute(), lpf_in.mute();
  for (long i = 0;i < FV3_STREV_NUM_ALLPASS_4;i ++) allpassC[i].mute();
  allpassM_23_24.mute(); delayC_63.mute(); delayC_30.mute(); lpfC_30.mute(); allpassC_31_33.mute();
  allpassM_46_48.mute(); delayC_39.mute(); delayC_54.mute(); lpfC_54.mute(); allpassC_55_59.mute();
  lfo1.mute(); lfo2.mute(); lfo1_lpf.mute(); lfo2_lpf.mute(); out1_lpf.mute(); out2_lpf.mute();
}

/*
  The following reverb algorithm and the allpass modulation filter's algorithm
  originate from the following article and was originally developed by David Griesinger.
  
  J. Dattorro, Effect Design Part 1: Reverberator and Other Filters (1997)
  J. Audio Eng. Soc., vol. 45, pp. 660-684, Sept. 1997.
  https://ccrma.stanford.edu/~dattorro/EffectDesignPart1.pdf

  J. Dattorro, Effect Design Part 2: Delay-Line Modulation and Chorus (1997)
  J. Audio Eng. Soc., vol. 45, pp. 764-788, Oct. 1997.
  http://ccrma.stanford.edu/~dattorro/EffectDesignPart2.pdf

  J. Dattorro, Effect Design: Part 3 Oscillators: Sinusoidal and Pseudonoise (2002)
  J. Audio Eng. Soc., vol. 50, pp. 115-146, Mar. 2002.
  http://ccrma.stanford.edu/~dattorro/EffectDesignPart3.pdf
*/

void FV3_(strev)::processreplace(fv3_float_t *inputL, fv3_float_t *inputR, fv3_float_t *outputL, fv3_float_t *outputR, long numsamples)
		    /*throw(std::bad_alloc)*/
{
  if(numsamples <= 0) return;
  long count = numsamples*SRC.getSRCFactor();
  //try{
	  growWave(count);
  //}
  //catch(std::bad_alloc)
  //{
//	  throw;
  //}

  fv3_float_t outL, outR, input, *origOutL = outputL, *origOutR = outputR;
  SRC.usrc(inputL, inputR, over.L, over.R, numsamples);
  inputL = over.L; inputR = over.R; outputL = overO.L; outputR = overO.R;

  while(count-- > 0)
    {
      UNDENORMAL(*inputL); UNDENORMAL(*inputR);
      outL = outR = 0.0;
      input = (*inputL + *inputR) / 2;

      // DC-cut HPF
      input = dccut1.process(input);
      // LPF
      input = lpf_in.process(input);
      // Feed through allpasses in series
      for(long i = 0;i < FV3_STREV_NUM_ALLPASS_4;i ++)
	input = allpassC[i].process(input);
      // Split into L/R part
      outL = input + delayC_63.getlast()*decay;
      outR = input + delayC_39.getlast()*decay;
      
      outL = allpassM_23_24.process(outL, lfo1_lpf.process(lfo1.process())*wander);
      outL = delayC_30.process(outL);
      outL = lpfC_30.process(outL);
      outL = allpassC_31_33.process(outL*decay);
      delayC_39.process(outL);
      
      outR = allpassM_46_48.process(outR, lfo2_lpf.process(lfo2.process())*wander*(-1));
      outR = delayC_54.process(outR);
      outR = lpfC_54.process(outR);
      outR = allpassC_55_59.process(outR*decay);
      delayC_63.process(outR);
      
      outL = delayC_54.get_z(iLC[0]) + delayC_54.get_z(iLC[1]) - allpassC_55_59.get_z(iLC[2]) + delayC_63.get_z(iLC[3])
	- delayC_30.get_z(iLC[4]) - allpassC_31_33.get_z(iLC[5]) - delayC_39.get_z(iLC[6]);
      outR = delayC_30.get_z(iRC[0]) + delayC_30.get_z(iRC[1]) - allpassC_31_33.get_z(iRC[2]) + delayC_39.get_z(iRC[3])
	- delayC_54.get_z(iRC[4]) - allpassC_55_59.get_z(iRC[5]) - delayC_63.get_z(iRC[6]);
      
      fv3_float_t fpL = delayWL.process(out1_lpf.process(outL));
      fv3_float_t fpR = delayWR.process(out2_lpf.process(outR));
      *outputL = fpL*wet1 + fpR*wet2 + delayL.process(*inputL)*dry;
      *outputR = fpR*wet1 + fpL*wet2 + delayR.process(*inputR)*dry;
      UNDENORMAL(*outputL); UNDENORMAL(*outputR);
      inputL ++; inputR ++; outputL ++; outputR ++;
    }
  SRC.dsrc(overO.L, overO.R, origOutL, origOutR, numsamples);
}

void FV3_(strev)::setroomsize(fv3_float_t value)
{
  rt60 = value;
  fv3_float_t back = rt60 * (fv3_float_t)currentfs * (fv3_float_t)SRC.getSRCFactor();
  UNDENORMAL(back);
  if(back > 0) decay = std::pow(10.0, -3 * (fv3_float_t)tankDelay / back);
  else decay = 0;
}

fv3_float_t FV3_(strev)::getroomsize()
{
  return rt60;
}

void FV3_(strev)::setdccutfreq(fv3_float_t value)
{
  dccutfq = value;
  dccut1.setCutOnFreq(dccutfq, currentfs*SRC.getSRCFactor());
}

fv3_float_t FV3_(strev)::getdccutfreq()
{
  return dccutfq;
}

void FV3_(strev)::setidiffusion1(fv3_float_t value)
{
  idiff1 = value;
  allpassC[0].setfeedback(-1*idiff1);
  allpassC[1].setfeedback(-1*idiff1);
}

fv3_float_t FV3_(strev)::getidiffusion1()
{
  return idiff1;
}

void FV3_(strev)::setidiffusion2(fv3_float_t value)
{
  idiff2 = value;
  allpassC[2].setfeedback(-1*idiff2);
  allpassC[3].setfeedback(-1*idiff2);
}

fv3_float_t FV3_(strev)::getidiffusion2()
{
  return idiff2;
}

void FV3_(strev)::setdiffusion1(fv3_float_t value)
{
  diff1 = value;
  allpassC_31_33.setfeedback(-1*diff1);
  allpassC_55_59.setfeedback(-1*diff1);
}

fv3_float_t FV3_(strev)::getdiffusion1()
{
  return diff1;
}

void FV3_(strev)::setdiffusion2(fv3_float_t value)
{
  diff2 = value;
  allpassM_23_24.setfeedback(diff2);
  allpassM_46_48.setfeedback(diff2);
}

fv3_float_t FV3_(strev)::getdiffusion2()
{
  return diff1;
}

void FV3_(strev)::setspin(fv3_float_t value)
{
  long factor = SRC.getSRCFactor();
  if(value < 0) value = 0;
  if(value > currentfs*factor/2) value = currentfs*factor/2;
  spin = value;
  if(spin > 0)
    {
      lfo1.setFreq(spin,          currentfs*factor);
      lfo2.setFreq(spin+spindiff, currentfs*factor);
    }
  else
    {
      lfo1.setFreq(0); lfo1.setFreq(0);
    }
}

fv3_float_t FV3_(strev)::getspin()
{
  return spin;
}


void FV3_(strev)::setspindiff(fv3_float_t value)
{
  long factor = SRC.getSRCFactor();
  if(value < 0) value = 0;
  if(value > currentfs*factor/2) value = currentfs*factor/2;
  spindiff = value;
  setspin(getspin());
}

fv3_float_t FV3_(strev)::getspindiff()
{
  return spindiff;
}

void FV3_(strev)::setspinlimit(fv3_float_t value)
{
  long factor = SRC.getSRCFactor();
  if(value < 0) value = 0;
  if(value > currentfs*factor/2) value = currentfs*factor/2;
  spinlimit = value;
  lfo1_lpf.setLPFA(spinlimit, currentfs*factor);
  lfo2_lpf.setLPFA(spinlimit, currentfs*factor);
}

fv3_float_t FV3_(strev)::getspinlimit()
{
  return spinlimit;
}

void FV3_(strev)::setwander(fv3_float_t value)
{
  if(value < 0) value = 0;
  if(value > 1) value = 1;
  wander = value;
}

fv3_float_t FV3_(strev)::getwander()
{
  return wander;
}

void FV3_(strev)::setinputdamp(fv3_float_t value)
{
  long factor = SRC.getSRCFactor();
  if(value > factor*currentfs/2) value = factor*currentfs/2;
  inputdamp = value;
  lpf_in.setLPFA(inputdamp, factor*currentfs);
}

fv3_float_t FV3_(strev)::getinputdamp()
{
  return inputdamp;
}

void FV3_(strev)::setdamp(fv3_float_t value)
{
  long factor = SRC.getSRCFactor();
  if(value > factor*currentfs/2) value = factor*currentfs/2;
  damp = value;
  lpfC_30.setLPFA(damp, factor*currentfs);
  lpfC_54.setLPFA(damp, factor*currentfs);
}

fv3_float_t FV3_(strev)::getdamp()
{
  return damp;
}

void FV3_(strev)::setoutputdamp(fv3_float_t value)
{
  long factor = SRC.getSRCFactor();
  if(value < 0) value = 0;
  if(value > factor*currentfs/2) value = factor*currentfs/2;
  outputdamp = value;
  out1_lpf.setLPFA(outputdamp, factor*currentfs);
  out2_lpf.setLPFA(outputdamp, factor*currentfs);
}

fv3_float_t FV3_(strev)::getoutputdamp()
{
  return outputdamp;
}

void FV3_(strev)::setFsFactors(long fs, long factor)
{
  fv3_float_t totalFactor = (fv3_float_t)fs*(fv3_float_t)factor/(fv3_float_t)FV3_STREV_DEFAULT_FS;
  for(long i = 0;i < FV3_STREV_NUM_ALLPASS_4;i ++) allpassC[i].setsize(f_(allpCo[i],totalFactor));
  allpassM_23_24.setsize(f_(allpCo[4],totalFactor), f_(allpM_EXCURSION,totalFactor));
  allpassM_46_48.setsize(f_(allpCo[5],totalFactor), f_(allpM_EXCURSION,totalFactor));
  allpassC_31_33.setsize(f_(allpCo[6],totalFactor));
  allpassC_55_59.setsize(f_(allpCo[7],totalFactor));
  delayC_30.setsize(f_(delayCo[0],totalFactor));
  delayC_39.setsize(f_(delayCo[1],totalFactor));
  delayC_54.setsize(f_(delayCo[2],totalFactor));
  delayC_63.setsize(f_(delayCo[3],totalFactor));
  for(long i = 0;i < FV3_STREV_NUM_INDEX;i ++)
    {
      iLC[i] = f_(idxLCo[i],totalFactor);
      iRC[i] = f_(idxRCo[i],totalFactor);
    }
  tankDelay = (delayC_30.getsize()+delayC_39.getsize()+delayC_54.getsize()+delayC_63.getsize())/4;
  setroomsize(getroomsize());
  setidiffusion1(getidiffusion1());
  setidiffusion2(getidiffusion2());
  setdiffusion1(getdiffusion1());
  setdiffusion2(getdiffusion2());
  setdamp(getdamp());
  setwander(getwander());
  setspin(getspin());
  setspindiff(getspindiff());
  setdccutfreq(getdccutfreq());
  mute();
}

#include "freeverb/fv3_ns_end.h"
