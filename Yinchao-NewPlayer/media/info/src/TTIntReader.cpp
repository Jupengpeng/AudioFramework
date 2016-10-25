#include "TTIntReader.h"
#include "TTMacrodef.h"

// TTBool CTTIntReader::IsLitteEndian()
// {
// 	unsigned short n = 0x1234;
// 	unsigned char* ch = (unsigned char*)&n;
// 	return (*ch == 0x34);
// }

TTUint16 CTTIntReader::ReadUint16(const TTUint8* aReadPtr)
{
	return TTUint16(aReadPtr[1] << 8) | aReadPtr[0];
}

TTUint16 CTTIntReader::ReadUint16BE(const TTUint8* aReadPtr)
{
	return TTUint16(aReadPtr[0] << 8) | aReadPtr[1];
}

TTUint32 CTTIntReader::ReadUint32(const TTUint8* aReadPtr)
{
	return TTUint32(aReadPtr[3] << 24) | TTUint32(aReadPtr[2] << 16) | TTUint32(aReadPtr[1] << 8) | aReadPtr[0];
}

TTUint32 CTTIntReader::ReadUint32BE(const TTUint8* aReadPtr)
{
	return TTUint32(aReadPtr[0] << 24) | TTUint32(aReadPtr[1] << 16) | TTUint32(aReadPtr[2] << 8) | aReadPtr[3];
}

TTUint64 CTTIntReader::ReadUint64(const TTUint8* aReadPtr)
{
	TTUint32 nSizeLow = TTUint32(aReadPtr[3] << 24) | TTUint32(aReadPtr[2] << 16) | TTUint32(aReadPtr[1] << 8) | aReadPtr[0];
	TTUint32 nSizeHigh = TTUint32(aReadPtr[7] << 24) | TTUint32(aReadPtr[6] << 16) | TTUint32(aReadPtr[5] << 8) | aReadPtr[4];

	TTUint64 nRetVal = nSizeHigh;

	return ((nRetVal << 32) | nSizeLow);
}

TTUint64 CTTIntReader::ReadUint64BE(const TTUint8* aReadPtr)
{
	TTUint32 nSizeLow = TTUint32(aReadPtr[0] << 24) | TTUint32(aReadPtr[1] << 16) | TTUint32(aReadPtr[2] << 8) | aReadPtr[3];
	TTUint32 nSizeHigh = TTUint32(aReadPtr[4] << 24) | TTUint32(aReadPtr[5] << 16) | TTUint32(aReadPtr[6] << 8) | aReadPtr[7];

	TTUint64 nRetVal = nSizeHigh;

	return ((nRetVal << 32) | nSizeLow);
}

TTUint16 CTTIntReader::ReadWord(const TTUint8* aReadPtr)
{
	return TTUint16(aReadPtr[0] << 8) | aReadPtr[1];
}

TTUint32 CTTIntReader::ReadDWord(const TTUint8* aReadPtr)
{
	return TTUint32(aReadPtr[0] << 24) | TTUint32(aReadPtr[1] << 16) | TTUint32(aReadPtr[2] << 8) | aReadPtr[3];
}

TTUint32 CTTIntReader::ReadBytesNBE(const TTUint8* aReadPtr, TTInt n)
{
	TTUint32 nRead = 0;
	switch(n)
	{
	case 1:
		nRead = aReadPtr[0];
		break;
	case 2:
		nRead = TTUint32(aReadPtr[0] << 8) | aReadPtr[1];
		break;
	case 3:
		nRead = TTUint32(aReadPtr[0] << 16) | TTUint32(aReadPtr[1] << 8) | aReadPtr[2];
		break;
	case 4:
		nRead = TTUint32(aReadPtr[0] << 24) | TTUint32(aReadPtr[1] << 16) | TTUint32(aReadPtr[2] << 8) | aReadPtr[3];
		break;
	}
	
	return nRead;
}

TTUint32 CTTIntReader::ReadBytesN(const TTUint8* aReadPtr, TTInt n)
{
	TTUint32 nRead = 0;
	switch(n)
	{
	case 1:
		nRead = aReadPtr[0];
		break;
	case 2:
		nRead = TTUint32(aReadPtr[1] << 8) | aReadPtr[0];
		break;
	case 3:
		nRead = TTUint32(aReadPtr[2] << 16) | TTUint32(aReadPtr[1] << 8) | aReadPtr[0];
		break;
	case 4:
		nRead = TTUint32(aReadPtr[3] << 24) | TTUint32(aReadPtr[2] << 16) | TTUint32(aReadPtr[1] << 8) | aReadPtr[0];
		break;
	}
	
	return nRead;
}