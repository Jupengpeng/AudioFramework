#include "STOSConfig.h"
#include "STDataReaderSelector.h"
#include "STFileReader.h"


ISTDataReaderItf* STDataReaderSelector::SelectDataReader(const STChar* aUrl)
{	
#ifndef __ST_OS_WINDOWS__
	if (!memcmp(aUrl, "http://", 7))
	{
		return (ISTDataReaderItf*)new STHttpReader();
	}
	else
	{
		return (ISTDataReaderItf*)new STFileReader();
	}
#else
	return (ISTDataReaderItf*)new STFileReader();
#endif
}
