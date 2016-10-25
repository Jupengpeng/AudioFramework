/******************************************************************************
*
* Shuidushi Software Inc.
* (c) Copyright 2014 Shuidushi Software, Inc.
* ALL RIGHTS RESERVED.
*
*******************************************************************************
*
*  File Name: ttMP3DecBuf.h
*
*  File Description:TT MP3 decoder stream buffer management
*
*******************************Change History**********************************
* 
*    DD/MM/YYYY      Code Ver     Description       Author
*    -----------     --------     -----------       ------
*    30/June/2013      v1.0        Initial version   Kevin
*
*******************************************************************************/

#ifndef __TT_MP3DECBUF_H__
#define __TT_MP3DECBUF_H__

#include "ttMP3DecGlobal.h"
#include "ttMP3DecBit.h"

#define BUFFER_GUARD	8
#define BUFFER_MDLEN	(512 + 2048 + BUFFER_GUARD)
#define BUFFER_DATA	    2048*32 + BUFFER_GUARD

typedef struct _FrameStream{
  unsigned char	*buffer;				
  unsigned char	*buffer_bk;				
  unsigned int	inlen;				
  unsigned char	*main_data;				
  unsigned char *this_frame;
  unsigned char *next_frame;
  unsigned int	length;
  int	storelength;
  int   usedlength;
  unsigned int 	md_len;					
  unsigned int	free_bitrates;
  Bitstream	bitptr;				
} FrameStream;

void ttMP3DecStreamInit(FrameStream *fstream);

#endif  //__TT_MP3DECBUF_H__
