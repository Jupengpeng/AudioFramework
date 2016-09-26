/**
 *  Comb (delay) filter with LPF
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

#include "freeverb/comb.hpp"
#include "freeverb/fv3_type_float.h"
#include "freeverb/fv3_ns_start.h"

// simple comb filter

FV3_(comb)::FV3_(comb)()
{
  bufsize = bufidx = 0;
  buffer = NULL;
  setdamp(0);
}

FV3_(comb)::FV3_(~comb)()
{
  if(bufsize != 0) delete[] buffer;
}

long FV3_(comb)::getsize()
{
  return bufsize;
}

void FV3_(comb)::setsize(long size)
                throw(std::bad_alloc)
{
  if(size < 0) size = 0;
  if(bufsize != 0) delete[] buffer;
  if(size > 0)
    {
      try
        {
          buffer = new fv3_float_t[size];
        }
      catch(std::bad_alloc)
        {
          std::fprintf(stderr, "comb::setsize(%ld) bad_alloc\n", size);
          delete[] buffer;
          bufsize = 0;
          throw;
        }
    }
  bufsize = size;
  bufidx = 0;
}

void FV3_(comb)::mute()
{
  if(buffer == NULL||bufsize == 0) return;
  FV3_(utils)::mute(buffer, bufsize);
  filterstore = 0;
}

void FV3_(comb)::setdamp(fv3_float_t val) 
{
  damp1 = val; damp2 = 1-val;
}

fv3_float_t FV3_(comb)::getdamp() 
{
  return damp1;
}
void FV3_(comb)::setfeedback(fv3_float_t val) 
{
  feedback = val;
}

fv3_float_t FV3_(comb)::getfeedback() 
{
  return feedback;
}

// modulated comb filter

FV3_(combm)::FV3_(combm)()
{
  bufsize = readidx = writeidx = delaysize = modulationsize = 0;
  buffer = NULL;
  setdamp(0);
}

FV3_(combm)::FV3_(~combm)()
{
  if(bufsize != 0) delete[] buffer;
}

long FV3_(combm)::getsize()
{
  return bufsize;
}

long FV3_(combm)::getdelaysize()
{
  return delaysize;
}

long FV3_(combm)::getmodulationsize()
{
  return modulationsize;
}

void FV3_(combm)::setsize(long size)
		throw(std::bad_alloc)
{
  setsize(size, 0);
}

void FV3_(combm)::setsize(long size, long modsize)
		throw(std::bad_alloc)
{
  if(size < 0) size = 0; if(modsize < 0) modsize = 0;
  if(modsize > size) modsize = size;
  if(bufsize != 0) delete[] buffer;
  if(size+modsize > 0)
    {
      try
	{
	  buffer = new fv3_float_t[size+modsize];
	}
      catch(std::bad_alloc)
	{
	  std::fprintf(stderr, "combm::setsize(%ld) bad_alloc\n", size+modsize);
	  delete[] buffer;
	  bufsize = 0;
	  throw;
	}
    }
  bufsize = size+modsize;
  writeidx = 0;
  readidx = modsize*2;
  delaysize = size;
  modulationsize = modsize;
}

void FV3_(combm)::mute()
{
  if(buffer == NULL||bufsize == 0) return;
  FV3_(utils)::mute(buffer, bufsize);
  filterstore = z_1 = 0;
}

void FV3_(combm)::setdamp(fv3_float_t val) 
{
  damp1 = val; damp2 = 1-val;
}

fv3_float_t FV3_(combm)::getdamp() 
{
  return damp1;
}

void FV3_(combm)::setfeedback(fv3_float_t val) 
{
  feedback = val;
}

fv3_float_t FV3_(combm)::getfeedback() 
{
  return feedback;
}

#include "freeverb/fv3_ns_end.h"
