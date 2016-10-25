#ifndef __TT_TYPEDEF_H__
#define __TT_TYPEDEF_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// 基本数据类型
//#if defined (__GCC32__)
typedef unsigned long long				TTUint64;
typedef long long						TTInt64;
//#else
//typedef unsigned __int64				TTUint64;
//typedef __int64							TTInt64;
//#endif

typedef	unsigned int					TTUint32;
typedef	signed int						TTInt32;

typedef	unsigned short					TTUint16;
typedef	signed short					TTInt16;

typedef	unsigned char					TTUint8;
typedef	signed char						TTInt8;

typedef	unsigned char					TTBYTE;
typedef	unsigned char*					TTPBYTE;

typedef char							TTChar;

typedef	unsigned int					TTUint;
typedef	signed int						TTInt;

typedef void							TTVoid;
typedef void*							TTPtr;
typedef void*							TTHandle;

typedef int								TTBool;
typedef	void*							(*TThreadFunc)(void*);	

enum TTCodecProcessResult
{	
	EProcessUnknown,
	EDstFilled,//Src未完 Dst用完
	ESrcEmptied,//Src用完 Dst未完
	EProcessComplete,//Src Dst都用完
	EProcessError//出错
};

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif // __TT_TYPEDEF_H__
