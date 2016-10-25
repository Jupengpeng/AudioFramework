#ifndef __TT_INT_READER_H__
#define __TT_INT_READER_H__
#include "TTTypedef.h"
class CTTIntReader
{
public:
	//static TTBool	IsLitteEndian();
	static TTUint16 ReadUint16(const TTUint8* aReadPtr);// 按高字节在后面读
	static TTUint16 ReadUint16BE(const TTUint8* aReadPtr);// 按高字节在前面读
	static TTUint32 ReadUint32(const TTUint8* aReadPtr);
	static TTUint32 ReadUint32BE(const TTUint8* aReadPtr);
	static TTUint64 ReadUint64(const TTUint8* aReadPtr);
	static TTUint64 ReadUint64BE(const TTUint8* aReadPtr);

	static TTUint16 ReadWord(const TTUint8* aReadPtr);
	static TTUint32 ReadDWord(const TTUint8* aReadPtr);//按顺序读

	static TTUint32 ReadBytesNBE(const TTUint8* aReadPtr, TTInt n);
	static TTUint32 ReadBytesN(const TTUint8* aReadPtr, TTInt n);
};

#endif