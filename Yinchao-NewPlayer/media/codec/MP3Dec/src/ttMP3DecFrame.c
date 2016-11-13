/******************************************************************************
*
* Shuidushi Software Inc.
* (c) Copyright 2014 Shuidushi Software, Inc.
* ALL RIGHTS RESERVED.
*
*******************************************************************************
*
*  File Name: ttMP3DecFrame.c
*
*  File Description:TT MP3 decoder frame process
*
*******************************Change History**********************************
* 
*    DD/MM/YYYY      Code Ver     Description       Author
*    -----------     --------     -----------       ------
*    30/June/2013      v1.0        Initial version   Kevin
*
*******************************************************************************/

#include "ttMP3DecCfg.h"
#include "ttMP3DecGlobal.h"
#include "ttMP3DecBit.h"
#include "ttMP3DecBuf.h"
#include "ttMP3DecFrame.h"

const int ttMP3DecSrTab[3][3] = {
	{0xac44, 0xbb80, 0x7d00},		/* MPEG-1 */
	{0x5622, 0x5dc0, 0x3e80},		/* MPEG-2 */
	{0x2b11, 0x2ee0, 0x1f40},		/* MPEG-2.5 */
};

/* indexing = [version][layer][bitrate index]
 * bitrate (kbps) of frame
 *   - bitrate index == 0 is "free" mode (bitrate determined on the fly by
 *       counting bits between successive sync words)
 */
const short ttMP3DecBrTab[3][3][15] = {
	{
		/* MPEG-1 */
		{  0x0, 0x20, 0x40, 0x60, 0x80, 0xa0, 0xc0, 0xe0, 0x100, 0x120, 0x140, 0x160, 0x180, 0x1a0, 0x1c0}, /* Layer 1 */
		{  0x0, 0x20, 0x30, 0x38, 0x40, 0x50, 0x60, 0x70,  0x80,  0xa0,  0xc0,  0xe0, 0x100, 0x140, 0x180}, /* Layer 2 */
		{  0x0, 0x20, 0x28, 0x30, 0x38, 0x40, 0x50, 0x60,  0x70,  0x80,  0xa0,  0xc0,  0xe0, 0x100, 0x140}, /* Layer 3 */
	},
	{
		/* MPEG-2 */
		{  0x0, 0x20, 0x30, 0x38, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90, 0xa0, 0xb0, 0xc0, 0xe0, 0x100}, /* Layer 1 */
		{  0x0,  0x8, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90,  0xa0}, /* Layer 2 */
		{  0x0,  0x8, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90,  0xa0}, /* Layer 3 */
	},
	{
		/* MPEG-2.5 */
		{  0x0, 0x20, 0x30, 0x38, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90, 0xa0, 0xb0, 0xc0, 0xe0, 0x100}, /* Layer 1 */
		{  0x0,  0x8, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90,  0xa0}, /* Layer 2 */
		{  0x0,  0x8, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90,  0xa0}, /* Layer 3 */
	},
};


int	 ttMP3DecHeaderSync(unsigned char *buf, unsigned int nbytes)
{
	int i;
	
	if(nbytes < BUFFER_GUARD)
		return -1;

	for (i = 0; i < nbytes - BUFFER_GUARD; i++) 
	{
		if ( buf[i+0] == 0xff && (buf[i+1] & 0xe0) == 0xe0)
			return i;
	}
	
	return -2;
}

static int ttMP3DecHeaderinfo(FrameHeader* pHeader, unsigned char *pBuf)
{
	int verIdx, brIdx, srIdx;
	int slots_per_frame;

	verIdx			    = (pBuf[1] >> 3) & 0x03;
	pHeader->version    = (MPAVersion)( verIdx == 0 ? MPEG25 : ((verIdx & 0x01) ? MPEG1 : MPEG2) );
	pHeader->layer	    = (MPA_LAYER)(4 - ((pBuf[1] >> 1) & 0x03));     
	pHeader->crc	    = 1 - ((pBuf[1] >> 0) & 0x01);
	brIdx			    = (pBuf[2] >> 4) & 0x0f;
	srIdx			    = (pBuf[2] >> 2) & 0x03;
	pHeader->paddingBit = (pBuf[2] >> 1) & 0x01;
	pHeader->mode	    = (MPA_MODE)(3 - ((pBuf[3] >> 6) & 0x03));        
	pHeader->modeext    = (pBuf[3] >> 4) & 0x03;

	if(pHeader->layer == 4) 
		pHeader->layer = MPA_LAYER_III;

	if (srIdx == 3 || brIdx == 15 || verIdx == 1)
		return -1;
	
	pHeader->channels = (pHeader->mode == MPA_MODE_SINGLE_CHANNEL ? 1 : 2);
	pHeader->samplerate = ttMP3DecSrTab[pHeader->version][srIdx];
	pHeader->subIndex = pHeader->version*3 + srIdx;
	pHeader->bitrate = ((int)ttMP3DecBrTab[pHeader->version][pHeader->layer - 1][brIdx]) * 1000;	

	if(pHeader->bitrate)
	{	
		if (pHeader->layer == MPA_LAYER_I)
		{
			pHeader->framelen = ((12 * pHeader->bitrate / pHeader->samplerate) + pHeader->paddingBit) * 4;
		}
		else 
		{
			if(pHeader->version > MPEG1 && pHeader->layer == MPA_LAYER_III)
			{
				slots_per_frame = 72;
			}
			else
			{
				slots_per_frame = 144;
			}
			pHeader->framelen = slots_per_frame * pHeader->bitrate/pHeader->samplerate + pHeader->paddingBit;					
		}
	}	
	
	if (pHeader->crc) 
	{
		Bitstream bitptr;
		ttMP3DecInitBits(&bitptr, pBuf + 2, 2);
		pHeader->crc_target = ((int)pBuf[4] << 8 | (int)pBuf[5] << 0);
		pHeader->crc_check = ttMP3DecBits_Crc(&bitptr, 16, 0xffff);
		pHeader->headLen = 6;
		return 6;
	}
	else
	{
		pHeader->crc_target = 0;
		pHeader->headLen = 4;
		return 4;
	}	
}

/*
* NAME:	header->decode()
* DESCRIPTION:	read the next frame header from the stream
*/
int ttMP3DecHeaderDecode(FrameHeader *pHeader, FrameStream *pStream, FrameHeader* pHeader_bk)
{
	unsigned char *ptr, *end;
	int len, hlen; 
	int	slots_per_frame, rate;
	unsigned int N, pad_slot;
	
LOOKFOR_HEADER:
	ptr = pStream->this_frame;
	end = pStream->this_frame + pStream->length;
	hlen = 0;

	do{
		if(hlen < 0) ptr++;
		len  = 	ttMP3DecHeaderSync(ptr, end - ptr);
		if(len < 0)
		{
			if(end - ptr >= BUFFER_GUARD)
			{
				pStream->this_frame = end - BUFFER_GUARD;
				pStream->length = BUFFER_GUARD;
			}
			
			if(len == -2){
				pStream->md_len = 0;
			}

			return -2;
		}
		
		ptr += len;
		pStream->this_frame = ptr;
		pStream->length = end - ptr;
		
		hlen = ttMP3DecHeaderinfo(pHeader, ptr);
	}while(hlen < 0);	
	
	pStream->free_bitrates = 0;
	if (pHeader->bitrate == 0 ) 
	{
		FrameHeader next_header;
		pad_slot = pHeader->paddingBit;

		if(pHeader->version > MPEG1 && pHeader->layer == MPA_LAYER_III)
		{
			slots_per_frame = 72;
		}
		else
		{
			slots_per_frame = 144;
		}

		ptr += hlen;
		for(;;)
		{
			len = ttMP3DecHeaderSync(ptr, end - ptr);
			if (len < 0) 
			{
				pStream->this_frame += 1;
				pStream->length -= 1;
				pStream->md_len = 0;
				goto LOOKFOR_HEADER;
			}

			pStream->next_frame = ptr + len;
			if (ttMP3DecHeaderinfo(&next_header, pStream->next_frame) == 0 &&
				next_header.layer == pHeader->layer &&
				next_header.samplerate == pHeader->samplerate) 
			{			
				N = pStream->next_frame - pStream->this_frame;	
				
				if (pHeader->layer == MPA_LAYER_I) 
				{
					rate = pHeader->samplerate *
						(N - 4 * pad_slot + 4) / 48; 
				}
				else 
				{
					rate = pHeader->samplerate *
						(N - pad_slot + 1) / slots_per_frame;
				}
				
				if (rate >= 8000)
				{
					pHeader->framelen = N;
					break;					
				}
			}
			ptr += len + 1;
		}

		if (rate < 8000 || (pHeader->layer == MPA_LAYER_III && rate > 640000)) 
		{
			pStream->this_frame += 1;
			pStream->length -= 1;
			pStream->md_len = 0;
			goto LOOKFOR_HEADER;
		}
		
		pHeader->bitrate = rate;
		pStream->free_bitrates = rate;
	}	
	
	if(end - pStream->this_frame < pHeader->framelen)
		return -1;

	pStream->next_frame = pStream->this_frame + pHeader->framelen;
	if(end - pStream->this_frame >= pHeader->framelen + 6) 
	{
		if(!(pStream->next_frame[0] == 0xff && (pStream->next_frame[1] & 0xe0) == 0xe0))
		{
			pStream->this_frame += 1;
			pStream->length -= 1;
			pStream->md_len = 0;
			goto LOOKFOR_HEADER;
		}
		else
		{
			FrameHeader Check_header;
			hlen = ttMP3DecHeaderinfo(&Check_header, pStream->next_frame);
			if(hlen < 0 || Check_header.channels != pHeader->channels || 
				Check_header.layer != pHeader->layer || Check_header.samplerate != pHeader->samplerate)
			{
				pStream->this_frame += 1;
				pStream->length -= 1;
				pStream->md_len = 0;
				goto LOOKFOR_HEADER;
			}
		}
	}
	else
	{
		if(pHeader_bk->channels == 0 || pHeader_bk->samplerate == 0 || pHeader_bk->layer == 0)
		{
			return -1;
		}

		if(pHeader_bk->channels != pHeader->channels || 
			pHeader_bk->layer != pHeader->layer || pHeader_bk->samplerate != pHeader->samplerate)
		{
			pStream->this_frame += 1;
			pStream->length -= 1;
			pStream->md_len = 0;
			goto LOOKFOR_HEADER;
		}
	}	
	return 0;
}




