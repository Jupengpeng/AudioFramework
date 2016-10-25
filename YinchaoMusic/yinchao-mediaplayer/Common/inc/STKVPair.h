#ifndef __ST_KV_PAIR_H__
#define __ST_KV_PAIR_H__
#include <string.h>
#include "STTypedef.h"
#include "STMacrodef.h"


#define KMAX_KVPAIR_KEY_VALUE_LEN   31

class STKVPair
{
public:
	/**
	* 构造函数
	* \param[in] aKey					Key值 以"\0"结束
	* \param[in] aValue					Value值 以"\0"结束
	*/
	STKVPair(const STChar* aKey, const STChar* aValue) 
	{
		STASSERT(aKey != NULL && strlen(aKey) <= KMAX_KVPAIR_KEY_VALUE_LEN);
		STASSERT(aValue != NULL && strlen(aValue) < KMAX_KVPAIR_KEY_VALUE_LEN);

		strcpy(iKey, aKey);
		strcpy(iValue, aValue);
	}

public:
	/**
	* \fn				            STChar* GetKey()	
	* \brief				        获取Key值
	* \return						Key值
	*/
	STChar*		GetKey()
	{
		return iKey;
	}

	/**
	* \fn				            STChar* GetValue()	
	* \brief				        获取Value值
	* \return						Value值
	*/
	STChar*		GetValue()
	{
		return iValue;
	}
	
private:
	STChar		iKey[KMAX_KVPAIR_KEY_VALUE_LEN + 1];
	STChar		iValue[KMAX_KVPAIR_KEY_VALUE_LEN + 1];
};

#endif