
#include "TTID3Tag.h"
#include "TTAPETag.h"
#include "TTDataReaderItf.h"


TTInt APETagSize(ITTDataReader& aDataReader)
{
	APE_TAG_FOOTER APETagFooter;
	if (aDataReader.ReadSync((TTUint8*)&APETagFooter, aDataReader.Size() - ID3v1TagSize(aDataReader) - APE_TAG_FOOTER_BYTES, APE_TAG_FOOTER_BYTES) != APE_TAG_FOOTER_BYTES
		|| !APETagFooter.GetIsValid(ETTFalse))
	{
		return 0;
	}

	return APETagFooter.GetTotalTagBytes();
}

