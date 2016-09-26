#ifndef __ST_TYPEDEF_H__
#define __ST_TYPEDEF_H__

// 基本数据类型
typedef unsigned long long				STUint64;
typedef long long						STInt64;

typedef	unsigned long					STUint32;
typedef	signed long						STInt32;

typedef	unsigned short					STUint16;
typedef	signed short					STInt16;

typedef	unsigned char					STUint8;
typedef	signed char						STInt8;

typedef char							STChar;

typedef	unsigned int					STUint;
typedef	signed int						STInt;

typedef float							STFloat;

typedef double							STDouble;

typedef bool							STBool;
typedef	void*							(*STThreadFunc)(void*);	

#endif // __ST_TYPEDEF_H__
