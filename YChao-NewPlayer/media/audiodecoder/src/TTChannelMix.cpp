#include "TTOSALConfig.h"

#include <math.h>
#include <assert.h>

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

#include "TTChannelMix.h"

#define MIXED_BUFFER_LENGTH	448 * 1152

#define CHANNEL_MONO	1
#define CHANNEL_STEREO	2



CTTChannelMixer* ChannelMixerFactory::NewMixer(int inChannel, int outChannel)
{
	if (inChannel == CHANNEL_STEREO && outChannel == CHANNEL_MONO)
	{
		return new CTTStereoMonoMixer();
	}
	else if (inChannel == CHANNEL_MONO && outChannel == CHANNEL_STEREO)
	{
		return new CTTMonoStereoMixer();
	}
	else
	{
		return new CTTNullMixer();
	}
}

CTTChannelMixer::CTTChannelMixer()
{
	//CreateMixedBuffer();
}

CTTChannelMixer::~CTTChannelMixer()
{
	//DestroyMixedBuffer();
}

void CTTChannelMixer::CreateMixedBuffer()
{
	iMixedBuffer = (TTInt16*)malloc(MIXED_BUFFER_LENGTH * sizeof(TTInt16));
}

void CTTChannelMixer::DestroyMixedBuffer()
{
	if (iMixedBuffer != NULL)
	{
		free(iMixedBuffer);
		iMixedBuffer = NULL;
	}
}

DataUnit<TTInt16> CTTStereoMonoMixer::Mix(DataUnit<TTInt16> inDataUnit)
{
	TTInt16* inPtr = inDataUnit.GetPointer();
	long inLength = inDataUnit.GetLength();

	assert (inPtr != NULL && (inLength % 2 == 0) && (inLength /2 <= MIXED_BUFFER_LENGTH));

	bool bias = GetCrossCorrelation(inPtr, inLength) < -0.98;
	TTInt16* outPtr = inPtr;

	for (int i = 0; i < inLength; i += 2)
	{
		TTInt16 left = *inPtr++;
		TTInt16 right = *inPtr++;
		if (bias)
		{
			right = -right;
		}

		*outPtr++ = (left + right) / 2;
	}


	DataUnit<TTInt16> outDataUnit(inDataUnit.GetPointer(), inLength / 2, inLength);

	return outDataUnit;
}

double CTTStereoMonoMixer::GetCrossCorrelation(TTInt16* samples, long length)
{
      // Cross Channel Correlation - stereo signals only
      long k;
      double C12 = 0, C11 = 0, C22 = 0;

      for (k=0; k<length*2; k+=2)
      {
            C12 += samples[k]*samples[k+1];
            C11 += samples[k]*samples[k];
            C22 += samples[k+1]*samples[k+1];
      }

      return C12/sqrt(C11*C22);
}

DataUnit<TTInt16> CTTMonoStereoMixer::Mix(DataUnit<TTInt16> inDataUnit)
{
	TTInt16* inPtr = inDataUnit.GetPointer();
	long inLength = inDataUnit.GetLength();

	assert (inPtr != NULL && (inLength * 2 <= MIXED_BUFFER_LENGTH));

	TTInt16* outPtr = iMixedBuffer;

	for (int i = 0; i < inLength; i++)
	{
		TTInt16 val = *inPtr++;
		*outPtr++ = val;
		*outPtr++ = val;
	}

	DataUnit<TTInt16> outDataUnit(iMixedBuffer, inLength * 2, MIXED_BUFFER_LENGTH);

	return outDataUnit;
}

DataUnit<TTInt16> CTTNullMixer::Mix(DataUnit<TTInt16> inDataUnit)
{
	return inDataUnit;
}

