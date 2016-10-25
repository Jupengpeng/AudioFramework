#ifndef __TT_MEDIA_INFO_DEF_H__
#define __TT_MEDIA_INFO_DEF_H__

#include "TTArray.h"

enum TTMediaType//音视频类型
{
	EMediaTypeNone = 0
	, EMediaTypeAudio = 1
	, EMediaTypeVideo = 2
};


enum TTMediaStreamId
{
	EMediaStreamIdNone = -1
	, EMediaStreamIdAudioL = 0//这里对于与KAudioIdElementIdMap[0]
	, EMediaStreamIdAudioR	= 1	//这里对于与KAudioIdElementIdMap[1]
	, EMediaStreamIdVideo = 100 //这里对于与KAudioIdElementIdMap[2]
	, EMediaStreamMaxCount = 0x7fffffff
};

class TTVideoInfo
{
public:

	//Standard FourCC codes
	static const TTUint32 KTTMediaTypeVideoCodeHEVC = MAKEFOURCC('H', 'E', 'V', 'C');
	static const TTUint32 KTTMediaTypeVideoCodeH264 = MAKEFOURCC('H', '2', '6', '4');
	static const TTUint32 KTTMediaTypeVideoCodeMPEG4 = MAKEFOURCC('M', 'P', '4', 'V');

	TTVideoInfo()
		: iFrameRate(0)
		, iBitRate(0)
		, iWidth(0)
		, iHeight(0)
		, iStreamId(EMediaStreamIdNone)
		, iDuration(0)
		, iFourCC(0)
		, iMaxVideoSize(0)
		, iDecInfo(NULL)
	{
	};

public:
	TTInt iFrameRate;
	TTInt iBitRate;
	TTInt iWidth;
	TTInt iHeight;
	TTInt iStreamId;
	TTInt64 iDuration;
	TTUint32	iMediaTypeVideoCode;
	TTInt		iFourCC;
	TTInt		iMaxVideoSize;
	void*		iDecInfo;
};

class TTAudioInfo
{
public:

	//Standard FourCC codes

	/** 8-bit PCM
	*/
	static const TTUint32 KTTMediaTypeAudioCodePCM8 = 0x38502020;		//( ' ', ' ', 'P', '8' )

	/** 8-bit unsigned PCM
	*/
	static const TTUint32 KTTMediaTypeAudioCodePCMU8 = 0x38555020;		//(' ', 'P', 'U', '8') 

	/** 16-bit PCM
	*/
	static const TTUint32 KTTMediaTypeAudioCodePCM16 = 0x36315020;		//(' ', 'P', '1', '6')

	static const TTUint32 KTTMediaTypeAudioCodeWAV = 0x56415720;		//(' ', 'W', 'A', 'V')


	static const TTUint32 KTTMediaTypeAudioCodeAPE = 0x45504120;		//(' ', 'A', 'P', 'E')

	static const TTUint32 KTTMediaTypeAudioCodeALAC = 0x43414C41;		//('A', 'L', 'A', 'C')

	/** Most-Significant-Byte-first (big endian) 16-bit PCM
	*/
	static const TTUint32 KTTMediaTypeAudioCodePCM16B = 0x42363150;		//('P', '1', '6', 'B')

	/** Most-Significant-Byte-first  (big endian)16-bit unsigned PCM
	*/
	static const TTUint32 KTTMediaTypeAudioCodeFLAC = 0x43414C46;		//('F', 'L', 'A', 'C')

	/** IMA ADPCM
	*/
	static const TTUint32 KTTMediaTypeAudioCodeIMAD = 0x44414d49;		//('I', 'M', 'A', 'D')

	/** IMA stereo ADPCM
	*/
	static const TTUint32 KTTMediaTypeAudioCodeIMAS = 0x53414d49;		//('I', 'M', 'A', 'S')

	/** ALAW
	*/
	static const TTUint32 KTTMediaTypeAudioCodeALAW = 0x57414c41;		//('A', 'L', 'A', 'W')

	/** MuLAW
	*/
	static const TTUint32 KTTMediaTypeAudioCodeMuLAW = 0x57414c75;		//('u', 'L', 'A', 'W')

	/** GSM 610
	*/
	static const TTUint32 KTTMediaTypeAudioCodeGSM610 = 0x364d5347;		//('G', 'S', 'M', '6')

	/** FourCC used if the actual fourCC is not known or not applicable
	*/
	static const TTUint32 KTTMediaTypeAudioCodeNULL = 0x4c4c554e;		//('N', 'U', 'L', 'L')

	/** AMR
	*/
	static const TTUint32 KTTMediaTypeAudioCodeAMR = 0x524d4120;		//(' ',' A', 'M', 'R')

	/** Advanced Audio Codec (MPEG4, Audio layer 3)
	*/
	static const TTUint32 KTTMediaTypeAudioCodeAAC = 0x43414120;		//(' ', 'A', 'A', 'C')

	/** Wideband AMR
	*/
	static const TTUint32 KTTMediaTypeAudioCodeAWB = 0x42574120;		//(' ', 'A', 'W', 'B')

	/** MPEG, Audio layer 3
	*/
	static const TTUint32 KTTMediaTypeAudioCodeMP3 = 0x33504d20;		//(' ', 'M', 'P', '3')

	/** ATRAC3
	*/
	static const TTUint32 KTTMediaTypeAudioCodeATRAC3 = 0x33525441;		//('A', 'T', 'R', '3')

	/** SBC
	*/
	static const TTUint32 KTTMediaTypeAudioCodeSBC = 0x43425320;		//(' ' ,'S', 'B', 'C')

	/** AMR Wideband
	*/
	static const TTUint32 KTTMediaTypeAudioCodeAMRW = 0x57524d41;		//('A', 'M', 'R', 'W')

	/** AAC Variant ADTS
	*/
	static const TTUint32 KTTMediaTypeAudioCodeADTS = 0x53544441;		//('A', 'D', 'T', 'S')

	/** Windows Media Audio  (WMA)
	*/
	static const TTUint32 KTTMediaTypeAudioCodeWMA = 0x414d5720;		//(' ', 'W', 'M', 'A')

	static const TTUint32 KTTMediaTypeAudioCodeWMAFLOAT = 0x414d5721;	//(' ', 'W', 'M', 'A')


	static const TTUint32 KTTMediaTypeAudioCodeWMAPRO = 0x414d5722;		//(' ', 'W', 'M', 'A')
    
    /** APPLE Pure Frame
     */
	static const TTUint32 KTTMediaTypeAudioCodeIPodLibrary = 0x414d5723;		

		/**DTS
	*/
	static const TTUint32 KTTMediaTypeAudioCodeDTS = 0x53544420;		//(' ', 'D', 'T', 'S')

public:
	TTAudioInfo()
		: iSampleRate(0)
		, iChannel(0)
		, iBitRate(0)
		, iDuration(0)
		, iFourCC(0)
		, iMaxAudioSize(0)
		, iStreamId(EMediaStreamIdNone)
		, iDecInfo(NULL)
	{
		memset(iLanguage, 0, sizeof(TTChar)*4);
	};

public:
	TTInt		iSampleRate;
	TTInt		iChannel;
	TTInt		iBitRate;
	TTInt		iStreamId;
	TTInt64		iDuration;
	TTUint32	iMediaTypeAudioCode;
	TTInt		iFourCC;
	TTInt		iMaxAudioSize;
	TTChar		iLanguage[4];
	void*		iDecInfo;
};

// INCLUDES
class TTMediaInfo
{
public:
	TTMediaInfo()
		: iVideoInfo(NULL)
		, iAudioInfoArray(2)
	{

	};

	~TTMediaInfo()
	{
 		iAudioInfoArray.ResetAndDestroy();
 		iAudioInfoArray.Close();
 		SAFE_DELETE(iVideoInfo);
	};

	void Reset()
	{
		SAFE_DELETE(iVideoInfo);
		iAudioInfoArray.ResetAndDestroy();
	};

public:
	TTVideoInfo*				 iVideoInfo;
	RTTPointerArray<TTAudioInfo> iAudioInfoArray;
};

#endif
