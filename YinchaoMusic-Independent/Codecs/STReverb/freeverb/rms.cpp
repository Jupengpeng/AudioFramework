/*  RMS
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

#include "freeverb/rms.hpp"
#include "freeverb/fv3_type_float.h"
#include "freeverb/fv3_ns_start.h"

FV3_(rms)::FV3_(rms)()
{
  sum = 0;
  bufs = bufsize = 0;
  bufidx = 0;
}

FV3_(rms)::~FV3_(rms)()
{
  if(bufsize != 0)
    {
      delete[] buffer;
    }
}

long FV3_(rms)::getsize()
{
  return bufsize;
}

void FV3_(rms)::setsize(long size)
	       /*throw(std::bad_alloc)*/
{
  if(bufsize != 0)
    {
      delete[] buffer;
    }
 // try
   // {
      buffer = new fv3_float_t[size];
   // }
  //catch(std::bad_alloc)
   // {
    //  std::fprintf(stderr, "rms::setsize(%ld) bad_alloc\n", size);
   //   delete[] buffer;
    //  bufsize = 0;
   //   throw;
   // }
  bufs = bufsize = size;
  bufidx = 0;
  mute();
}

void FV3_(rms)::mute()
{
  for (long i = 0;i < bufsize;i ++)
    buffer[i] = 0;
  sum = 0;
}

#include "freeverb/fv3_ns_end.h"
