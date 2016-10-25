#include "TTOSALConfig.h"
#include <assert.h>

#include "TTSamplerateConv.h"

#define CONVERTED_BUFFER_LENGTH	96 * 1024

#ifdef __TT_OS_ANDROID__
#include <android/log.h>
#define  LOG_TAG    "ChannelMix"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define  LOGV(...)  __android_log_print(ANDROID_LOG_VERBOSE,LOG_TAG,__VA_ARGS__)
#endif

#ifdef __TT_OS_WINDOWS__
#include <malloc.h>
#else
#include <string.h>
#endif

CTTSamplerateConv* SamplerateConvFactory::NewConv(int inSamplerate, int outSamplerate, int channels)
{
	if (inSamplerate != outSamplerate)
	{
//		return new ResampleConv(double(inSamplerate) / outSamplerate);
		return new CTTAflibConv(double(outSamplerate) / inSamplerate, channels);
	}
	else
	{
		return new CTTNullConv();
	}
}

//CTTSamplerateConv::CTTSamplerateConv()
//{
//}

//CTTSamplerateConv::~CTTSamplerateConv()
//{
//}

/*
ResampleConv::ResampleConv(double factor)
{
	iDtb = int (factor * (1<<15) + 0.5);
	iValidX1 = false;
	iX1 = 0;
	iTime = 0;
//	LOGD("factor: %f, iDtb: %d", factor, iDtb);

	CreateConvertedBuffer();
}

ResampleConv::~ResampleConv()
{
	DestroyConvertedBuffer();
}

DataUnit<short> ResampleConv::Conv(DataUnit<short> inDataUnit)
{
	short* unConvertedBuffer = inDataUnit.GetPointer();
	long inLength = inDataUnit.GetLength() - 1;

	assert (unConvertedBuffer != NULL);

	if (inDataUnit.GetLength() == 0)
	{
		return inDataUnit;
	}

	int outLength = 0;

	short* input = unConvertedBuffer;
	short* output = iConvertedBuffer;
	int nIdx = 0;
	int nX2;

	if (iValidX1)
	{
		input = unConvertedBuffer;
	}
	else
	{
		nIdx = iTime >> 15;
		// ptr to current sample
		input = unConvertedBuffer + nIdx;
		iX1 = *input++;
	}

	unsigned short decimal = iTime & 0x7FFF;

	do
	{
		// calculate output sample
		nX2 = *input;
		int xL1 = iX1 * ((1<<15) - decimal);
		int xL2 = nX2 * decimal;
		int nSum = xL1 + xL2;
		*output++ = (short)(nSum >> 15);

		// move to next sample
		iTime += iDtb;
		nIdx = iTime >> 15;
		if (nIdx >= inLength)
		{
			break;
		}
		decimal = iTime & 0x7FFF;

		// ptr to current sample
		input = unConvertedBuffer + nIdx;
		iX1 = *input++;
	}while(1);

	if (nIdx == inLength)
	{
		input = unConvertedBuffer + nIdx;
		iX1 = *input;
		iValidX1 = true;
	}
	else
	{
		iValidX1 = false;
	}

	iTime = iTime - inLength * (1<<15);
	outLength = output - iConvertedBuffer;

	DataUnit<short> outDataUnit(iConvertedBuffer, outLength, CONVERTED_BUFFER_LENGTH);

	return outDataUnit;
}

void ResampleConv::CreateConvertedBuffer()
{
	iConvertedBuffer = (short*)malloc(CONVERTED_BUFFER_LENGTH * sizeof(short));
}

void ResampleConv::DestroyConvertedBuffer()
{
	if (iConvertedBuffer != NULL)
	{
		free(iConvertedBuffer);
		iConvertedBuffer = NULL;
	}
}
*/

CTTAflibConv::CTTAflibConv(double factor, int channels)
: iFactor(factor)
, iAflibConverter(false, false, true)
{
	iAflibConverter.initialize(factor, channels);
//	LOGD("factor: %f, channels: %d", factor, channels);

	CreateConvertedBuffer();
}

CTTAflibConv::~CTTAflibConv()
{
	DestroyConvertedBuffer();
}

DataUnit<TTInt16> CTTAflibConv::Conv(DataUnit<TTInt16> inDataUnit)
{
	if (inDataUnit.GetLength() == 0)
	{
		return inDataUnit;
	}

	TTInt16* input = inDataUnit.GetPointer();
	int inLength = inDataUnit.GetLength();

	TTInt16* output = iConvertedBuffer;
	long outLength = inLength * iFactor;

	assert (input != NULL);

	outLength = iAflibConverter.resample(inLength, outLength, input, output);

	return DataUnit<TTInt16> (output, outLength, CONVERTED_BUFFER_LENGTH);
}

void CTTAflibConv::CreateConvertedBuffer()
{
	iConvertedBuffer = (TTInt16*)malloc(CONVERTED_BUFFER_LENGTH * sizeof(TTInt16));
}

void CTTAflibConv::DestroyConvertedBuffer()
{
	if (iConvertedBuffer != NULL)
	{
		free(iConvertedBuffer);
		iConvertedBuffer = NULL;
	}
}

DataUnit<TTInt16> CTTNullConv::Conv(DataUnit<TTInt16> inDataUnit)
{
	return inDataUnit;
}
