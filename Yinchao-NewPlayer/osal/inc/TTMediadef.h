#ifndef __TT_MEDIADEF_H__
#define __TT_MEDIADEF_H__

#include "TTTypedef.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define	TT_PID_COMMON_BASE				0x00000000	
#define	TT_PID_COMMON_JAVAVM			(TT_PID_COMMON_BASE | 0X0001)		
#define	TT_PID_COMMON_SURFACEOBJ		(TT_PID_COMMON_BASE | 0X0002)		
#define	TT_PID_COMMON_DATASOURCE		(TT_PID_COMMON_BASE | 0X0003)		
#define	TT_PID_COMMON_STATUSCODE		(TT_PID_COMMON_BASE | 0X0004)		
#define	TT_PID_COMMON_HOSTIP			(TT_PID_COMMON_BASE | 0X0005)		


/**
 * Enumeration of buffer flags, used in struct TTBuffer
 */
typedef enum
{
    TT_FLAG_BUFFER_KEYFRAME             = 0X00000001,       /*!< Buffer is key frame */
    TT_FLAG_BUFFER_TIMESTAMP_RESET      = 0X00000002,       /*!< Sample timestamp rollback */
	TT_FLAG_BUFFER_HEADDATA             = 0X00000004,       /*!< Buffer is header data */
	TT_FLAG_BUFFER_FLUSH	            = 0X00000008,       /*!< flush decoder */

	TT_FLAG_BUFFER_NEW_PROGRAM          = 0X00000010,       /*!< Buffer starts new program, codec is changed, bitrate or sample rate is changed, timestamp is reset, pData point to TTAudioInfo/TTVideoInfo, pBuffer is NULL */
    TT_FLAG_BUFFER_NEW_FORMAT           = 0X00000020,       /*!< Buffer starts new format, codec is not changed, pData point to TTAudioInfo/TTVideoInfo, pBuffer is NULL */
    
    TT_FLAG_BUFFER_DROP_FRAME           = 0X00000100,       /*!< Previous buffer was dropped */
    TT_FLAG_BUFFER_DECODE_ONLY			= 0x00000200,       /*!< Buffer shall only be decoded, but not be rendered */
	TT_FLAG_BUFFER_SEEKING				= 0x00000400,       /*!< tell the hardware decoder, we should seeking */

	TT_FLAG_BUFFER_EOS					= 0x00001000,       /*!< Buffer shall be the end of file */

    TT_FLAG_BUFFER_MAX                  = 0X7FFFFFFF        /*!< Max value definition */
}TT_BUFFER_FLAG;

/**
 * General data buffer, used as input or output
 */
typedef struct
{
    TTInt32         nFlag;          /*!< Flag of the buffer, refer to TT_BUFFER_FLAG
                                        TT_FLAG_BUFFER_KEYFRAME:    pBuffer is video key frame
                                        TT_FLAG_BUFFER_HEADDATA:    pBuffer is head data  
										TT_FLAG_BUFFER_NEW_PROGRAM: pBuffer is NULL, pData is TTAudioInfo/TTVideoInfo
                                        TT_FLAG_BUFFER_NEW_FORMAT:  pBuffer is NULL, pData is TTAudioInfo/TTVideoInfo  */
	TTInt32         nSize;          /*!< Buffer size in byte */
    TTPBYTE			pBuffer;        /*!< Buffer pointer */
    TTInt64	        llTime;         /*!< [in/out] The time of the buffer */
    TTInt32         nDuration;      /*!< [In]AV offset, [out]Duration of buffer(MS) */  

    TTPtr           pData;          /*!< Special data pointer, depends on the flag */
    TTInt32		    lReserve;       /*!< Reserve value */
	TTPtr		    pReserve;		/*!< Reserve value */
}TTBuffer;


/**
 * General audio format info
 */
typedef struct
{
	TTInt32		SampleRate;  /*!< Sample rate */
	TTInt32		Channels;    /*!< Channel count */
	TTInt32		SampleBits;  /*!< Bits per sample */
	TTInt32		nReserved;	 /*!< nReserved */
} TTAudioFormat;

/**
 * General video format info
 */
typedef struct
{
	TTInt32					Width;		 /*!< Width */
	TTInt32					Height;		 /*!< Height */
	TTInt32					Type;		 /*!< Color type  */
	TTInt32					nReserved;	 /*!< nReserved */
} TTVideoFormat;

/**
 * Definition of rect structure
 */
typedef struct
{
    TTInt32                 nLeft;      /*!< Left */
    TTInt32                 nTop;       /*!< top */
    TTInt32                 nRight;     /*!< right */
    TTInt32                 nBottom;    /*!< bottom */
}TTRect;

/**
 * Definition of video decoder type
 */
typedef enum
{
	TT_VIDEODEC_SOFTWARE			= 0,		/*!< software decoder */
	TT_VIDEODEC_IOMX_ICS			= 1,		/*!< IOMX ICS decoder  */
	TT_VIDEODEC_IOMX_JB				= 2,		/*!< IOMX JB decoder  */
	TT_VIDEODEC_MediaCODEC_JAVA		= 3,		/*!< MediaCodec Java decoder  */
	TT_VIDEODEC_MediaCODEC_NDK		= 4,		/*!< MediaCodec ndk decoder  */

	TT_VIDEODEC_MAX					= 0X7FFFFFF
}  TT_DECODER_TYPE;

/**
 * Definition of color format
 */
typedef enum
{
	TT_COLOR_YUV_PLANAR420			= 0,		/*!< YUV planar mode:420  vertical sample is 2, horizontal is 2  */
	TT_COLOR_YUV_NV12				= 1,		/*!< YUV planar mode:420  vertical sample is 2, horizontal is 2, uv interleave  */
	TT_COLOR_YUV_NV21				= 2,		/*!< YUV planar mode:420  vertical sample is 2, horizontal is 2, vu interleave  */
	TT_COLOR_YUV_YUYV422			= 10,		/*!< YUV planar mode:420  vertical sample is 1, horizontal is 2, Y U Y V  */
	TT_COLOR_YUV_UYVY422			= 11,		/*!< YUV planar mode:420  vertical sample is 1, horizontal is 2, U Y V Y  */

	TT_COLOR_ARGB32_PACKED			= 30,		/*!< ARGB packed mode, data: B G R A							 */
	TT_COLOR_RGB888_PACKED			= 31,		/*!< RGB packed mode, data: B G R		 						 */
	TT_COLOR_RGB565_PACKED			= 32,		/*!< RGB packed mode, data: B:5 G:6 R:5   						 */
	TT_COLOR_RGB555_PACKED			= 33,		/*!< RGB packed mode, data: B:5 G:5 R:5   						 */

	TT_COLOR_TYPE_MAX					= 0X7FFFFFF
}  TT_COLORTYPE;

/**
 * Video data buffer, usually used as input or output of video codec.
 */
typedef struct
{
	TTPBYTE	 			Buffer[3];			/*!< Buffer pointer */
	TTInt32				Stride[3];			/*!< Buffer stride */
	TTInt32				ColorType;			/*!< Color Type */
	TTInt32				nFlag;				/*!< Video flag */
	TTInt64				Time;				/*!< The time of the video buffer */
    TTInt32				lReserve;       /*!< Reserve value */
	TTPtr				pReserve;		/*!< Reserve value */
} TTVideoBuffer;

typedef struct
{
	TTInt64	nStartTime;
	TTInt64	nStopTime;
	TTBool	bEnable;
}TTPlayRange;

typedef int (* _Observer) (void * pUserData, TTInt32 nID, TTInt32 nParam1, TTInt32 nParam2, void * pParam3);

typedef struct __TTObserver {

	_Observer		pObserver;
	void*			pUserData;
}TTObserver;

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif // __TT_MEDIADEF_H__
