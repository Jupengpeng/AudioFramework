#include <string.h>
#include "TTMP3Header.h"
#include "TTIntReader.h"

static const short g_nMP3BitRates[2][3][16] =
{
    {
        {0,32,64,96,128,160,192,224,256,288,320,352,384,416,448,-1},
        {0,32,48,56, 64, 80, 96,112,128,160,192,224,256,320,384,-1},
        {0,32,40,48, 56, 64, 80, 96,112,128,160,192,224,256,320,-1}
    },

    {
        {0,32,48,56, 64, 80, 96,112,128,144,160,176,192,224,256,-1},
        {0, 8,16,24, 32, 40, 48, 56, 64, 80, 96,112,128,144,160,-1},
        {0, 8,16,24, 32, 40, 48, 56, 64, 80, 96,112,128,144,160,-1}
    },
};

static const int g_nMP3SampleRates[4][3] =
{
    { 11025, 12000,  8000 },    // MPEG-2.5
    {     0,     0,     0 },
    { 22050, 24000, 16000 },    // MPEG-2
    { 44100, 48000, 32000 }     // MPEG-1
};


CTTMP3Header::CTTMP3Header()
{
}

CTTMP3Header::~CTTMP3Header()
{
}

HEADERTYPE CTTMP3Header::Type()
{
	return MP3CBR_HEADER;
}

TTBool CTTMP3Header::Parse(const TTUint8* /*pbData*/, int /*cbData*/)
{
	return ETTTrue;
}

TTBool CTTMP3Header::MP3CheckHeader(const TTUint8* pbData, MP3_HEADER& mh)
{
    if (pbData[0] == 0xff && (pbData[1] & 0xe0))
    {
        TTUint8* pmh = (TTUint8*)&mh;
        pmh[0] = pbData[3];
        pmh[1] = pbData[2];
        pmh[2] = pbData[1];
        pmh[3] = pbData[0];
        return mh.framesync == MP3_FRAME_INFO_SYNC_FLAG
            && mh.bitrate != 0x0f && mh.bitrate != 0x00
            && mh.mpeglayer != 0x00
            && mh.samplerate != 0x03
            && mh.mpegver != 0x01;
    }
    return ETTFalse;
}

TTBool CTTMP3Header::MP3ParseFrame(MP3_HEADER mh, MP3_FRAME_INFO& mi)
{
    switch (mh.mpegver)
    {
    case 3: mi.nMPEGVersion = MPEGVER_1;    break;
    case 2: mi.nMPEGVersion = MPEGVER_2;    break;
    case 1: mi.nMPEGVersion = MPEGVER_NA;   return ETTFalse;
    case 0: mi.nMPEGVersion = MPEGVER_25;   break;
    }

    mi.nMPEGLayer = 4 - mh.mpeglayer;
    if (mi.nMPEGLayer != 3 &&  mi.nMPEGLayer != 2)//cs modified on 2013.7.17
        return ETTFalse;

    //  read the bitrate, based on the mpeg layer and version
	int nBitRate = 0;
    if (mi.nMPEGVersion == MPEGVER_1)
        nBitRate = g_nMP3BitRates[0][mi.nMPEGLayer - 1][mh.bitrate] * 1000;
    else
        nBitRate = g_nMP3BitRates[1][mi.nMPEGLayer - 1][mh.bitrate] * 1000;

	if (nBitRate != 0)
		mi.nBitRate = nBitRate;
	else
		return ETTFalse;

    if ((mi.nSampleRate = g_nMP3SampleRates[mh.mpegver][mh.samplerate]) == 0)
		return ETTFalse;

    mi.nChannelMode = (ENCHANNELMODE)mh.chanmode;
    mi.nChannels = mi.nChannelMode == MP3CM_SINGLE_CHANNEL ? 1 : 2;

    mi.nEmphasis = (ENEMPHASIS)mh.emphasis;

    //  read the copyright and original bits
    mi.bCopyrighted = mh.copyright;
    mi.bOriginal = mh.original;
    mi.bHasCRC = mh.hascrc;

    //  frame size, in bytes
    if (mi.nBitRate != 0)
    {
        if (mi.nMPEGLayer == 1)
            mi.nFrameSize = (12 * mi.nBitRate / mi.nSampleRate + mh.padding) * 4;
        else
        {
            int nSlotPerFrame = (mi.nMPEGLayer == 3 && mi.nMPEGVersion != MPEGVER_1) ? 72 : 144;
            mi.nFrameSize = (nSlotPerFrame * mi.nBitRate / mi.nSampleRate) + mh.padding;
        }
    }
    else
        return ETTFalse;

	mi.nSamplesPerFrame = (mi.nMPEGVersion == MPEGVER_1) ? MP3_SAMPLES_PER_FRAME : MP3_SAMPLES_PER_FRAME / 2;
	/*
    if (mi.nMPEGLayer == 3)
    {
        if (mi.nMPEGVersion == MPEGVER_1)
            mi.nSamplesPerFrame = MP3_SAMPLES_PER_FRAME;
        else
            mi.nSamplesPerFrame = MP3_SAMPLES_PER_FRAME / 2;
    }
	//非Layer 3 不支持
    else if (mi.nMPEGLayer == 2)
        mi.nSamplesPerFrame = MP3_SAMPLES_PER_FRAME;
    else
        mi.nSamplesPerFrame = MP3_SAMPLES_PER_FRAME / 3;
	*/
    return ETTTrue;
}
/**
* \fn 寻找MP3头 pbData：数据指针，DataSize： 数据大小， SyncOffset帧数据偏移，mi帧信息
*/
TTBool CTTMP3Header::MP3SyncFrameHeader(const TTUint8 *pbData, TTInt DataSize, TTInt &SyncOffset, MP3_FRAME_INFO& mi)
{
	if (DataSize < 4)
	{
		return ETTFalse;
	}

    TTBool bFoundSync = ETTFalse;
    MP3_HEADER mh;

    SyncOffset = DataSize;

    do 
    {
        if (MP3CheckHeader(pbData, mh) && MP3ParseFrame(mh, mi))
        {
            bFoundSync = ETTTrue;
            SyncOffset -= DataSize;
            break;
        }
        pbData++;
        DataSize--;
    } while (DataSize >= 4);

    return bFoundSync;
}


CTTXingHeader::CTTXingHeader()
: m_dwMagic(0)
, m_dwFlags(0)
, m_dwFrames(0)
, m_dwBytes(0)
{
    memset(m_arToc, 0, MP3XING_TOCENTRIES * sizeof(TTUint8));
}

CTTXingHeader::~CTTXingHeader()
{
}

HEADERTYPE CTTXingHeader::Type()
{
    return MP3XING_HEADER;
}

TTBool CTTXingHeader::Parse(const TTUint8* pbData, int cbData)
{
    int nOffset = 0;
    int h_id = (pbData[1] >> 3) & 1;
    int h_mode = (pbData[3] >> 6) & 3;
    if (h_id)   // MPEG-1
    {           
        if (h_mode != 3)
            nOffset = 32 + 4;
        else
            nOffset = 17 + 4;
    }
    else        //  MPEG-2
    {
        if (h_mode != 3)
            nOffset = 17 + 4;
        else
            nOffset = 9 + 4;
    }

    pbData += nOffset;
    cbData -= nOffset;
    if (cbData < 16)
        return ETTFalse;

    m_dwMagic = CTTIntReader::ReadDWord(pbData);
    pbData += 4; cbData -= 4;
    if ((m_dwMagic != MP3XING_MAGIC) && (m_dwMagic !=MP3INFO_MAGIC))
        return ETTFalse;

    m_dwFlags = CTTIntReader::ReadDWord(pbData);
    pbData += 4; cbData -= 4;
    if (m_dwFlags & XING_FRAMES)
    {
        m_dwFrames = CTTIntReader::ReadDWord(pbData);
        pbData += 4; cbData -= 4;
    }
    if (m_dwFlags & XING_BYTES)
    {
        m_dwBytes = CTTIntReader::ReadDWord(pbData); 
        pbData += 4; cbData -= 4;
    }
    if (m_dwFlags & XING_TOC)
    {
        if (cbData < MP3XING_TOCENTRIES)
            return ETTFalse;

        memcpy(m_arToc, pbData, MP3XING_TOCENTRIES);
        pbData += MP3XING_TOCENTRIES;
        cbData -= MP3XING_TOCENTRIES;
    }
    
    return m_dwFrames != 0;
}

CTTVbriHeader::CTTVbriHeader()
: m_dwMagic(0)
, m_wVersion(0)
, m_wDelay(0)
, m_wQuality(0)
, m_dwBytes(0)
, m_dwFrames(0)
, m_wTableSize(0)
, m_wTableScale(0)
, m_wEntryBytes(0)
, m_wEntryFrames(0)
, m_pnTable(NULL)
{
    
}

CTTVbriHeader::~CTTVbriHeader()
{
    if (m_pnTable != NULL)
        delete[] m_pnTable;
}

HEADERTYPE CTTVbriHeader::Type()
{
    return MP3VBRI_HEADER;
}

TTBool CTTVbriHeader::Parse(const TTUint8* pbData, int cbData)
{
    MP3_HEADER mh = {0};
    if (! MP3CheckHeader(pbData, mh))
        return ETTFalse;

    int nOffset = 36;
    pbData += nOffset;
    cbData -= nOffset;
    if (cbData < (int)(sizeof(CTTVbriHeader)))
        return ETTFalse;

    m_dwMagic = CTTIntReader::ReadDWord(pbData); pbData += 4; cbData -= 4;
    if (m_dwMagic != MP3VBRI_MAGIC)
        return ETTFalse;

    m_wVersion      = CTTIntReader::ReadWord(pbData); pbData += 2; cbData -= 2;
    m_wDelay        = CTTIntReader::ReadWord(pbData); pbData += 2; cbData -= 2;
    m_wQuality      = CTTIntReader::ReadWord(pbData); pbData += 2; cbData -= 2;
    m_dwBytes       = CTTIntReader::ReadDWord(pbData); pbData += 4; cbData -= 4;
    m_dwFrames      = CTTIntReader::ReadDWord(pbData); pbData += 4; cbData -= 4;
    m_wTableSize    = CTTIntReader::ReadWord(pbData); pbData += 2; cbData -= 2;
    m_wTableScale   = CTTIntReader::ReadWord(pbData); pbData += 2; cbData -= 2;
    m_wEntryBytes   = CTTIntReader::ReadWord(pbData); pbData += 2; cbData -= 2;
    m_wEntryFrames  = CTTIntReader::ReadWord(pbData); pbData += 2; cbData -= 2;

    if (m_wEntryBytes == 0 || m_wEntryBytes > 4)
        return ETTFalse;

    int nTableLength = m_wTableSize * m_wEntryBytes;
    if (cbData < nTableLength)
        return ETTFalse;

    if (m_pnTable != NULL)
        delete[] m_pnTable;
    m_pnTable = new int[m_wTableSize + 1];
    if (m_pnTable == NULL)
        return ETTFalse;

    TTInt nTotalSize = 0;
	for (TTUint16 i = 0; i <= m_wTableSize; i++)
    {
		m_pnTable[i] = nTotalSize;
		nTotalSize += CTTIntReader::ReadBytesNBE(pbData, m_wEntryBytes) * m_wTableScale;
        pbData += m_wEntryBytes;
    }

    return m_dwFrames != 0;
}

CTTMP3Header* MP3ParseFrameHeader(const TTUint8 *pbData, int aDataSize, MP3_FRAME_INFO& aFrameInfo)
{
	MP3_HEADER mh;
	CTTMP3Header* pHeader = NULL;

	if (CTTMP3Header::MP3CheckHeader(pbData, mh))
	{
		pHeader = new CTTXingHeader();
		TTASSERT(pHeader != NULL);
		if (!pHeader->Parse(pbData,aDataSize))
		{
			SAFE_DELETE(pHeader);

			pHeader = new CTTVbriHeader();
			TTASSERT(pHeader != NULL);
			if(!pHeader->Parse(pbData,aDataSize))
			{
				SAFE_DELETE(pHeader);

				pHeader = new CTTMP3Header();
			}
		}

		CTTMP3Header::MP3ParseFrame(mh, aFrameInfo);  
	}

	return pHeader;
}

//end of file
