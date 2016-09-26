/**
 *  CCRMA NRev
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

#include "freeverb/nrev.hpp"
#include "freeverb/fv3_type_float.h"
#include "freeverb/fv3_ns_start.h"

const long FV3_(nrev)::combCo[] =    {1433, 1601, 1867, 2053, 2251, 2399,};
const long FV3_(nrev)::allpassCo[] = {347, 113, 37, 59, 53, 43, 37, 29, 19,};
//                                   (  0    1   2) F3  R4  L5  R6 RL7 RR8

FV3_(nrev)::FV3_(nrev)()
	   /*throw(std::bad_alloc)*/
{
  feedback = 0.7f;
  damp2 = 1; damp2_1 = 1 - damp2;
  damp3 = 1; damp3_1 = 1 - damp3;
  dccutfq = 8;
  setwetrear(-10);
  setCurrentFs(48000);
  wet_scale = FV3_NREV_SCALE_WET;
}

FV3_(nrev)::FV3_(~nrev)()
{
  ;
}

void FV3_(nrev)::setRearDelay(long numsamples)
{
  if(initialDelay >= 0)
    {
      rearDelay = numsamples;
      delayRearL.setsize(0);
      delayRearR.setsize(0);
    }
  delayRearL.mute();
  delayRearR.mute();
}

long FV3_(nrev)::getRearDelay()
{
  return rearDelay;
}


void FV3_(nrev)::printconfig()
{
  std::fprintf(stderr, "*** NRev config ***\n");
  std::fprintf(stderr, "Fs=%ld[Hz] x %ld\n", currentfs,SRC.getSRCFactor());
  std::fprintf(stderr, "roomsize %f\n", (double)roomsize);
  std::fprintf(stderr, "damp %f damp2 %f damp3 %f wet %f wet1 %f wet2 %f\n",
	       (double)damp, (double)damp2, (double)damp3, (double)wet, (double)wet1, (double)wet2);
  std::fprintf(stderr, "dry %f width %f\n", (double)dry, (double)width);
}

void FV3_(nrev)::mute()
{
  FV3_(revbase)::mute();
  for (long i = 0;i < FV3_NREV_NUM_COMB;i ++)
    {
      combL[i].mute(); combR[i].mute();
    }
  for (long i = 0;i < FV3_NREV_NUM_ALLPASS;i ++)
    {
      allpassL[i].mute(); allpassR[i].mute();
    }
  overORear.mute();
  hpf = lpfL = lpfR = 0;
  inDCC.mute(); lLDCC.mute(); lRDCC.mute();
}

void FV3_(nrev)::growWave(long size)
		/*throw(std::bad_alloc)*/
{
  if(size > over.getsize())
    {
      freeWave();
     // try
	//{
	  FV3_(revbase)::growWave(size);
	  overORear.alloc(size,2);
	//}
     // catch(std::bad_alloc)
	//{
	 // std::fprintf(stderr, "nrev::growWave(%ld): bad_alloc", size);
	 // freeWave();
	 // throw;
	//}
    }
}

void FV3_(nrev)::freeWave()
{
  FV3_(revbase)::freeWave();
  overORear.free();
}

void FV3_(nrev)::processreplace(fv3_float_t *inputL, fv3_float_t *inputR, fv3_float_t *outputL, fv3_float_t *outputR, long numsamples)
		/*throw(std::bad_alloc)*/
{
  processreplace(inputL,inputR,outputL,outputR,NULL,NULL,numsamples);
}

void FV3_(nrev)::processreplace(fv3_float_t *inputL, fv3_float_t *inputR, fv3_float_t *outputL, fv3_float_t *outputR,
				fv3_float_t *outputRearL, fv3_float_t *outputRearR, long numsamples)
		/*throw(std::bad_alloc)*/
{
  if(numsamples <= 0) return;
  long count = numsamples*SRC.getSRCFactor();
  //try{
	  growWave(count);
  //}
 // catch(std::bad_alloc)
 // {
//	  throw;
  //}
  SRC.usrc(inputL, inputR, over.L, over.R, numsamples);
  if(outputRearL == NULL||outputRearR == NULL)
    {
      processloop2(count, over.L, over.R, overO.L, overO.R);
      SRC.dsrc(overO.L, overO.R, outputL, outputR, numsamples);
    }
  else
    {
      processloop4(count, over.L, over.R, overO.L, overO.R, overORear.L, overORear.R);
      SRC.dsrc(overO.L, overO.R, outputL, outputR, numsamples);
      SRCRear.dsrc(overORear.L, overORear.R, outputRearL, outputRearR, numsamples);
    }
}

void FV3_(nrev)::processloop2(long count, fv3_float_t *inputL, fv3_float_t *inputR, fv3_float_t *outputL, fv3_float_t *outputR)
{
  fv3_float_t outL, outR;
  while(count-- > 0)
    {
      outL = outR = 0; UNDENORMAL(*inputL); UNDENORMAL(*inputR);
      hpf = damp3_1*inDCC.process(*inputL + *inputR) - damp3*hpf; UNDENORMAL(hpf);
      
      for(long i = 0;i < FV3_NREV_NUM_COMB;i ++) outL += combL[i].process(hpf);
      for(long i = 0;i < 3;i ++) outL = allpassL[i].process_ov(outL);
      lpfL = damp2*lpfL + damp2_1*outL; UNDENORMAL(lpfL);
      outL = allpassL[3].process_ov(lpfL);
      outL = allpassL[5].process_ov(outL);
      outL = lLDCC.process(outL);
      outL = delayWL.process(outL);
      
      for(long i = 0;i < FV3_NREV_NUM_COMB;i ++) outR += combR[i].process(hpf);
      for(long i = 0;i < 3;i ++) outR = allpassR[i].process_ov(outR);
      lpfR = damp2*lpfR + damp2_1*outR; UNDENORMAL(lpfR);
      outR = allpassR[3].process_ov(lpfR);
      outR = allpassL[6].process_ov(outR);
      outR = lRDCC.process(outR);
      outR = delayWR.process(outR);
      
      *outputL = outL*wet1 + outR*wet2 + delayL.process(*inputL)*dry;
      *outputR = outR*wet1 + outL*wet2 + delayR.process(*inputR)*dry;
      inputL ++; inputR ++; outputL ++; outputR ++;
    }
}

void FV3_(nrev)::processloop4(long count, fv3_float_t *inputL, fv3_float_t *inputR, fv3_float_t *outputL, fv3_float_t *outputR,
			      fv3_float_t *outRearL, fv3_float_t *outRearR)
{
  fv3_float_t outL, outR;
  while(count-- > 0)
    {
      outL = outR = 0;
      UNDENORMAL(*inputL); UNDENORMAL(*inputR);
      hpf = damp3_1*inDCC.process(*inputL + *inputR) - damp3*hpf; UNDENORMAL(hpf);
      
      for(long i = 0;i < FV3_NREV_NUM_COMB;i ++) outL += combL[i].process(hpf);
      for(long i = 0;i < 3;i ++) outL = allpassL[i].process_ov(outL);
      lpfL = damp2*lpfL + damp2_1*outL; UNDENORMAL(lpfL);
      outL = allpassL[3].process_ov(lpfL);
      *outRearL = allpassL[4].process_ov(outL);
      outL = allpassL[5].process_ov(outL);
      outL = lLDCC.process(outL);
      outL = delayWL.process(outL);
      
      *outRearL = allpassL[7].process_ov(*outRearL)*wetRear;
      *outRearL = delayRearL.process(*outRearL);
      outRearL ++;

      for(long i = 0;i < FV3_NREV_NUM_COMB;i ++) outR += combR[i].process(hpf);
      for(long i = 0;i < 3;i ++) outR = allpassR[i].process_ov(outR);
      lpfR = damp2*lpfR + damp2_1*outR; UNDENORMAL(lpfR);
      outR = allpassR[3].process_ov(lpfR);
      *outRearR = allpassR[4].process_ov(outR);
      outR = allpassL[6].process_ov(outR);
      outR = lRDCC.process(outR);
      outR = delayWR.process(outR);
      
      *outRearR = allpassL[8].process_ov(*outRearR)*wetRear;
      *outRearR = delayRearR.process(*outRearR);
      outRearR ++;
            
      *outputL = outL*wet1 + outR*wet2 + delayL.process(*inputL)*dry;
      *outputR = outR*wet1 + outL*wet2 + delayR.process(*inputR)*dry;
      inputL ++; inputR ++; outputL ++; outputR ++;
    }
}

void FV3_(nrev)::setroomsize(fv3_float_t value)
{
  roomsize = value;
  fv3_float_t back = roomsize * (fv3_float_t)currentfs * (fv3_float_t)SRC.getSRCFactor();
  UNDENORMAL(back);
  if(back > 0)
    {
      for(long i = 0;i < FV3_NREV_NUM_COMB;i ++)
	{
	  combL[i].setfeedback(std::pow(10.0, -3 * (fv3_float_t)combL[i].getsize() / back));
	  combR[i].setfeedback(std::pow(10.0, -3 * (fv3_float_t)combR[i].getsize() / back));
	}
    }
  else
    {
      for(long i = 0;i < FV3_NREV_NUM_COMB;i ++)
	{
	  combL[i].setfeedback(0);
	  combR[i].setfeedback(0);
	}
    }
}

fv3_float_t FV3_(nrev)::getroomsize()
{
  return roomsize;
}

void FV3_(nrev)::setfeedback(fv3_float_t value)
{
  feedback = value;
  for(long i = 0;i < FV3_NREV_NUM_ALLPASS;i ++)
    {
      allpassL[i].setfeedback(value);
      allpassR[i].setfeedback(value);
    }
}

fv3_float_t FV3_(nrev)::getfeedback()
{
  return feedback;
}

void FV3_(nrev)::setdamp(fv3_float_t value)
{
  damp = value;
  for(long i = 0;i < FV3_NREV_NUM_COMB;i ++)
    {
      combL[i].setdamp(damp);
      combR[i].setdamp(damp);
    }
}

fv3_float_t FV3_(nrev)::getdamp()
{
  return damp;
}

void FV3_(nrev)::setdamp2(fv3_float_t value)
{
  damp2 = value;
  damp2_1 = 1 - value;
}

fv3_float_t FV3_(nrev)::getdamp2()
{
  return damp2;
}

void FV3_(nrev)::setdamp3(fv3_float_t value)
{
  damp3 = value;
  damp3_1 = 1 - value;
}

fv3_float_t FV3_(nrev)::getdamp3()
{
  return damp3;
}

void FV3_(nrev)::setwetrear(fv3_float_t value)
{
  wetRearReal = value;
  wetRear = FV3_(utils)::dB2R(value)*FV3_NREV_SCALE_WET;
}

fv3_float_t FV3_(nrev)::getwetrear()
{
  return wetRearReal;
}

void FV3_(nrev)::setOverSamplingFactor(long factor, long converter_type)
{
  FV3_(revbase)::setOverSamplingFactor(factor, converter_type);
  SRCRear.setSRCFactor(factor, converter_type);
  setFsFactors(currentfs, factor);
}

void FV3_(nrev)::setFsFactors(long fs, long factor)
{
  fv3_float_t totalFactor = (fv3_float_t)factor*(fv3_float_t)fs/(fv3_float_t)FV3_NREV_DEFAULT_FS;
  long stereoSpread = f_(FV3_NREV_STEREO_SPREAD, totalFactor);
  for(long i = 0;i < FV3_NREV_NUM_COMB;i ++)
    {
      combL[i].setsize(p_(combCo[i],totalFactor));
      combR[i].setsize(p_(f_(combCo[i],totalFactor)+stereoSpread,1));
    }
  for(long i = 0;i < FV3_NREV_NUM_ALLPASS;i ++)
    {
      allpassL[i].setsize(p_(allpassCo[i],totalFactor));
      allpassR[i].setsize(p_(f_(allpassCo[i],totalFactor)+stereoSpread,1));
    }
  setroomsize(getroomsize());
  inDCC.setCutOnFreq(dccutfq,fs*factor);
  lLDCC.setCutOnFreq(dccutfq,fs*factor);
  lRDCC.setCutOnFreq(dccutfq,fs*factor);
  mute();
}

#include "freeverb/fv3_ns_end.h"
