// INCLUDES
#include <stdio.h>
#include <string.h>
#include "TTLog.h"
#include "TTMacrodef.h"
#include "TTOsalConfig.h"
#include "TTHttpReader.h"
#include "TTHttpClient.h"
#include "TTHttpCacheFile.h"
#include "TTHttpReaderProxy.h"

//CTTHttpReaderProxy*	CTTHttpReader::iHttpReaderProxy = NULL;

CTTHttpReader::CTTHttpReader()
{
	iHttpReaderProxy = new CTTHttpReaderProxy();
}

CTTHttpReader::~CTTHttpReader()
{
	if (iHttpReaderProxy != NULL)
	{
		if (iHttpReaderProxy->Release() == 0)
		{
			iHttpReaderProxy = NULL;
		}
	}
}

TTInt CTTHttpReader::Open(const TTChar* aUrl)
{
	if (iHttpReaderProxy != NULL)
	{
		return iHttpReaderProxy->Open(aUrl);
	}

	return TTKErrNotReady;
}

TTInt CTTHttpReader::Close()
{	
	if (iHttpReaderProxy != NULL)
	{
		iHttpReaderProxy->Close();
	}
	return TTKErrNone;
}

TTInt CTTHttpReader::ReadSync(TTUint8* aReadBuffer, TTInt aReadPos, TTInt aReadSize)
{
	return iHttpReaderProxy->Read(aReadBuffer, aReadPos, aReadSize);
}

TTInt CTTHttpReader::ReadWait(TTUint8* aReadBuffer, TTInt aReadPos, TTInt aReadSize)
{
	return iHttpReaderProxy->ReadWait(aReadBuffer, aReadPos, aReadSize);
}

TTInt CTTHttpReader::Size() const
{
	return iHttpReaderProxy->Size();
}

TTUint16 CTTHttpReader::ReadUint16(TTInt aReadPos)
{
	TTUint8 buf[sizeof(TTUint16)];
	if (sizeof(TTUint16) == ReadSync(buf, aReadPos, sizeof(TTUint16)))
	{
		return TTUint16((buf[1] << 8) | buf[0]);
	}
	return 0;
}

TTUint16 CTTHttpReader::ReadUint16BE(TTInt aReadPos)
{
	TTUint8 buf[sizeof(TTUint16)];
	if (sizeof(TTUint16) == ReadSync(buf, aReadPos, sizeof(TTUint16)))
	{
		return TTUint16((buf[0] << 8) | buf[1]);
	}
	return 0;
}

TTUint32 CTTHttpReader::ReadUint32(TTInt aReadPos)
{
	TTUint8 buf[sizeof(TTUint32)];
	if (sizeof(TTUint32) == ReadSync(buf, aReadPos, sizeof(TTUint32)))
	{
		return TTUint32((buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | buf[0]);
	}
	return 0;
}

TTUint32 CTTHttpReader::ReadUint32BE(TTInt aReadPos)
{
	TTUint8 buf[sizeof(TTUint32)];
	if (sizeof(TTUint32) == ReadSync(buf, aReadPos, sizeof(TTUint32)))
	{
		return TTUint32((buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3]);
	}
	return 0;
}

TTUint64 CTTHttpReader::ReadUint64(TTInt aReadPos)
{
	TTUint32 nMsb = ReadUint32(aReadPos);
	TTUint32 nLsb = ReadUint32(aReadPos + 4);

	TTUint64 nRetVal = nMsb;
	return (nRetVal << 32) | nLsb;
}

TTUint64 CTTHttpReader::ReadUint64BE(TTInt aReadPos)
{
	TTUint32 nLsb = ReadUint32(aReadPos);
	TTUint32 nMsb = ReadUint32(aReadPos + 4);

	TTUint64 nRetVal = nMsb;
	return (nRetVal << 32) | nLsb;
}

ITTDataReader::TTDataReaderId CTTHttpReader::Id()
{
	return ITTDataReader::ETTDataReaderIdHttp;
}

TTUint CTTHttpReader::BufferedSize()
{
	TTASSERT(iHttpReaderProxy != NULL);
	return iHttpReaderProxy->BufferedSize();
}

TTUint CTTHttpReader::ProxySize()
{
	TTASSERT(iHttpReaderProxy != NULL);
	return iHttpReaderProxy->ProxySize();
}

TTUint CTTHttpReader::BandWidth()
{
	return iHttpReaderProxy->BandWidth();
}

void CTTHttpReader::SetStreamBufferingObserver(ITTStreamBufferingObserver* aObserver)
{
	TTASSERT(iHttpReaderProxy != NULL);
	//LOGI("CTTHttpReader::SetStreamBufferingObserver. iHttpReaderProxy = %p, aObserver = %p", iHttpReaderProxy, aObserver);
	iHttpReaderProxy->SetStreamBufferingObserver(aObserver);	
	//LOGI("CTTHttpReader::SetStreamBufferingObserver return");
}

void CTTHttpReader::CloseConnection()
{
	if (iHttpReaderProxy != NULL)
		iHttpReaderProxy->Cancel();
}

void CTTHttpReader::CheckOnLineBuffering()
{
    if (iHttpReaderProxy != NULL)
		iHttpReaderProxy->CheckOnLineBuffering();
}

TTInt CTTHttpReader::PrepareCache(TTInt aReadPos, TTInt aReadSize, TTInt aFlag)
{
	if (iHttpReaderProxy != NULL)
		return iHttpReaderProxy->PrepareCache(aReadPos, aReadSize, aFlag);
		
	return TTKErrNotReady;
}

void CTTHttpReader::SetDownSpeed(TTInt aFast)
{
    if (iHttpReaderProxy != NULL)
		iHttpReaderProxy->SetDownSpeed(aFast);
}

void CTTHttpReader::SetBitrate(TTInt aMediaType, TTInt aBitrate)
{
    if (iHttpReaderProxy != NULL)
		iHttpReaderProxy->SetBitrate(aMediaType, aBitrate);
}

void CTTHttpReader::SetNetWorkProxy(TTBool aNetWorkProxy)
{
	if (iHttpReaderProxy != NULL)
		iHttpReaderProxy->SetNetWorkProxy(aNetWorkProxy);
}

TTInt CTTHttpReader::GetStatusCode()
{
	if (iHttpReaderProxy != NULL)
		return iHttpReaderProxy->GetStatusCode();

	return 0;
}

TTUint CTTHttpReader::GetHostIP()
{
	if (iHttpReaderProxy != NULL)
		return iHttpReaderProxy->GetHostIP();

	return 0;
}
