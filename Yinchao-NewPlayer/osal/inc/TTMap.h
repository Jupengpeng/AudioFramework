#ifndef __TT_CMAP_H__
#define __TT_CMAP_H__

// INCLUDES
#include "TTTypedef.h"
#include "TTMacrodef.h"


// CLASSES DECLEARATION
template<class T,class S>
class CTTMap
{
public:

	CTTMap();

	~CTTMap();

	/**
	* \fn                       void put(T key, S target)
	* \brief                    保存[key,target]
	* \param[in]	key	域名	
	* \param[in]    target	 
	*/
	void Put(T key, S target);

	/**
	* \fn                       void del(T key);
	* \brief                    删除key对应的target
	* \param[in]	key		
	*/
	void Del(T key);

	/**
	* \fn                       S get(T key);
	* \brief                    根据key查找target
	* \param[in]	key	 	
	* \return		target
	*/
	S Get(T key);


private:
	typedef struct Node
	{
		T    key;
		S    target;
		Node* next;
		Node()
		{
			key = 0;
			target = 0;
			next = NULL;
		}
	} Node, *PNode;

private:
	PNode		iList;
	RTTCritical	iCritical;
};


template<class T,class S>
CTTMap<T,S>::CTTMap()
{
	iList = NULL;
	iCritical.Create();	
}

template<class T,class S>
CTTMap<T,S>::~CTTMap()
{
	iCritical.Lock();
	if (iList != NULL)
	{
		PNode pNext = iList;
		PNode pCurr;
		while (pNext)
		{
			pCurr = pNext;
			pNext = pNext->next;
			delete pCurr;
		}
		iList = NULL;
	}
	iCritical.UnLock();

	iCritical.Destroy();
}

template<class T,class S>
void CTTMap<T,S>::Put(T key, S target)
{
	iCritical.Lock();
 	PNode ptr = iList;
	PNode ptrPrev = iList;

	while (ptr != NULL)
	{
		ptrPrev = ptr;
		ptr = ptr->next;
	}

	if(iList == ptr)
	{
		iList = new Node;
		ptr = iList;
	}
	else
	{
		ptrPrev->next = new Node;
		ptr = ptrPrev->next;
	}

	ptr->key = key;
	ptr->target = target;
	iCritical.UnLock();
}

template<class T,class S>
void CTTMap<T,S>::Del(T key)
{
	iCritical.Lock();
	PNode ptr = iList;
	PNode ptrPrev = ptr;

	if (ptr == NULL)
	{
		return ;
	}

	while (ptr != NULL)
	{ 
		if (ptr->key == key)
		{
			//handle the first item
			if (ptr == iList)
				iList = ptr->next;
			else
				ptrPrev->next = ptr->next;

			delete ptr;
			break;
		}

		ptrPrev = ptr;
		ptr = ptr->next;
	}
	iCritical.UnLock();
}

template<class T,class S>
S CTTMap<T,S>::Get(T key)
{
	PNode ptr = iList;
    S nret = NULL;
	while (ptr)
	{
		if (ptr->key == key)
		{
			nret = ptr->target;
			break;
		}
		else
			ptr = ptr->next;
	}

	return nret;
}


#endif
