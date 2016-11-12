/******************************************************************************
*
* Shuidushi Software Inc.
* (c) Copyright 2014 Shuidushi Software, Inc.
* ALL RIGHTS RESERVED.
*
*******************************************************************************
*
*  File Name: ttMP3DecApi.c
*
*  File Description:TT MP3 decoder APIs function
*
*******************************Change History**********************************
* 
*    DD/MM/YYYY      Code Ver     Description       Author
*    -----------     --------     -----------       ------
*    30/June/2013      v1.0        Initial version   Kevin
*
*******************************************************************************/

#include	"ttMP3DecCfg.h"
#include	"ttMP3DecGlobal.h"
#include	"ttMP3DecFrame.h"
#include	"ttMP3Dec.h"
#include    "ttMemAlign.h"
#include <string.h>
//#define LOGDUMP
#ifdef LOGDUMP
static FILE* dump =NULL;
#endif

#define PCMDUMP
#ifdef PCMDUMP
static FILE* pcm =NULL;
#endif

static TTInt32 ttMP3DecInit(TTHandle * phCodec)
{
	MP3DecInfo		*decoder = NULL;
	FrameStream		*stream = NULL;
	FrameDataInfo   *frame = NULL;
	SubbandInfo		*subband = NULL;

	decoder = (MP3DecInfo *)mem_malloc(sizeof(MP3DecInfo), 32);
	if(NULL == decoder)
		goto INIT_FAIL;

	frame = (FrameDataInfo *)mem_malloc(sizeof(FrameDataInfo), 32);
	if(NULL == frame)
		goto INIT_FAIL;

	stream = (FrameStream *)mem_malloc(sizeof(FrameStream), 32);
	if(NULL == stream)
		goto INIT_FAIL;
	ttMP3DecStreamInit(stream);

	subband = (SubbandInfo *)mem_malloc(sizeof(SubbandInfo), 32);
	if(NULL == subband)
		goto INIT_FAIL;

	decoder->decoder_buf = (unsigned char*)mem_malloc(BUFFER_DATA, 32);
	if(decoder->decoder_buf == 0)
		goto INIT_FAIL;	

	stream->main_data = (unsigned char *)mem_malloc(BUFFER_MDLEN, 32);
	if (stream->main_data == 0) 
		goto INIT_FAIL;

	stream->main_data[BUFFER_MDLEN - 4] = 
		stream->main_data[BUFFER_MDLEN - 5] = 0xFF;

	stream->buffer_bk = decoder->decoder_buf;

	decoder->stream	= stream;
	decoder->frame	= frame;
	decoder->subband = subband;

#ifdef LOGDUMP
	dump = fopen("/Users/yintao/Desktop/PCMS/dump.pcm", "wb");
#endif

#ifdef PCMDUMP
	pcm = fopen("/Users/yintao/Desktop/PCMS/dump.pcm", "wb");
#endif
	*phCodec = (TTHandle)decoder;

	return TTKErrNone;
	
INIT_FAIL:
	if(stream)	{
		if (stream->main_data) {
			mem_free(stream->main_data);
			stream->main_data = 0;
		}
		mem_free(decoder->stream);
		decoder->stream = NULL;
	}
	if(decoder->decoder_buf){
		mem_free(decoder->decoder_buf);
		decoder->decoder_buf = NULL;
	}
	
	if(frame) mem_free(frame);
	if(subband) mem_free(subband);	
	if(decoder)	mem_free(decoder);
	*phCodec = NULL;

	return TTKErrNoMemory;	
}

static TTInt32 ttMP3DecSetInputData(TTHandle hCodec, TTBuffer * pInput)
{
	MP3DecInfo		*decoder;
	FrameStream		*stream;
	int				len;

	if(NULL == hCodec)
	{
		return TTKErrArgument;
	}

	decoder = (MP3DecInfo *)hCodec;
	stream = decoder->stream;

	if(NULL == pInput || NULL == pInput->pBuffer || 0 > pInput->nSize)
	{
		return TTKErrArgument;
	}	

#ifdef LOGDUMP
	if(dump)
	{
		fwrite(&pInput->nSize, 1, 4, dump);
		fwrite(pInput->pBuffer, 1, pInput->nSize, dump);
	}
#endif

	stream->buffer = pInput->pBuffer;
	stream->inlen = pInput->nSize;

	stream->this_frame = stream->buffer;
	stream->length = stream->inlen;
	stream->usedlength = 0;

	if(stream->storelength)
	{
		len = MIN( BUFFER_DATA - stream->storelength, stream->inlen);
		memcpy(stream->buffer_bk  + stream->storelength, stream->buffer, len);

		stream->this_frame = stream->buffer_bk;
		stream->length = stream->storelength + len;
		stream->usedlength = -stream->storelength;
	}

	return TTKErrNone;
}


static TTInt32 ttMP3DecGetOutputData(TTHandle hCodec, TTBuffer * pOutput, TTAudioFormat* pOutInfo)
{
	MP3DecInfo		*decoder;
	FrameStream		*stream;
	FrameDataInfo   *frame;
	FrameHeader		*header;
	SubbandInfo		*subband;
	int				len;
	unsigned int	nch, ns;
	short*			outbuf;
	unsigned char*  start;

	if(NULL == hCodec)
	{
		return TTKErrArgument;
	}

	decoder = (MP3DecInfo *)hCodec;
	stream = decoder->stream;
	frame  = decoder->frame;
	subband = decoder->subband;
	header = &frame->header;	

	if(NULL == pOutput || NULL == pOutput->pBuffer )
	{
		return TTKErrArgument;
	}
	
	start = stream->this_frame;
	len = ttMP3DecHeaderDecode(header, stream, &decoder->header_bk);
	if(len == -2) 
	{
		if(stream->storelength) {
			len = MIN( BUFFER_DATA - stream->storelength, stream->inlen) - stream->length;
			len = MAX(len , 0);
			stream->this_frame = stream->buffer + len;
			stream->length = stream->inlen - len;
			stream->storelength = 0;
			stream->usedlength += len;
			start = stream->this_frame;

			len = ttMP3DecHeaderDecode(header, stream, &decoder->header_bk);
		}
	}

	if(len < 0)
	{
		stream->usedlength += (stream->this_frame - start);
		len = stream->length;		
		//if(stream->storelength == 0)
		{	
			memcpy(stream->buffer_bk, stream->this_frame, len);			
		}
		stream->storelength = len;
		pOutput->nSize = 0;
		stream->usedlength += len;

		return TTKErrUnderflow;
	}

	frame->nGrans = (header->version == MPEG1 && header->layer == MPA_LAYER_III) ? 2 : 1;
	switch(header->layer)
	{
	case 1:
		len = ttMP3DecLayerI(frame ,stream);
		break;
	case 2:
		len = ttMP3DecLayerII(frame ,stream);
		break;
	case 3:
		len = ttMP3DecLayerIII(frame ,stream);
		break;
	default:
		break;
	}

	stream->this_frame += header->framelen;
	stream->length -= header->framelen;

	if(stream->storelength)
	{
		int leftlen;
		int length = stream->this_frame - start;
		stream->usedlength += length;
		if(length > stream->storelength)
		{
			leftlen = length - stream->storelength;
			stream->buffer += leftlen;
			stream->inlen  -= leftlen;
			stream->this_frame = stream->buffer;
			stream->length = stream->inlen;
			stream->storelength = 0;
		}
		else
		{			
			memcpy(stream->buffer_bk, stream->this_frame, stream->length);
			stream->usedlength += length;
			stream->this_frame = stream->buffer_bk;
			stream->storelength = stream->length;
		}		
	}
	else
	{
		stream->usedlength += stream->this_frame - start;
	}

	if(len < 0) 
	{
		pOutput->nSize = 0;
		return TTErrMP3InvFrame;
	}	

	if(len==TT_MP3_INPUT_BUFFER_SMALL){
		 pOutput->nSize = 0;
		return TTKErrUnderflow;
	}

	nch = header->channels;
	ns  = header->layer == MPA_LAYER_I ? 12 : 
		((header->version > MPEG1 && header->layer == MPA_LAYER_III) ? 18 : 36);
	
	len = SBLIMIT * ns * nch *sizeof(short);
	if(len > pOutput->nSize)
	{
		pOutput->nSize = 0;	
		return TTKErrOverflow;
	}

	pOutput->nSize = len;
	outbuf = (short *)pOutput->pBuffer;

	ttMP3DecSubbandFrame(frame, subband, outbuf, nch, ns);

	if(pOutInfo)
	{
		pOutInfo->Channels = nch;
		pOutInfo->SampleBits = 16;
		pOutInfo->SampleRate = header->samplerate;
	}

#ifdef PCMDUMP
	if(pcm){
	 fwrite(pOutput->pBuffer, 1, pOutput->nSize, pcm);
	}
#endif
	memcpy(&decoder->header_bk, header, sizeof(FrameHeader));

	return TTKErrNone;
}

static TTInt32 ttMP3DecSetParam(TTHandle hCodec, TTInt32 uParamID, TTPtr pData)
{
	MP3DecInfo *decoder;
	FrameStream  *stream;
	FrameHeader  *header;
	SubbandInfo	*subband;
	FrameDataInfo  *frame = NULL;
	TTAudioFormat	*pFormat;
	if(NULL == hCodec)
	{
		return TTKErrArgument;
	}

	decoder = (MP3DecInfo *)hCodec;
	stream = decoder->stream;
	subband = decoder->subband;
	header  = &decoder->frame->header;
	frame = decoder->frame;

	switch(uParamID)
	{
	case TT_PID_AUDIO_DECODER_INFO:
		break;
	case TT_PID_AUDIO_FLUSH:
		if(pData == NULL)
			return TTKErrArgument;

#ifdef LOGDUMP
		if(dump)
		{
			fwrite(pData, 1, 4, dump);
		}
#endif
		if(*((int *)pData))
		{
			stream->inlen = 0;
			stream->length = 0;
			stream->storelength = 0;
			stream->md_len = 0;
			stream->usedlength = 0;

			memset(subband, 0, sizeof(SubbandInfo));
			memset(frame, 0, sizeof(FrameDataInfo));
		}
		break;
	case TT_PID_AUDIO_FORMAT:
		pFormat = (TTAudioFormat *)pData;
		header->channels = pFormat->Channels;
		header->samplerate = pFormat->SampleRate;
		break;
	default:
		return TTKErrArgument;
	}

	return TTKErrNone;
}

static TTInt32 ttMP3DecGetParam(TTHandle hCodec, TTInt32 uParamID, TTPtr pData)
{
	MP3DecInfo		*decoder;
	FrameHeader     *header;
	TTAudioFormat	*pFormat;
	if(NULL == hCodec || pData == NULL)
	{
		return TTKErrArgument;
	}
	
	decoder = (MP3DecInfo *)hCodec;
	header  = &decoder->frame->header;

	switch(uParamID)
	{
	case TT_PID_AUDIO_FORMAT:
		pFormat = (TTAudioFormat *)pData;
		pFormat->Channels = header->channels;
		pFormat->SampleBits = 16;
		pFormat->SampleRate = header->samplerate;
		break;
	case TT_PID_AUDIO_BITRATE:
		*((int *)pData) = header->bitrate;
		break;
	default:
		return TTKErrArgument;
	}

	return TTKErrNone;
}

static TTInt32 ttMP3DecUninit(TTHandle hCodec)
{
	/* release the decoder */
	MP3DecInfo *decoder;

	if(NULL == hCodec)
	{
		return TTKErrArgument;
	}
	
	decoder = (MP3DecInfo *)hCodec;

	if(decoder->stream)	{
		if (decoder->stream->main_data) {
			mem_free(decoder->stream->main_data);
			decoder->stream->main_data = NULL;
		}
		mem_free(decoder->stream);
		decoder->stream = NULL;
	}

	if(decoder->decoder_buf){
		mem_free(decoder->decoder_buf);
		decoder->decoder_buf = NULL;
	}
	
	if(decoder->frame) {
		mem_free(decoder->frame);
		decoder->frame = NULL;
	}

	if(decoder->subband) {
		mem_free(decoder->subband);
		decoder->subband = NULL;
	}

#ifdef LOGDUMP
	if(dump)
		fclose(dump);
#endif
	mem_free(decoder);

	return TTKErrNone;
}

TTInt32 ttGetMP3DecAPI(TTAudioCodecAPI * pDecHandle)
{
	if(pDecHandle == NULL)
		return TTKErrArgument;
		
	pDecHandle->Open = ttMP3DecInit;
	pDecHandle->SetInput = ttMP3DecSetInputData;
	pDecHandle->Process = ttMP3DecGetOutputData;
	pDecHandle->SetParam = ttMP3DecSetParam;
	pDecHandle->GetParam = ttMP3DecGetParam;
	pDecHandle->Close = ttMP3DecUninit;

	return TTKErrNone;
}
