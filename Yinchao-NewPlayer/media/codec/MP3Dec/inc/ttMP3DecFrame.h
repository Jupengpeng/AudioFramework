/******************************************************************************
*
* Shuidushi Software Inc.
* (c) Copyright 2014 Shuidushi Software, Inc.
* ALL RIGHTS RESERVED.
*
*******************************************************************************
*
*  File Name: ttMP3DecFrame.h
*
*  File Description:TT MP3 decoder frame operation header file 
*
*******************************Change History**********************************
* 
*    DD/MM/YYYY      Code Ver     Description       Author
*    -----------     --------     -----------       ------
*    30/June/2013      v1.0        Initial version   Kevin
*
*******************************************************************************/

#ifndef __TT_MP3DECFRAME_H__
#define __TT_MP3DECFRAME_H__

#include "ttMP3DecCfg.h"
#include "ttMP3DecGlobal.h"
#include "ttMP3DecBuf.h"

/* map to 0,1,2 to make table indexing easier */
typedef enum {
	MPEG1 =  0,
	MPEG2 =  1,
	MPEG25 = 2
} MPAVersion;

typedef enum _MPA_LAYER{
	MPA_LAYER_I   = 1,					/* Layer I */
	MPA_LAYER_II  = 2,					/* Layer II */
	MPA_LAYER_III = 3					/* Layer III */
} MPA_LAYER;

typedef enum _MPA_MODE{
	MPA_MODE_SINGLE_CHANNEL   = 0,		/* single channel */
	MPA_MODE_DUAL_CHANNEL	  = 1,		/* dual channel */
	MPA_MODE_JOINT_STEREO	  = 2,		/* joint (MS/intensity) stereo */
	MPA_MODE_STEREO			  = 3		/* normal LR stereo */
} MPA_MODE;


typedef struct _FrameHeader{
	MPAVersion  version;
	MPA_LAYER	layer;				
	int			crc;					/* CRC flag: 0 = disabled, 1 = enabled */	 
	int			bitrate;		
	int			samplerate;		
	MPA_MODE	mode;			
	int			modeext;	
	int			paddingBit;
	int			channels;
	int			subIndex;
	int			crc_check;	
	int			crc_target; 
	int			headLen;
	int		    framelen;
} FrameHeader;

typedef struct _SideInfoSub {
	int		part23Length;				
	int		nBigvals;					
	int		globalGain;					
	int		sfCompress;					
	int		winSwitchFlag;				
	int		blockType;
	int		mixedBlock;					
	int		tableSelect[3];				
	int		subBlockGain[3];			
	int		region0Count;				
	int		region1Count;				
	int		preFlag;					
	int		sfactScale;					
	int		count1TableSelect;
	unsigned char scalefac[39];
} SideInfoSub;

typedef struct _SideInfo {
	int mainDataBegin;
	int privateBits;
	int scfSi[MAX_NCHAN];					 /* 4 scalefactor bands per channel */

	SideInfoSub	sis[MAX_NGRAN][MAX_NCHAN];
} SideInfo;

typedef struct _FrameDataInfo{
	FrameHeader header;		
	SideInfo	sideInfoPS;
	int			sbSample[MAX_NCHAN][36][32];
	int			overlap[MAX_NCHAN][576/2];	
	int			xr[MAX_NCHAN][576];
	int			modes[40];
	int			xr_nzero[MAX_NCHAN];
	int			preIMDCT[MAX_NCHAN];
	int			preType[MAX_NCHAN];
	int			siLen;
	int			nGrans;

}FrameDataInfo;

typedef struct _SubbandInfo{
#ifndef ARMV7_OPT
	int vbuf[MAX_NCHAN * VBUF_LENGTH];	
#else
	short vbuf[MAX_NCHAN * VBUF_LENGTH];	
#endif
	int vindex;							
}SubbandInfo;

typedef struct _MP3DecInfo{
	FrameHeader      header_bk;
	FrameStream		*stream;
	FrameDataInfo	*frame;
	SubbandInfo		*subband;
	unsigned char	*decoder_buf;
}MP3DecInfo;

int	 ttMP3DecHeaderSync(unsigned char *buf, unsigned int nbytes);
int  ttMP3DecHeaderDecode(FrameHeader*, FrameStream *, FrameHeader*);

int  ttMP3DecLayerI(FrameDataInfo *, FrameStream * );
int  ttMP3DecLayerII(FrameDataInfo *, FrameStream *);
int  ttMP3DecLayerIII(FrameDataInfo *, FrameStream *);

#ifndef ARMV7_OPT
void ttMP3DecDCT32(int *buf, int *dest, int offset, int oddBlock);
#else
void ttMP3DecDCT32(int *buf, short *dest, int offset, int oddBlock);
#endif
int  ttMP3DecSubbandFrame(FrameDataInfo *, SubbandInfo *, short *, int, int);

/*
* table for requantization
*
* rq_table[x].mantissa * 2^(rq_table[x].exponent) = x^(4/3)
*/
typedef struct _fixedfloat {
	unsigned int mantissa  : 27;
	unsigned short exponent :  5;
} fixedfloat;

extern const int ttMP3DecSrTab[3][3];             //sample rate table 
extern const fixedfloat ttMP3DecRqTab[8207];      //requantization table
extern const short ttMP3DecBrTab[3][3][15];       //bit rate table

#ifdef ARMV7_OPT
extern const int dcttab[48];
extern const int dctIdx[12];
extern const int csa_table[8][4];
extern const int IDCT9_D[5];
#endif

#endif //__TT_MP3DECFRAME_H__
