#ifndef _TTUTILS_H__
#define _TTUTILS_H__

#include "TTTUtils.h"

#define XRAW_IS_ANNEXB(p) ( !(*((p)+0)) && !(*((p)+1)) && (*((p)+2)==1))
#define XRAW_IS_ANNEXB2(p) ( !(*((p)+0)) && !(*((p)+1)) && !(*((p)+2))&& (*((p)+3)==1))

bool IsAVCIDR(TTPBYTE pBuffer, TTUint32 nSize)
{
	int size = nSize;
	bool bIDR = false;
	int naluType = buffer[0]&0x0f;

	if (buffer[2]==0 && buffer[3]==1) {
		buffer+=4;
		size -= 4;
	} else {
		buffer+=3;
		size -= 3;
	}

	while(1)
	{
		char* p = buffer;  
		char* endPos = buffer+size;
		for (; p < endPos; p++)
		{
			if (XRAW_IS_ANNEXB(p))
			{
				size  -= p-buffer;
				buffer = p+3;
				naluType = buffer[0]&0x0f;
				if(naluType == 5)
					bIDR = true;
				break;
			}
			if (XRAW_IS_ANNEXB2(p))
			{
				size  -= p-buffer;
				buffer = p+4;
				naluType = buffer[0]&0x0f;
				if(naluType == 5)
					bIDR = true;
				break;
			}
		}

		if(p>=endPos)
			return false; 
	}

	return bIDR;
}


bool IsAVCReferenceFrame(TTPBYTE pBuffer, TTUint32 nSize)
{
	int size = nSize;
	TTPBYTE buffer = pBuffer;
	if (buffer[2]==0 && buffer[3]==1) {
		buffer+=4;
		size -= 4;
	} else {
		buffer+=3;
		size -= 3;
	}

	int naluType = buffer[0]&0x0f;
	int isRef	 = 1;
	while(naluType!=1 && naluType!=5)//find next NALU
	{
		char* p = buffer;  
		char* endPos = buffer+size;
		for (; p < endPos; p++) {
			if (XRAW_IS_ANNEXB(p))	{
				size  -= p-buffer;
				buffer = p+3;
				naluType = buffer[0]&0x0f;
				break;
			}

			if (XRAW_IS_ANNEXB2(p))	{
				size  -= p-buffer;
				buffer = p+4;
				naluType = buffer[0]&0x0f;
				break;
			}
		}

		if(p>=endPos)
			return false; 
	}
	
	if(naluType == 5)
		return true;

	if(naluType==1)	{
		isRef = (buffer[0]>>5) & 3;
	}

	return (isRef != 0);
}

#endif
