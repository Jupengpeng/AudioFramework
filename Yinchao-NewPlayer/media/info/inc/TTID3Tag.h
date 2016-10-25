#ifndef __TTID3TAG_H__

#include "TTTypedef.h"

class ITTDataReader;


/**
* \fn                       TTInt ID3v2TagSize(ITTDataReader& aDataReader, TTInt aPos = 0);
* \brief                    获取ID3v2信息头长度
* \param [in] aDataReader	ITTDataReader对象引用
* \param [in] aPos			Tag起始位置，默认为0
* \return                   如果存在ID3v2信息头，返回信息头长度(包含KID3V2_TAG_HEADER_BYTES)；如果不存在ID3v2信息头，返回0
*/
TTInt						ID3v2TagSize(ITTDataReader& aDataReader, TTInt aPos = 0);

/**
* \fn                       TTInt ID3v1TagSize(ITTDataReader& aDataReader);
* \brief                    获取ID3v1信息头长度
* \param [in] aDataReader	ITTDataReader对象引用
* \return                   如果存在ID3v1信息头，返回信息头长度(KID3V1_TAG_BYTES)；如果不存在ID3v1信息头，返回0
*/
TTInt						ID3v1TagSize(ITTDataReader& aDataReader);

#endif