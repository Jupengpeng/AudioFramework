// TestDec.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include		<stdio.h>
#include		<stdlib.h>
#include		<string.h>
#include		<time.h>
#include		"ttMP3Dec.h"

#define READ_SIZE	(2000)	
short outBuf[1024*8];
unsigned char inBuf[READ_SIZE];


int main(int argc, char* argv[])
{
	FILE *infile, *outfile;
	TTAudioCodecAPI AudioAPI;
	TTHandle hCodec;
	TTBuffer inData;
	TTBuffer outData;
	TTAudioFormat outInfo;
    int firstWrite = 1;
	int eofFile = 0;
	int* info=(int*)inBuf;
	int bytesLeft, nRead;
	int   Isoutput = 1;
	int	returnCode;
	int seekNum = 0;
#define  OUTPUT	  1
	
	/* open input file */
	infile = fopen((const char *)argv[1], "rb");
	if (!infile) {
		return -1;
	}

	/* open output file */
	if(Isoutput)
	{
		outfile = fopen((const char *)argv[2], "wb"); 
		if (!outfile) {
			return -1;
		}
	}

	returnCode  = ttGetMP3DecAPI(&AudioAPI);
	if(returnCode)
		return -1;

//#######################################   Init Decoding Section   #########################################
	returnCode = AudioAPI.Open(&hCodec);
	if(returnCode < 0)
	{
		printf("#### VOI_Error2:fail to initialize the decoder###\n");
		return -1;
	}

	nRead = fread(inBuf, 1, 4, infile);

	bytesLeft = inBuf[0] | inBuf[1] << 8 | inBuf[2] << 16 | inBuf[3] << 24;

	//bytesLeft = READ_SIZE;
	nRead = fread(inBuf, 1, bytesLeft, infile);

	bytesLeft = nRead;
	inData.pBuffer = inBuf;
	inData.nSize = bytesLeft;

//#######################################    Decoding Section   #########################################
	
	do {
		/* decode one AAC frame *///
		returnCode = AudioAPI.SetInput(hCodec,&inData);

		
		do {
			outData.pBuffer   = (TTPBYTE)outBuf;
			outData.nSize	 = 1024*8;

			returnCode = AudioAPI.Process(hCodec,&outData, &outInfo);

#if OUTPUT
			if (returnCode == 0)
			{
				fwrite(outData.pBuffer, 1, outData.nSize, outfile);
				//printf(outfile, "%d\n", decodedFrame);
			}
#endif
		} while(returnCode != TTKErrUnderflow);


#define  MAX_REMAINED 2048
		nRead = fread(inBuf, 1, 4, infile);
		if (!nRead)
			break;

		bytesLeft = inBuf[0] | inBuf[1] << 8 | inBuf[2] << 16 | inBuf[3] << 24;

		if(bytesLeft == 1)
		{
			AudioAPI.SetParam(hCodec, TT_PID_AUDIO_FLUSH, &bytesLeft);
			seekNum++;

			if(seekNum == 34) {
				int nn = 0;
				nn++;
			}
			

			nRead = fread(inBuf, 1, 4, infile);
			if (!nRead)
				break;

			bytesLeft = inBuf[0] | inBuf[1] << 8 | inBuf[2] << 16 | inBuf[3] << 24;

			if(bytesLeft != 418) {
				int nn = 0;
				nn++;
			}

			if (outfile)
			{
				fclose(outfile);
			}

			outfile = fopen((const char *)argv[2], "wb"); 
				if (!outfile) {
				return -1;
			}
		}
		//bytesLeft = READ_SIZE;
		nRead = fread(inBuf, 1, bytesLeft, infile);
		if (!nRead)
			break;

		bytesLeft = nRead;
		inData.pBuffer = inBuf;
		inData.nSize = bytesLeft;
	} while (returnCode);

//################################################  End Decoding Section  #######################################################
	returnCode = AudioAPI.Close(hCodec);
	
	fclose(infile);
	if (outfile)
    {
        fclose(outfile);
    }

	return 0;
}


