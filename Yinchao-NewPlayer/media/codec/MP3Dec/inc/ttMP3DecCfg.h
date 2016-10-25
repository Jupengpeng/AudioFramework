/******************************************************************************
*
* Shuidushi Software Inc.
* (c) Copyright 2014 Shuidushi Software, Inc.
* ALL RIGHTS RESERVED.
*
*******************************************************************************
*
*  File Name: ttMP3DecCfg.h
*
*  File Description:TT MP3 decoder some constant value define
*
*******************************Change History**********************************
* 
*    DD/MM/YYYY      Code Ver     Description       Author
*    -----------     --------     -----------       ------
*    30/June/2013      v1.0        Initial version   Kevin
*
*******************************************************************************/

#ifndef __TT_MP3DECCFG_H__
#define __TT_MP3DECCFG_H__

#ifndef CHAR_BIT
#define CHAR_BIT				8
#endif

#define MAX_SCFBD				4
#define MAX_NGRAN				2	
#define MAX_NCHAN				2	
#define MAX_NSAMP				576	
#define SBLIMIT					32

#define VBUF_LENGTH				(17 * 2 * SBLIMIT)

# define F_FRACBITS		28
# define F_ONE			0x10000000

#ifndef MAX_32
#define MAX_32			(int)0x7fffffffL
#endif

#ifndef MIN_32
#define MIN_32			(int)0x80000000L
#endif

#ifndef MAX_16
#define MAX_16			(short)0x7fff
#endif

#ifndef MIN_16
#define MIN_16			(short)0x8000
#endif

#ifndef MAX
#define MAX(a,b)	((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b)	((a) < (b) ? (a) : (b))
#endif

#define TT_MP3_SUCCEED           	0
#define TT_MP3_INPUT_BUFFER_SMALL   5

#endif //__TT_MP3DECCFG_H__