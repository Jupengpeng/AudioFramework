/**
 *  Reverb Base Class
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

#include "freeverb/revbase.hpp"
#include "freeverb/fv3_type_float.h"
#include "freeverb/fv3_ns_start.h"

FV3_(revbase)::FV3_(revbase)()
	      throw(std::bad_alloc)
{
  wet_scale = dry_scale = 1;
  primeMode = true;
}

FV3_(revbase)::FV3_(~revbase)()
{
  freeWave();
}

void FV3_(revbase)::setInitialDelay(long numsamples)
{
  initialDelay = numsamples;
  if(initialDelay >= 0)
    {
#ifdef DEBUG
      std::fprintf(stderr, "revbase::setInitialDelay(%ld) delayW(%ld))\n", numsamples, initialDelay);
#endif
      delayL.setsize(0);
      delayR.setsize(0);
      delayWL.setsize(initialDelay);
      delayWR.setsize(initialDelay);
    }
  else
    {
      long dryD = - initialDelay;
#ifdef DEBUG
      std::fprintf(stderr, "revbase::setInitialDelay(%ld) delayD(%ld))\n", numsamples, dryD);
#endif
      delayL.setsize(dryD);
      delayR.setsize(dryD);
      delayWL.setsize(0);
      delayWR.setsize(0);
    }
}

long FV3_(revbase)::getInitialDelay()
{
  return initialDelay;
}

void FV3_(revbase)::setPreDelay(fv3_float_t value_ms)
{
  long iDelay = (long)((fv3_float_t)currentfs*value_ms/1000.0f);
  setInitialDelay(iDelay);
}

fv3_float_t FV3_(revbase)::getPreDelay()
{
  return (fv3_float_t)initialDelay*1000.0f/(fv3_float_t)currentfs;
}


long FV3_(revbase)::getLatency()
{
  return SRC.getLatency();
}

void FV3_(revbase)::printconfig()
{
  std::fprintf(stderr, "*** Revbase config ***\n");
  std::fprintf(stderr, "Fs = %ld[Hz] X %ld\n",currentfs,SRC.getSRCFactor());
  std::fprintf(stderr, "Wet %f Dry %f Width %f\n", (double)wet, (double)dry, (double)width);
}

void FV3_(revbase)::mute()
{
  over.mute(); overO.mute();
  delayL.mute(); delayR.mute();
  delayWL.mute(); delayWR.mute();
}


void FV3_(revbase)::growWave(long size)
		throw(std::bad_alloc)
{
  if(size > over.getsize())
    {
      FV3_(revbase)::freeWave();
      try
	{
	  over.alloc(size, 2);
	  overO.alloc(size,2);
	}
      catch(std::bad_alloc)
	{
	  std::fprintf(stderr, "revbase::growWave(%ld): bad_alloc", size);
	  FV3_(revbase)::freeWave();
	  throw;
	}
    }
}

void FV3_(revbase)::freeWave()
{
  over.free();
  overO.free();
}

void FV3_(revbase)::setwet(fv3_float_t value)
{
  wetDB = value;
  wet = FV3_(utils)::dB2R(value)*wet_scale;
  update_wet();

}
fv3_float_t FV3_(revbase)::getwet()
{
  return wet;
}

void FV3_(revbase)::setdry(fv3_float_t value)
{
  dryDB = value;
  dry = FV3_(utils)::dB2R(value)*dry_scale;
}

fv3_float_t FV3_(revbase)::getdry()
{
  return dry;
}

void FV3_(revbase)::setwidth(fv3_float_t value)
{
  width = value;
  update_wet();
}

fv3_float_t FV3_(revbase)::getwidth()
{
  return width;
}

void FV3_(revbase)::update_wet()
{
  wet1 = wet*(width/2 + 0.5f);
  wet2 = wet*((1-width)/2);
}

long FV3_(revbase)::getCurrentFs()
{
  return currentfs;
}

void FV3_(revbase)::setCurrentFs(long fs)
		    throw(std::bad_alloc)
{
  setFsFactors((currentfs = fs), SRC.getSRCFactor());
}

long FV3_(revbase)::getOverSamplingFactor()
{
  return SRC.getSRCFactor();
}

void FV3_(revbase)::setOverSamplingFactor(long factor)
{
  setOverSamplingFactor(factor, SRC_SINC_FASTEST);
}

void FV3_(revbase)::setOverSamplingFactor(long factor, long converter_type)
{
  SRC.setSRCFactor(factor, converter_type);
  setFsFactors(currentfs, factor);
}

long FV3_(revbase)::f_(long def, fv3_float_t factor)
{
  return (long)((fv3_float_t)(def)*factor);
}

void FV3_(revbase)::setPrimeMode(bool value)
{
  primeMode = value;
}

bool FV3_(revbase)::getPrimeMode()
{
  return primeMode;
}

long FV3_(revbase)::p_(long def, fv3_float_t factor)
{
  long base = f_(def,factor);
  if(primeMode)
    {
      while(!FV3_(utils)::isPrime(base)) base++;
    }
  return base;
}

#include "freeverb/fv3_ns_end.h"
