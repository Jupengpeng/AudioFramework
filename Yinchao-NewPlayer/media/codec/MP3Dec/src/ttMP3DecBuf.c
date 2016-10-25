/******************************************************************************
*
* Shuidushi Software Inc.
* (c) Copyright 2014 Shuidushi Software, Inc.
* ALL RIGHTS RESERVED.
*
*******************************************************************************
*
*  File Name: ttMP3DecBuf.c
*
*  File Description:TT MP3 decoder stream buffer init
*
*******************************Change History**********************************
* 
*    DD/MM/YYYY      Code Ver     Description       Author
*    -----------     --------     -----------       ------
*    30/June/2013      v1.0        Initial version   Kevin
*
*******************************************************************************/

# include "ttMP3DecCfg.h"
# include "ttMP3DecGlobal.h"
# include "ttMP3DecBuf.h"

void ttMP3DecStreamInit(FrameStream *stream)
{
  stream->buffer     = 0;
  stream->inlen     = 0;
  stream->this_frame = 0;
  stream->next_frame = 0;
  stream->main_data  = 0;
  stream->md_len     = 0;
  stream->free_bitrates = 0;
  stream->storelength = 0;
  stream->length = 0;
  stream->inlen = 0;

  ttMP3DecInitBits(&stream->bitptr, 0, 0);

}


