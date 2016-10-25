// INCLUDES
#include "TTBaseDataReader.h"
//#include "TTActiveScheduler.h"


CTTBaseDataReader::CTTBaseDataReader()
: iAudioBitrate(0)
, iVideoBitrate(0)
{
}

CTTBaseDataReader::~CTTBaseDataReader()
{
}


void CTTBaseDataReader::SetBitrate(TTInt aMediaType, TTInt aBitrate)
{
	if(aMediaType == 1) {
		iAudioBitrate = aBitrate;
	}else if(aMediaType == 2) {
		iVideoBitrate = aBitrate;
	}
}

void CTTBaseDataReader::SetNetWorkProxy(TTBool aNetWorkProxy)
{
	iUseProxy = aNetWorkProxy;
}

TTUint CTTBaseDataReader::ProxySize()
{
	return 0;
}

TTUint CTTBaseDataReader::BandWidth()
{
	return 0;	
}

TTUint CTTBaseDataReader::BandPercent()
{
	return 100;
}

TTInt CTTBaseDataReader::GetStatusCode()
{
	return 0;
}

TTUint CTTBaseDataReader::GetHostIP()
{
	return 0;
}