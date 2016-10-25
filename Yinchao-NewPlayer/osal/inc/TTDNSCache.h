#ifndef __TT_DNSCACHE_H__
#define __TT_DNSCACHE_H__

// INCLUDES
#include "TTTypedef.h"
#include "TTMacrodef.h"

#define IP_NOT_FOUND 0

typedef struct DNSNode
{
    TTChar*    hostName;
    TTUint32   ipAddress;
    DNSNode* next;
    DNSNode()
    {
        hostName = NULL;
        ipAddress = 0;
        next = NULL;
    }
} DNSNode, *PDNSNode;

// CLASSES DECLEARATION
class CTTDNSCache
{
public:

	CTTDNSCache();

	~CTTDNSCache();

	/**
	* \fn                       void put(TTChar* hostName, TTUint32 ipAddress);
	* \brief                    保存[域名,ip地址]到cache中去
	* \param[in]	hostName	域名	
	* \param[in]	ipAddress	ip地址
	*/
	void put(TTChar* hostName, TTUint32 ipAddress);

	/**
	* \fn                       void del(TTChar* hostName);
	* \brief                    在cache中根据域名删除ip地址
	* \param[in]	hostName	域名	
	*/
	void del(TTChar* hostName);

	/**
	* \fn                       TTUint32 get(TTChar*  hostName);
	* \brief                    在cache中根据域名查找ip地址
	* \param[in]	hostName	域名	
	* \return					ip地址
	*/
	TTUint32 get(TTChar*  hostName);

private:
	DNSNode*  iDNSList;
};


#endif
