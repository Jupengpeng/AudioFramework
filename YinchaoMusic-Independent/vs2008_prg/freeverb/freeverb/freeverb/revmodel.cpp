/**
 *  Enhanced Freeverb
 *
 *  Copyright (C) 2000 Jezar at Dreampoint
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

#include "freeverb/revmodel.hpp"
#include "freeverb/fv3_type_float.h"
#include "freeverb/fv3_ns_start.h"

const long FV3_(revmodel)::combCo[] = {1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617,};
const long FV3_(revmodel)::allpCo[] = {556, 441, 341, 225,};

FV3_(revmodel)::FV3_(revmodel)()
	       throw(std::bad_alloc)
{
  setCurrentFs(44100);
}

FV3_(revmodel)::FV3_(~revmodel)()
{
  freeWave();
}

void FV3_(revmodel)::printconfig()
{
  std::fprintf(stderr, "*** Freeverb config ***\n");
  std::fprintf(stderr, "Fs=%ld[Hz] x %ld\n",currentfs,SRC.getSRCFactor());
  std::fprintf(stderr, "roomsize %f damp %f wet %f wet1 %f wet2 %f\n",
	       (double)roomsize, (double)damp, (double)wet, (double)wet1, (double)wet2);
  std::fprintf(stderr, "dry %f width %f\n", (double)dry, (double)width);
}

void FV3_(revmodel)::mute()
{
  FV3_(revbase)::mute();
  for (long i = 0;i < FV3_FREV_NUM_COMB;i ++)
    {
      combL[i].mute();
      combR[i].mute();
    }
  for (long i = 0;i < FV3_FREV_NUM_ALLPASS;i ++)
    {
      allpassL[i].mute();
      allpassR[i].mute();
    }
}

void FV3_(revmodel)::processreplace(fv3_float_t *inputL, fv3_float_t *inputR, fv3_float_t *outputL, fv3_float_t *outputR, long numsamples)
		    throw(std::bad_alloc)
{
  if(numsamples <= 0) return;
  long count = numsamples*SRC.getSRCFactor();
  try
    {
      growWave(count);
    }
  catch(std::bad_alloc)
    {
      throw;
    }

  fv3_float_t outL, outR, input, *origOutL = outputL, *origOutR = outputR;
  SRC.usrc(inputL, inputR, over.L, over.R, numsamples);
  inputL = over.L; inputR = over.R; outputL = overO.L; outputR = overO.R;

  while(count-- > 0)
    {
      UNDENORMAL(*inputL); UNDENORMAL(*inputR);
      outL = outR = 0.0;
      input = (*inputL + *inputR) * FV3_FREV_FIXED_GAIN;

      // Accumulate comb filters in parallel
      for(long i = 0;i < FV3_FREV_NUM_COMB;i ++)
	{
	  outL += combL[i].process(input);
	  outR += combR[i].process(input);
	}
      
      // Feed through allpasses in series
      for(long i = 0;i < FV3_FREV_NUM_ALLPASS;i ++)
	{
	  outL = allpassL[i].process_ov(outL);
	  outR = allpassR[i].process_ov(outR);
	}

      fv3_float_t fpL = delayWL.process(outL);
      fv3_float_t fpR = delayWR.process(outR);
      *outputL = fpL*wet1 + fpR*wet2 + delayL.process(*inputL)*dry;
      *outputR = fpR*wet1 + fpL*wet2 + delayR.process(*inputR)*dry;
      UNDENORMAL(*outputL); UNDENORMAL(*outputR);
      inputL ++; inputR ++; outputL ++; outputR ++;
    }
  SRC.dsrc(overO.L, overO.R, origOutL, origOutR, numsamples);
}

void FV3_(revmodel)::setroomsize(fv3_float_t value)
{
  roomsize = (value*FV3_FREV_SCALE_ROOM) + FV3_FREV_OFFSET_ROOM;
  for(long i = 0;i < FV3_FREV_NUM_COMB;i ++)
    {
      combL[i].setfeedback(roomsize);
      combR[i].setfeedback(roomsize);
    }
}

fv3_float_t FV3_(revmodel)::getroomsize()
{
  return (roomsize-FV3_FREV_OFFSET_ROOM)/FV3_FREV_SCALE_ROOM;
}

void FV3_(revmodel)::setdamp(fv3_float_t value)
{
  damp = value;
  for(long i = 0;i < FV3_FREV_NUM_COMB;i ++)
    {
      combL[i].setdamp(damp);
      combR[i].setdamp(damp);
    }
}

fv3_float_t FV3_(revmodel)::getdamp()
{
  return damp;
}

void FV3_(revmodel)::setwet(fv3_float_t value)
{
  wet = value*FV3_FREV_SCALE_WET;
  update_wet();
}

fv3_float_t FV3_(revmodel)::getwet()
{
  return wet/FV3_FREV_SCALE_WET;
}

void FV3_(revmodel)::setdry(fv3_float_t value)
{
  dry = value*FV3_FREV_SCALE_DRY;
}

fv3_float_t FV3_(revmodel)::getdry()
{
  return dry/FV3_FREV_SCALE_DRY;
}

void FV3_(revmodel)::setFsFactors(long fs, long factor)
{
  fv3_float_t totalFactor = (fv3_float_t)fs*(fv3_float_t)factor/(fv3_float_t)FV3_FREV_DEFAULT_FS;
  for(long i = 0;i < FV3_FREV_NUM_ALLPASS;i ++)
    {
      allpassL[i].setsize(f_(allpCo[i],totalFactor));
      allpassR[i].setsize(f_(allpCo[i]+FV3_FREV_STEREO_SPREAD441,totalFactor));
    }
  for(long i = 0;i < FV3_FREV_NUM_COMB;i ++)
    {
      combL[i].setsize(f_(combCo[i],totalFactor));
      combR[i].setsize(f_(combCo[i]+FV3_FREV_STEREO_SPREAD441,totalFactor));
    }
  setAllpassFeedback(FV3_FREV_ALLPASS_FEEDBACK);
  setdamp(getdamp());
  setroomsize(getroomsize());
  mute();
}

void FV3_(revmodel)::setAllpassFeedback(fv3_float_t fb)
{
  for(long i = 0;i < FV3_FREV_NUM_ALLPASS;i ++)
    {
      allpassL[i].setfeedback(fb);
      allpassR[i].setfeedback(fb);
    }
}

#include "freeverb/fv3_ns_end.h"
