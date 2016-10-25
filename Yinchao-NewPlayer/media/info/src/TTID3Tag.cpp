#include <string.h>
#include "TTID3Tag.h"
#include "TTDataReaderItf.h"
#include "TTIntReader.h"

static const TTInt	 KID3V1_TAG_BYTES = 128;
static const TTInt   KID3V1_TAG_FLAG_BYTES = 3;
static const TTUint8 KID3V1_TAG_FLAG[KID3V1_TAG_FLAG_BYTES] = {	'T', 'A', 'G'};

static const TTInt	 KID3V2_TAG_HEADER_BYTES = 10;
static const TTInt   KID3V2_TAG_FLAG_BYTES = 3;
static const TTUint8 KID3V2_TAG_FLAG[KID3V2_TAG_FLAG_BYTES] = {	'I', 'D', '3'};

static TTInt ID3v2DataSize(const TTUint8* aHeader)
{
	if (memcmp(aHeader, KID3V2_TAG_FLAG, KID3V2_TAG_FLAG_BYTES) != 0)
	{		
		return 0;
	}

	TTInt nSum = 0;
	TTInt nLast = 3;

	for(int i = 0; i <= nLast; i++) 
	{
		if(aHeader[6 + i] & 0x80)
			return 0;

		nSum |= (aHeader[6 + i] & 0x7f) << ((nLast - i) * 7);
	}

	return	nSum;
}

TTInt ID3v2TagSize(ITTDataReader& aDataReader,TTInt aPos)
{
	TTUint8 tHeader[KID3V2_TAG_HEADER_BYTES];
	if(aDataReader.ReadWait(tHeader, aPos, KID3V2_TAG_HEADER_BYTES) != KID3V2_TAG_HEADER_BYTES)
	{
		return 0;
	}

	TTInt nID3v2TagSize = ID3v2DataSize(tHeader);	
	return nID3v2TagSize > 0 ? nID3v2TagSize + KID3V2_TAG_HEADER_BYTES : 0;
}

TTInt ID3v1TagSize(ITTDataReader& aDataReader)
{
	TTUint8 tHeader[KID3V1_TAG_FLAG_BYTES];
	return (aDataReader.ReadSync(tHeader, aDataReader.Size() - KID3V1_TAG_BYTES, KID3V1_TAG_FLAG_BYTES) == KID3V1_TAG_FLAG_BYTES
		&& memcmp(tHeader, KID3V1_TAG_FLAG, KID3V1_TAG_FLAG_BYTES) == 0) ? KID3V1_TAG_BYTES : 0;
}
