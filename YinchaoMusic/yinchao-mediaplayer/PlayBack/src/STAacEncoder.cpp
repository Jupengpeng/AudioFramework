#include <string.h>
#include "STAacEncoder.h"
#include "STLog.h"
#include "STMacrodef.h"

STAacEncoder::STAacEncoder()
: iTotalFrameCount(0)
{

}

STAacEncoder::~STAacEncoder()
{

}

STInt STAacEncoder::Init(STInt aSampleRate, STInt aChannels, const STChar* aSavePath)
{
    if (aacEncOpen(&iHandle, 0, aChannels) != AACENC_OK)
    {
        STLOGE("Unable to open encoder\n");
        return STKErrUnknown;
    }

    if (aacEncoder_SetParam(iHandle, AACENC_AOT, AOT_AAC_LC) != AACENC_OK)
    {
        STLOGE("Unable to set the AOT_AAC_LC\n", AOT_AAC_LC);
        return STKErrUnknown;
    }

    if (aacEncoder_SetParam(iHandle, AACENC_SAMPLERATE, aSampleRate) != AACENC_OK)
    {
        STLOGE("Unable to set the aSampleRate(%d)\n", aSampleRate);
        return STKErrUnknown;
    }

    if (aacEncoder_SetParam(iHandle, AACENC_CHANNELMODE, MODE_2) != AACENC_OK)
    {
        STLOGE("Unable to set the channel MODE_2\n");
        return STKErrUnknown;
    }

    if (aacEncoder_SetParam(iHandle, AACENC_CHANNELORDER, 1) != AACENC_OK) 
    {
        STLOGE("Unable to set the wav channel order\n");
        return STKErrUnknown;
    }

    if (aacEncoder_SetParam(iHandle, AACENC_BITRATE, 64000) != AACENC_OK)
    {
        STLOGE("Unable to set the bitrate\n");
        return STKErrUnknown;
    }

    if (aacEncoder_SetParam(iHandle, AACENC_TRANSMUX, 2) != AACENC_OK) 
    {
        STLOGE("Unable to set the ADTS transmux\n");
        return STKErrUnknown;
    }

    if (aacEncoder_SetParam(iHandle, AACENC_AFTERBURNER, 0) != AACENC_OK)
    {
        STLOGE("Unable to set the iAfterBurner 0\n");
        return STKErrUnknown;
    }

    if (aacEncEncode(iHandle, NULL, NULL, NULL, NULL) != AACENC_OK) 
    {
        STLOGE("Unable to initialize the encoder\n");
        return STKErrUnknown;
    }

    AACENC_InfoStruct tEncodeInfo;
    if (aacEncInfo(iHandle, &tEncodeInfo) != AACENC_OK)
    {
        STLOGE("Unable to get the encoder iInfo\n");
        return STKErrUnknown;
    }

    iSaveFile = fopen(aSavePath, "wb");
    if (!iSaveFile)
    {
        STLOGE("can't open %s", aSavePath);
        return STKErrUnknown;
    }

    iFrameDuration = ((STFloat)tEncodeInfo.frameLength) / aSampleRate;
    iOutBufferSize = aChannels * sizeof(STInt16) * tEncodeInfo.frameLength;
    iOutBuffer = new STUint8[iOutBufferSize];

    return STKErrNone;
}
void STAacEncoder::UnInit()
{
	STLOGE("UnInit");
	iTotalFrameCount = 0;
	iOutBufferSize = 0;
    SAFE_DELETE(iOutBuffer);
    aacEncClose(&iHandle);
    fclose(iSaveFile);
    iSaveFile = NULL;
}

STInt STAacEncoder::Finish()
{
	return Execute(NULL, 0, ESTTrue);
}

STInt STAacEncoder::Duration()
{
	return (STInt)(iTotalFrameCount * iFrameDuration * 1000);
}

STInt STAacEncoder::Execute(STUint8* aBuffer, STUint32 aSize, STBool aIsEnd)
{
    AACENC_BufDesc tInBuf;
    AACENC_BufDesc tOutBuf;
    AACENC_InArgs tInArgs;
    AACENC_OutArgs tOutArgs;
    STInt nInIdentifier = IN_AUDIO_DATA;
    STInt nOutIdentifier = OUT_BITSTREAM_DATA;
    AACENC_ERROR nErr = AACENC_ENCODE_EOF;
    STInt nInElemSize = 2;
    STInt nOutElemSize = 1;

    STInt nRemainSize = aSize;
    STUint8* pBuffer = aBuffer;

    memset(&tInBuf, 0x00, sizeof(AACENC_BufDesc));
	memset(&tOutBuf, 0x00, sizeof(AACENC_BufDesc));
	memset(&tInArgs, 0x00, sizeof(AACENC_InArgs));
	memset(&tOutArgs, 0x00, sizeof(AACENC_OutArgs));

    while (nRemainSize > 0 || aIsEnd)
    {
        tInArgs.numInSamples = aIsEnd ? -1 : (nRemainSize/2);
        tInBuf.numBufs = 1;
        tInBuf.bufs = (void **)&pBuffer;
        tInBuf.bufferIdentifiers = &nInIdentifier;
        tInBuf.bufSizes = &nRemainSize;
        tInBuf.bufElSizes = &nInElemSize;

        tOutBuf.numBufs = 1;
        tOutBuf.bufs = (void **)&iOutBuffer;
        tOutBuf.bufferIdentifiers = &nOutIdentifier;
        tOutBuf.bufSizes = &iOutBufferSize;
        tOutBuf.bufElSizes = &nOutElemSize;
        AACENC_ERROR nErr = aacEncEncode(iHandle, &tInBuf, &tOutBuf, &tInArgs, &tOutArgs);

//        STLOGE("log:%d,%d, %d", tOutArgs.numInSamples, tOutArgs.numOutBytes, nRemainSize);

        if (nErr == AACENC_OK || nErr == AACENC_ENCODE_EOF)
        {
			pBuffer += tOutArgs.numInSamples * 2;
			nRemainSize -= tOutArgs.numInSamples * 2;

			if (tOutArgs.numOutBytes > 0)
			{
				fwrite(iOutBuffer, 1, tOutArgs.numOutBytes, iSaveFile);
				iTotalFrameCount++;
			}

			if (nErr == AACENC_ENCODE_EOF)
			{
				STLOGE("AACENC_ENCODE_EOF.\n");
				break;
			}
        }
        else
        {
        	STLOGE("Encoding failed.\n");
        	return STKErrUnknown;
        }
    }

    return STKErrNone;
}
