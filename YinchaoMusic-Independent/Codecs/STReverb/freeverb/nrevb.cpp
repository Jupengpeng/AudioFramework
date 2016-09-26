/*  CCRMA NRev v2 (nrevb)
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

#include "freeverb/nrevb.hpp"
#include "freeverb/fv3_type_float.h"
#include "freeverb/fv3_ns_start.h"

FV3_(nrevb)::FV3_(nrevb)()
	    /*throw(std::bad_alloc)*/
{
  dinput = 0;
  apfeedback = 0.2;
}

FV3_(nrevb)::FV3_(~nrevb)()
{
  ;
}

void FV3_(nrevb)::mute()
{
  FV3_(nrev)::mute();
  dinput = 0;
}

void FV3_(nrevb)::processloop2(long count, fv3_float_t *inputL, fv3_float_t *inputR, fv3_float_t *outputL, fv3_float_t *outputR)
{
  fv3_float_t outL, outR, apL, apR;
  while(count-- > 0)
    {
      outL = outR = 0;
      hpf = damp3_1*dinput - damp3*hpf; UNDENORMAL(hpf);
      
      // Accumulate comb filters in parallel and Feed through allpasses in series
      for(long i = 0;i < FV3_NREV_NUM_COMB;i ++) outL += combL[i].process(hpf);
      for(long i = 0;i < 3;i ++) outL = allpassL[i].process(outL/2);
      lpfL = damp2*lpfL + damp2_1*outL; UNDENORMAL(lpfL);
      outL = allpassL[3].process(lpfL/2); outL /= 2;
      outL = allpassL[5].process(outL);
      outL = lLDCC.process(outL);
      apL = outL;
      outL = 512*delayWL.process(outL);
      
      for(long i = 0;i < FV3_NREV_NUM_COMB;i ++) outR += combR[i].process(hpf);
      for(long i = 0;i < 3;i ++) outR = allpassR[i].process(outR/2);
      lpfR = damp2*lpfR + damp2_1*outR; UNDENORMAL(lpfR);
      outR = allpassR[3].process(lpfR/2); outR /= 2;
      outR = allpassL[6].process(outR);
      outR = lRDCC.process(outR);
      apR = outR;
      outR = 512*delayWR.process(outR);
      
      dinput = ((*inputL + *inputR) + apfeedback * (apL + apR));
      dinput = inDCC.process(dinput);
      
      *outputL = outL*wet1 + outR*wet2 + delayL.process(*inputL)*dry;
      *outputR = outR*wet1 + outL*wet2 + delayR.process(*inputR)*dry;
      inputL ++; inputR ++; outputL ++; outputR ++;
    }
}

void FV3_(nrevb)::processloop4(long count, fv3_float_t *inputL, fv3_float_t *inputR, fv3_float_t *outputL, fv3_float_t *outputR,
			       fv3_float_t *outRearL, fv3_float_t *outRearR)
{
  fv3_float_t outL, outR, apL, apR;
  while(count-- > 0)
    {
      outL = outR = 0;
      hpf = damp3_1*dinput - damp3*hpf; UNDENORMAL(hpf);
      
      for(long i = 0;i < FV3_NREV_NUM_COMB;i ++) outL += combL[i].process(hpf);
      for(long i = 0;i < 3;i ++) outL = allpassL[i].process(outL/2);
      lpfL = damp2*lpfL + damp2_1*outL; UNDENORMAL(lpfL);
      outL = allpassL[3].process(lpfL/2); outL /= 2; *outRearL = allpassL[4].process(outL);
      outL = allpassL[5].process(outL);
      outL = lLDCC.process(outL);
      apL = outL;
      outL = 512*delayWL.process(outL);

      *outRearL = allpassL[7].process(*outRearL/2)*wetRear;
      *outRearL = 512*delayRearL.process(*outRearL);
      outRearL ++;
      
      for(long i = 0;i < FV3_NREV_NUM_COMB;i ++) outR += combR[i].process(hpf);
      outR = lRDCC.process(outR);
      for(long i = 0;i < 3;i ++) outR = allpassR[i].process(outR/2);
      lpfR = damp2*lpfR + damp2_1*outR; UNDENORMAL(lpfR);
      outR = allpassR[3].process(lpfR/2); outR /= 2; *outRearR = allpassR[4].process(outR);
      outR = allpassL[6].process(outR);
      outR = lRDCC.process(outR);
      apR = outR;
      outR = 512*delayWR.process(outR);
      
      *outRearR = allpassL[8].process(*outRearR/2)*wetRear;
      *outRearR = 512*delayRearR.process(*outRearR);
      outRearR ++;
      
      dinput = ((*inputL + *inputR) + apfeedback * (apL + apR));
      dinput = inDCC.process(dinput);
            
      *outputL = outL*wet1 + outR*wet2 + delayL.process(*inputL)*dry;
      *outputR = outR*wet1 + outL*wet2 + delayR.process(*inputR)*dry;
      inputL ++; inputR ++; outputL ++; outputR ++;
    }
}

#include "freeverb/fv3_ns_end.h"
