#ifndef  __TT_MACRODEF_H__
#define  __TT_MACRODEF_H__


#include <assert.h>
#include <stdio.h>

#include "TTOsalConfig.h"

#ifndef ETTTrue
#define ETTTrue					true
#endif

#ifndef ETTFalse
#define ETTFalse				false
#endif

#ifndef NULL
#define NULL					0
#endif

#ifndef KILO
#define KILO					1024
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)			{delete (p);    \
								(p) = NULL;}
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p)	{delete[] (p);    \
								(p) = NULL;}
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)			{if((p)) ((p)->Release()); \
								(p) = NULL;}
#endif

#ifndef SAFE_RELEASE_ARRAY
#define SAFE_RELEASE_ARRAY(p) { for (TTInt nIdx = ((p).Count() - 1); nIdx >= 0;	--nIdx)	\
								{							\
									(p)[nIdx]->Release();	\
								}							\
								(p).Close();				\
								}
#endif

#ifndef SAFE_FREE
#define SAFE_FREE(p)			{if((p)) free((p)); \
								(p) = NULL;}
#endif

#ifndef FILE_PATH_MAX_LENGTH
#define FILE_PATH_MAX_LENGTH 1024
#endif

#ifndef TTASSERT
#define TTASSERT(exp)			assert(exp)
#endif

#ifndef EXPORT
#define EXPORT				__declspec(dllexport)
#endif

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3) ( \
	( (TTInt)(char)(ch0) << 24 ) | \
	( (TTInt)(char)(ch1) << 16 ) | \
	( (TTInt)(char)(ch2) << 8 ) | \
	( (TTInt)(char)(ch3) ) )
#endif

#define WORD_ALGIN(size)		((size) & 1) ? (size) + 1 : (size)

#ifndef CLIP
#define CLIP(x, a, b)			{					\
									if ((x) < (a))	\
									(x) = (a);	\
									if ((x) > (b))	\
									(x) = (b);	\
								}
#endif		

#define SWAP16(x) (unsigned short)(((unsigned short)(x) >> 8) | ((unsigned short)(x) << 8))

#define SWAP32(x) (unsigned int)(((unsigned int)(x) >> 24) | \
								 (((unsigned int)(x) & 0xFF0000UL) >> 8) | \
								 (((unsigned int)(x) & 0xFF00UL) << 8) | \
								 ((unsigned int)(x) << 24))

#ifndef MAX
#define MAX(a,b)	((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b)	((a) < (b) ? (a) : (b))
#endif

#ifndef ABS
#define ABS(x)		((x) < 0 ? -(x) : (x))
#endif

#ifdef __TT_OS_ANDROID__
#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)
#else
#define likely(x)       (x)
#define unlikely(x)     (x)
#endif

#define TT_MAX_ENUM_VALUE	0X7FFFFFFF

/**
System wide error code 0 : this represents the no-error condition.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define TTKErrNone			0

/**
System wide error code -1 : item not found.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define TTKErrNotFound		-1 // Must remain set to -1

/**
System wide error code -2 : an error that has no specific categorisation.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define TTKErrGeneral			-2

/**
System wide error code -3 : indicates an operation that has been cancelled.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define TTKErrCancel          -3

/**
System wide error code -4 : an attempt to allocate memory has failed.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define TTKErrNoMemory        -4

/**
System wide error code -5 : some functionality is not supported in a given context.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.

There may be many reasons for this; for example, a device may not support
some specific behaviour.
*/
#define TTKErrNotSupported	-5

/**
System wide error code -6 : an argument is out of range.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define TTKErrArgument		-6

/**
System wide error code -7 : a calculation has lost precision.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.

This error arises when converting from an internal 96-bit real representation
to a TReal32; the exponent of the internal representation is so small
that the 32-bit real cannot contain it.
*/
#define TTKErrTotalLossOfPrecision -7

/**
System wide error code -8 : an invalid handle has been passed.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.

A function involving a resource owned by a server or the kernel has
specified an invalid handle.
*/
#define TTKErrBadHandle			-8




/**
System wide error code -9 : indicates an overflow in some operation.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.

In the context of mathematical or time/date functions, indicates a calculation
that has produced arithmetic overflow exceeding the bounds allowed by
the representation.

In the context of data transfer, indicates that a buffer has over-filled
without being emptied soon enough.
*/
#define TTKErrOverflow            -9




/**
System wide error code -10 : indicates an underflow in some operation.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.

In the context of mathematical or time/date functions, indicates a calculation
that has produced a result smaller than the smallest magnitude of
a finite number allowed by the representation.

In the context of data transfer, indicates that a buffer was under-filled
when data was required.
*/
#define TTKErrUnderflow			-10




/**
System wide error code -11 : an object already exists.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.

An object of some name/type is to be created, but an object of
that name/type already exists.
*/
#define TTKErrAlreadyExists       -11




/**
System wide error code -12 : in the context of file operations, a path
was not found.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define TTKErrPathNotFound		-12




/**
System wide error code -13 : a handle refers to a thread that has died.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define TTKErrDied				-13

/**
System wide error code -14 : a requested resource is already in exclusive use.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define TTKErrInUse				-14

/**
System wide error code -15 : client/server send/receive operation cannot run,
because the server has terminated.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define TTKErrServerTerminated	-15

/**
System wide error code -16 : a client/server send/receive operation cannot run,
because the server is busy handling another request.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define TTKErrServerBusy			-16




/**
System wide error code -17 : indicates that an operation is complete,
successfully or otherwise.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.

This code may be used to indicate that some follow on operation can take place.
It does not necessarily indicate an error condition.
*/
#define TTKErrCompletion			-17

/**
System wide error code -18 : indicates that a device required by an i/o operation
is not ready to start operations.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.

A common reason for returning this code is because a device has not been
initialised, or has no power.
*/
#define TTKErrNotReady			-18




/**
System wide error code -19 : a device is of unknown type.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define TTKErrUnknown				-19




/**
System wide error code -20 : indicates that some media is not formatted properly,
or links between sections of it have been corrupted.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define TTKErrCorrupt				-20




/**
System wide error code -21 : access to a file is denied, because the permissions on
the file do not allow the requested operation to be performed.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define TTKErrAccessDenied		-21




/**
System wide error code -22 : an operation cannot be performed, because the part
of the file to be read or written is locked.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define TTKErrLocked				-22




/**
System wide error code -23 : during a file write operation, not all the data
could be written.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define TTKErrWrite				-23




/**
System wide error code -24 : a volume which was to be used for a file system
operation has been dismounted.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define TTKErrDisMounted			-24   




/**
System wide error code -25 : indicates that end of file has been reached.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.

Note that RFile::Read() is a higher-level interface. When the end of
the file is reached, it returns zero bytes in the destination descriptor, and
a TTKErrNone return value. TTKErrEof is not used for this purpose; other error
conditions are returned only if some other error condition was indicated on
the file.
*/
#define TTKErrEof					-25   




/**
System wide error code -26 : a write operation cannot complete, because the disk
is full.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define TTKErrDiskFull			-26   




/**
System wide error code -27 : a driver DLL is of the wrong type.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define TTKErrBadDriver			-27   




/**
System wide error code -28 : a file name or other object name does not conform to
the required syntax.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define TTKErrBadName				 -28   




/**
System wide error code -29 : a communication line has failed.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define TTKErrCommsLineFail		  -29   




/**
System wide error code -30 : a frame error has occurred in
a communications operation.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define TTKErrCommsFrame				-30   




/**
System wide error code -31 : an overrun has been detected by
a communications driver.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define TTKErrCommsOverrun   -31   




/**
System wide error code -32 : a parity error has occurred in communications.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define TTKErrCommsParity   -32   




/**
System wide error code -33 : an operation has timed out.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define TTKErrTimedOut   -33   




/**
System wide error code -34 : a session could not connect.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define TTKErrCouldNotConnect   -34   




/**
System wide error code -35 : a session could not disconnect.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define TTKErrCouldNotDisconnect   -35   




/**
System wide error code -36 : a function could not be executed because the required
session was disconnected.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define TTKErrDisconnected   -36   




/**
System wide error code -37 : a library entry point was not of the required type.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define TTKErrBadLibraryEntryPoint   -37   




/**
System wide error code -38 : a non-descriptor parameter was passed by
a client interface, when a server expected a descriptor.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define TTKErrBadDescriptor   -38   




/**
System wide error code -39 : an operation has been aborted.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define TTKErrAbort   -39   




/**
System wide error code -40 : a number was too big.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define TTKErrTooBig   -40   




/**
System wide error code -41 : a divide-by-zero operation has been attempted.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define TTKErrDivideByZero   -41   		// Added by AnnW




/**
System wide error code -42 : insufficient power was available to
complete an operation.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define TTKErrBadPower   -42   




/**
System wide error code -43 : an operation on a directory has failed.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define TTKErrDirFull   -43   




/**
System wide error code -44 : an operation cannot be performed because
the necessary hardware is not available.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define TTKErrHardwareNotAvailable   -44   




/**
System wide error code -45 : the completion status when an outstanding
client/server message is completed because a shared session has been closed.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define TTKErrSessionClosed   -45   




/**
System wide error code -46 : an operation cannot be performed due to
a potential security violation.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define TTKErrPermissionDenied   -46   



/**
System wide error code -47 : a requested extension function is not
supported by the object concerned.

*/
#define TTKErrExtensionNotSupported   -47   



/**
System wide error code -48 : a break has occurred in
a communications operation.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define TTKErrCommsBreak   -48   


/**
System wide error code -49 : a trusted time source could not be found
and any time value given in conjunction with this error code should
not be trusted as correct.
*/
#define TTKErrNoSecureTime    -49   

#ifdef __TT_OS_IOS__
/**
System wide error code -50 : assert reader interrupted abnormally when
play ipod song in background state, at the same time user open camera
will occur this error.
 */
#define TTKErrOperationInterrupted -50


/**
System wide error code -51 : when start audio unit failed in the 
background state post this error code, so the UI part can reset its state.
 */
#define TTKErrStartAudioUnitFailed -51

#endif

/**
System wide error code -52 : when wav file is encodered by mp3 and failed to  
parse in the form of PCM,  post this error code
 */
#define TTKErrFormatIsMP3    -52  


/**
System wide error code -53 : when wav file is encodered by DTS and failed to  
parse in the form of PCM,  post this error code
 */
#define TTKErrFormatIsDTS    -53 

/**
System wide error code -54 : url invalid, parse error
 */
#define TTKErrUrlInValid     -54

/**
System wide error code -55 : online music format not support
 */
#define TTKErrOnLineFormatNotSupport   -55

/**
System wide error code -56 : file parse exception, file may has error content or file not support
 */
#define TTKErrFileParserException      -56

/**
System wide error code -57 : read time out
 */
#define TTKErrFileReadTimeout          -57

/**
System wide error code -58 : file can not be supproted to play
 */
#define TTKErrFileNotSupport		   -58

/**
 System wide error code -59 : phone cpu not supoort vfp function
 */
#define TTKErrVFPNotSupport		   -59

/**
 System wide error code -60 : file parse reach out of the range and parse failed
 */
#define TTKErrFilParseOutofRange	   -60

/**
 System wide error code -61 : file read systerm error
 */
#define TTKErrSyncReadErr			 -61

/**
 System wide error code -62 : reach file end and file is incomplete
 */
#define TTKErrSyncEofInComplete      -62

#define TTKErrNeedWait				 -63

#define TTKErrNetWorkAbnormallDisconneted -64

#define TTKErrFormatChanged    			-70
 
#endif
