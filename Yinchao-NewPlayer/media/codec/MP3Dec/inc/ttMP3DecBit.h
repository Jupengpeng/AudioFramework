/******************************************************************************
*
* Shuidushi Software Inc.
* (c) Copyright 2014 Shuidushi Software, Inc.
* ALL RIGHTS RESERVED.
*
*******************************************************************************
*
*  File Name: ttMP3DecBit.h
*
*  File Description:TT MP3 decoder bit parser 
*
*******************************Change History**********************************
* 
*    DD/MM/YYYY      Code Ver     Description       Author
*    -----------     --------     -----------       ------
*    30/June/2013      v1.0        Initial version   Kevin
*
*******************************************************************************/

#ifndef __TT_MP3DECBIT_H__
#define __TT_MP3DECBIT_H__

#include "ttMP3DecGlobal.h"

typedef struct {
	unsigned char *byte;
	unsigned int  cache;
	int   bitsleft;
	int   nbytes;		
}Bitstream;

void	ttMP3DecInitBits(Bitstream* bitptr, unsigned char  *ptr, unsigned int length);
unsigned int    ttMP3DecGetBits(Bitstream *bitptr, int length);
unsigned int    ttMP3DecSkipBits(Bitstream *bitptr, int nBits);
int		ttMP3DecCalcBitsUsed(Bitstream *bitptr, Bitstream *startptr);
unsigned int    ttMP3DecBits_Crc(Bitstream *bitptr, unsigned int   length, unsigned int  check_init);

#endif //__TT_MP3DECBIT_H__
