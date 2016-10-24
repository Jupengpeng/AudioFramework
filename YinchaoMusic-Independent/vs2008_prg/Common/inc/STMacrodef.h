#ifndef  __ST_MACRODEF_H__
#define  __ST_MACRODEF_H__

#include <assert.h>
#include <stdio.h>

#ifndef ESTTrue
#define ESTTrue					true
#endif

#ifndef ESTFalse
#define ESTFalse				false
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
#define SAFE_RELEASE_ARRAY(p) { for (STInt nIdx = ((p).Count() - 1); nIdx >= 0;	--nIdx)	\
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

#ifndef STASSERT
#define STASSERT(exp)			assert(exp)
#endif

#ifndef STMAX
#define STMAX(a,b)	((a) > (b) ? (a) : (b))
#endif

#ifndef STMIN
#define STMIN(a,b)	((a) < (b) ? (a) : (b))
#endif

#ifndef CLIP
#define CLIP(x, a, b)			{					\
                                    if ((x) < (a))	\
                                    (x) = (a);	\
                                    if ((x) > (b))	\
                                    (x) = (b);	\
                                }
#endif	

/**
System wide error code 0 : this represents the no-error condition.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define STKErrNone			0

/**
System wide error code -1 : item not found.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define STKErrNotFound		-1 // Must remain set to -1

/**
System wide error code -2 : an error that has no specific categorisation.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define STKErrGeneral			-2

/**
System wide error code -3 : indicates an operation that has been cancelled.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define STKErrCancel          -3

/**
System wide error code -4 : an attempt to allocate memory has failed.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define STKErrNoMemory        -4

/**
System wide error code -5 : some functionality is not supported in a given context.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.

There may be many reasons for this; for example, a device may not support
some specific behaviour.
*/
#define STKErrNotSupported	-5

/**
System wide error code -6 : an argument is out of range.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define STKErrArgument		-6

/**
System wide error code -7 : a calculation has lost precision.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.

This error arises when converting from an internal 96-bit real representation
to a TReal32; the exponent of the internal representation is so small
that the 32-bit real cannot contain it.
*/
#define STKErrTotalLossOfPrecision -7

/**
System wide error code -8 : an invalid handle has been passed.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.

A function involving a resource owned by a server or the kernel has
specified an invalid handle.
*/
#define STKErrBadHandle			-8




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
#define STKErrOverflow            -9




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
#define STKErrUnderflow			-10




/**
System wide error code -11 : an object already exists.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.

An object of some name/type is to be created, but an object of
that name/type already exists.
*/
#define STKErrAlreadyExists       -11




/**
System wide error code -12 : in the context of file operations, a path
was not found.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define STKErrPathNotFound		-12




/**
System wide error code -13 : a handle refers to a thread that has died.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define STKErrDied				-13

/**
System wide error code -14 : a requested resource is already in exclusive use.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define STKErrInUse				-14

/**
System wide error code -15 : client/server send/receive operation cannot run,
because the server has terminated.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define STKErrServerTerminated	-15

/**
System wide error code -16 : a client/server send/receive operation cannot run,
because the server is busy handling another request.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define STKErrServerBusy			-16




/**
System wide error code -17 : indicates that an operation is complete,
successfully or otherwise.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.

This code may be used to indicate that some follow on operation can take place.
It does not necessarily indicate an error condition.
*/
#define STKErrCompletion			-17

/**
System wide error code -18 : indicates that a device required by an i/o operation
is not ready to start operations.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.

A common reason for returning this code is because a device has not been
initialised, or has no power.
*/
#define STKErrNotReady			-18




/**
System wide error code -19 : a device is of unknown type.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define STKErrUnknown				-19




/**
System wide error code -20 : indicates that some media is not formatted properly,
or links between sections of it have been corrupted.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define STKErrCorrupt				-20




/**
System wide error code -21 : access to a file is denied, because the permissions on
the file do not allow the requested operation to be performed.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define STKErrAccessDenied		-21




/**
System wide error code -22 : an operation cannot be performed, because the part
of the file to be read or written is locked.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define STKErrLocked				-22




/**
System wide error code -23 : during a file write operation, not all the data
could be written.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define STKErrWrite				-23




/**
System wide error code -24 : a volume which was to be used for a file system
operation has been dismounted.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define STKErrDisMounted			-24   




/**
System wide error code -25 : indicates that end of file has been reached.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.

Note that RFile::Read() is a higher-level interface. When the end of
the file is reached, it returns zero bytes in the destination descriptor, and
a STKErrNone return value. STKErrEof is not used for this purpose; other error
conditions are returned only if some other error condition was indicated on
the file.
*/
#define STKErrEof					-25   




/**
System wide error code -26 : a write operation cannot complete, because the disk
is full.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define STKErrDiskFull			-26   




/**
System wide error code -27 : a driver DLL is of the wrong type.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define STKErrBadDriver			-27   




/**
System wide error code -28 : a file name or other object name does not conform to
the required syntax.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define STKErrBadName				 -28   




/**
System wide error code -29 : a communication line has failed.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define STKErrCommsLineFail		  -29   




/**
System wide error code -30 : a frame error has occurred in
a communications operation.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define STKErrCommsFrame				-30   




/**
System wide error code -31 : an overrun has been detected by
a communications driver.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define STKErrCommsOverrun   -31   




/**
System wide error code -32 : a parity error has occurred in communications.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define STKErrCommsParity   -32   




/**
System wide error code -33 : an operation has timed out.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define STKErrTimedOut   -33   




/**
System wide error code -34 : a session could not connect.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define STKErrCouldNotConnect   -34   




/**
System wide error code -35 : a session could not disconnect.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define STKErrCouldNotDisconnect   -35   




/**
System wide error code -36 : a function could not be executed because the required
session was disconnected.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define STKErrDisconnected   -36   




/**
System wide error code -37 : a library entry point was not of the required type.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define STKErrBadLibraryEntryPoint   -37   




/**
System wide error code -38 : a non-descriptor parameter was passed by
a client interface, when a server expected a descriptor.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define STKErrBadDescriptor   -38   




/**
System wide error code -39 : an operation has been aborted.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define STKErrAbort   -39   




/**
System wide error code -40 : a number was too big.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define STKErrTooBig   -40   




/**
System wide error code -41 : a divide-by-zero operation has been attempted.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define STKErrDivideByZero   -41   		// Added by AnnW




/**
System wide error code -42 : insufficient power was available to
complete an operation.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define STKErrBadPower   -42   




/**
System wide error code -43 : an operation on a directory has failed.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define STKErrDirFull   -43   




/**
System wide error code -44 : an operation cannot be performed because
the necessary hardware is not available.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define STKErrHardwareNotAvailable   -44   




/**
System wide error code -45 : the completion status when an outstanding
client/server message is completed because a shared session has been closed.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define STKErrSessionClosed   -45   




/**
System wide error code -46 : an operation cannot be performed due to
a potential security violation.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define STKErrPermissionDenied   -46   



/**
System wide error code -47 : a requested extension function is not
supported by the object concerned.

*/
#define STKErrExtensionNotSupported   -47   



/**
System wide error code -48 : a break has occurred in
a communications operation.

A system wide error code indicates an error in the environment, or in
user input from which a program may recover.
*/
#define STKErrCommsBreak   -48   


/**
System wide error code -49 : a trusted time source could not be found
and any time value given in conjunction with this error code should 
not be trusted as correct.
*/
#define STKErrNoSecureTime    -49   

#endif
