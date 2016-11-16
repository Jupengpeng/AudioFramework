#ifndef __ST_DATA_READER_SELECTOR__H__
#define __ST_DATA_READER_SELECTOR__H__
#include "STDataReaderItf.h"
#include "STTypedef.h"
#include <string.h>

class STDataReaderSelector
{
public:
	static ISTDataReaderItf* SelectDataReader(const STChar* aUrl);
};

#endif
