#include "TTHttpMP3Parser.h"

CTTHttpMP3Parser::CTTHttpMP3Parser(ITTDataReader& aDataReader, ITTMediaParserObserver& aObserver)
: CTTMP3Parser(aDataReader, aObserver)
{
}

void CTTHttpMP3Parser::StartFrmPosScan()
{
}

TTInt CTTHttpMP3Parser::RawDataEnd()
{
	return iDataReader.Size();
}