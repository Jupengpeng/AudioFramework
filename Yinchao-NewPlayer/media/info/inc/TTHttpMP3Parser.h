#ifndef __TT_HTTP_MP3_PARSER_H__
#define __TT_HTTP_MP3_PARSER_H__

#include "TTMP3Parser.h"

class CTTHttpMP3Parser : public CTTMP3Parser
{
public:
	CTTHttpMP3Parser(ITTDataReader& aDataReader, ITTMediaParserObserver& aObserver);

public: // Functions from ITTDataParser

	/**
	* \fn                           void StartFrmPosScan();
	* \brief                        开始扫描帧位置，建立帧索引表
	*/
	virtual void                    StartFrmPosScan();

	/**
	* \fn							RawDataEnd();
	* \brief						解析数据尾部位置
	* \return						数据尾部在文件中的偏移。
	*/
	virtual TTInt					RawDataEnd();
};

#endif