/**
 *  Allpass filter
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

#include "freeverb/allpass.hpp"
#include "freeverb/fv3_type_float.h"
#include "freeverb/fv3_ns_start.h"

// simple allpass filter

FV3_(allpass)::FV3_(allpass)()
{
  bufsize = bufidx = 0;
  buffer = NULL;
}

FV3_(allpass)::FV3_(~allpass)()
{
  if(bufsize != 0) delete[] buffer;
}

long FV3_(allpass)::getsize()
{
  return bufsize;
}

void FV3_(allpass)::setsize(long size)
                   /*throw(std::bad_alloc)*/
{
  if(size < 0) size = 0;
  if(bufsize != 0) delete[] buffer;
  if(size > 0)
    {
	  //modify by bin.liu
      //try
      //  {
          buffer = new fv3_float_t[size];
      //  }
     // catch(.../*std::bad_alloc*/)
      //  {
      //    std::fprintf(stderr, "allpass::setsize(%ld) bad_alloc\n", size);
       //   delete[] buffer;
      //    bufsize = 0;
       //   throw;
       // }
    }
  bufsize = size;
  bufidx = 0;
}

void FV3_(allpass)::mute()
{
  if(buffer == NULL||bufsize == 0) return;
  FV3_(utils)::mute(buffer, bufsize);
}

void FV3_(allpass)::setfeedback(fv3_float_t val) 
{
  feedback = val;
}

fv3_float_t FV3_(allpass)::getfeedback() 
{
  return feedback;
}

// modulated allpass filter

FV3_(allpassm)::FV3_(allpassm)()
{
  bufsize = readidx = writeidx = delaysize = modulationsize = 0;
  buffer = NULL;
}

FV3_(allpassm)::FV3_(~allpassm)()
{
  if(bufsize != 0) delete[] buffer;
}

long FV3_(allpassm)::getsize()
{
  return bufsize;
}

long FV3_(allpassm)::getdelaysize()
{
  return delaysize;
}

long FV3_(allpassm)::getmodulationsize()
{
  return modulationsize;
}

void FV3_(allpassm)::setsize(long size)
		   /*throw(std::bad_alloc)*/
{
  setsize(size, 0);
}

void FV3_(allpassm)::setsize(long size, long modsize)
		   /*throw(std::bad_alloc)*/
{
  if(size < 0) size = 0; if(modsize < 0) modsize = 0;
  if(modsize > size) modsize = size;
  if(bufsize != 0) delete[] buffer;
  if(size+modsize > 0)
    {

	   buffer = new fv3_float_t[size+modsize];
      /* modify by bin.liu
      try
	{
	  buffer = new fv3_float_t[size+modsize];
	}
      catch(std::bad_alloc)
	{
	  std::fprintf(stderr, "allpassm::setsize(%ld) bad_alloc\n", size+modsize);
	  delete[] buffer;
	  bufsize = 0;
	  throw;
	}*/
    }
  bufsize = size+modsize;
  writeidx = 0;
  readidx = modsize*2;
  delaysize = size;
  modulationsize = modsize;
}

void FV3_(allpassm)::mute()
{
  if(buffer == NULL||bufsize == 0) return;
  FV3_(utils)::mute(buffer, bufsize);
  z_1 = 0;
}

void FV3_(allpassm)::setfeedback(fv3_float_t val) 
{
  feedback = val;
}

fv3_float_t FV3_(allpassm)::getfeedback() 
{
  return feedback;
}

void FV3_(allpassm)::set_90degfq(fv3_float_t fc, fv3_float_t fs)
{
  fv3_float_t tant = std::tan(M_PI*fc/fs);
  feedback = (tant-1)/(tant+1);
}

#include "freeverb/fv3_ns_end.h"
