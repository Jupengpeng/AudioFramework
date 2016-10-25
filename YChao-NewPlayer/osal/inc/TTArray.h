
#ifndef __TT_ARRAY_H__
#define __TT_ARRAY_H__

// INCLUDES
#include <stdlib.h>
#include <string.h>
#include "TTMacrodef.h"
#include "TTTypedef.h"


static const TTInt KDefaultGranularity = 16;
static const TTInt KGranularityIncPace = 8;

template<class T>
class RTTPointerArray
{
public:
	
	/**
	* \fn                       RTTPointerArray()
	* \brief                    构造函数
	*/
	RTTPointerArray();

	/**
	* \fn                       ~RTTPointerArray()
	* \brief                    析构函数
	*/
	~RTTPointerArray();

	/**
	* \fn                       ~RTTPointerArray()
	* \brief                    析构函数
	* \param[in]  aGranularity	粒度		
	*/
	RTTPointerArray(TTInt aGranularity);

public:
	/**
	* \fn                       TTInt Append(const T* aEntry);
	* \brief                    添加
	* \param[in]  aEntry		表项实体
	* \return					操作状态
	*/
	TTInt						Append(const T* aEntry);

	/**
	* \fn                       void Close();
	* \brief                    退出前调用
	*/
	void						Close();
	
	/**
	* \fn                       TTInt Find(const T* aEntry);
	* \brief                    查找某一项，只比较指针
	* \param[in]  aEntry		表项实体
	* \return					操作状态，或者项索引
	*/
	TTInt						Find(const T* aEntry);

	/**
	* \fn                       TTInt Insert(const T* aEntry, TTInt aPos);
	* \brief                    插入某项
	* \param[in]  aEntry		表项实体
	* \param[in]  aPos			插入的位置
	* \return					操作状态
	*/
    TTInt						Insert(const T* aEntry, TTInt aPos);

	/**
	* \fn                       Remove(TTInt aIndex)
	* \brief                    删除某项
	* \param[in]  aIndex		表项索引
	*/
	void						Remove(TTInt aIndex);

	/**
	* \fn                       void Reset()
	* \brief                    复位，表项数为0， 不删除实体
	*/
	void						Reset();

	/**
	* \fn                       T* operator[](TTInt aPos); 
	* \brief				    获取某项
	* \param[in]  aPos			表项位置
	* \return					实体
	*/
	T* operator[](TTInt aPos); 

	/**
	* \fn                       T* const operator[](TTInt aPos) const; 
	* \brief				    获取某项
	* \param[in]  aPos			表项位置
	* \return					实体
	*/
	T* const operator[](TTInt aPos) const; 

	/**
	* \fn                       void ResetAndDestroy()
	* \brief                    复位，表项数为0， 删除实体
	*/
	void						ResetAndDestroy();

	/**
	* \fn                       TTInt Count();
	* \brief                    获取表项数
	* \return					表项数
	*/
	TTInt						Count() const;

private:
	void						ReAllocBuffer();

private:
	TTInt		iEntryNum;
	const T** 	iPtrArray;
	TTInt	    iGranularity;
};

template<class T>
RTTPointerArray<T>::~RTTPointerArray()
{
	//TTASSERT(iPtrArray == NULL);
	Close();
}

template<class T>
RTTPointerArray<T>::RTTPointerArray(TTInt aGranularity)
{
	TTASSERT(aGranularity > 0);
	iPtrArray = (const T**)(malloc(aGranularity * sizeof(T*)));
	iGranularity = aGranularity;
	iEntryNum = 0;
}

template<class T>
RTTPointerArray<T>::RTTPointerArray()
{
	iPtrArray = (const T**)(malloc(KDefaultGranularity * sizeof(T*)));
	//TTASSERT((TTInt(*iPtrArray))&0x4 == 0);
	iGranularity = KDefaultGranularity;	
	iEntryNum = 0;
}

template<class T>
TTInt RTTPointerArray<T>::Append(const T* aEntry)
{	
	if (iEntryNum >= iGranularity)
		ReAllocBuffer();

	*(iPtrArray + iEntryNum) = aEntry;
	iEntryNum++;
	
	return TTKErrNone;
}

template<class T>
void RTTPointerArray<T>::Close()
{
	free((void*)iPtrArray);
	iPtrArray = NULL;
	Reset();
}

template<class T>
TTInt RTTPointerArray<T>::Find(const T *aEntry)
{
	for (TTInt i = 0; i < iEntryNum; i++)
	{
		if ((*(iPtrArray + i)) == aEntry)
		{
			return i;
		}
	}
	
	return TTKErrNotFound;
}

template<class T>
TTInt RTTPointerArray<T>::Insert(const T* aEntry, TTInt aPos)
{
	TTASSERT(aPos >= 0 && aPos <= iEntryNum);

	if (iEntryNum >= iGranularity)
		ReAllocBuffer();

	if (aPos != iEntryNum)
	{
		memmove((void*)(iPtrArray + aPos + 1),(void*)(iPtrArray + aPos), sizeof(T*) * (iEntryNum - aPos));
	}

	iEntryNum++;

	*(iPtrArray + aPos) = aEntry;

	return TTKErrNone;
}

template<class T>
void RTTPointerArray<T>::ReAllocBuffer()
{
	iGranularity += KGranularityIncPace;

	T** pTempEntry = (T**)(malloc(iGranularity * sizeof(T*)));

	TTASSERT(pTempEntry != NULL);

	memcpy((void*)(pTempEntry), (void*)(iPtrArray), sizeof(T*) * iEntryNum);

	free((void*)iPtrArray);
	iPtrArray = (const T**)pTempEntry;
}

template<class T>
void RTTPointerArray<T>::Remove(TTInt aIndex)
{
	TTASSERT(aIndex >= 0 && aIndex < iEntryNum);
	
	if (aIndex != iEntryNum - 1)
		memmove((void*)(iPtrArray + aIndex), (void*)(iPtrArray + aIndex + 1),  sizeof(T*) * (iEntryNum - aIndex - 1));

	iEntryNum--;
}

template<class T>
void RTTPointerArray<T>::Reset()
{
	iEntryNum = 0;
}

template<class T>
TTInt RTTPointerArray<T>::Count() const
{
	return iEntryNum;
}

template<class T>
void RTTPointerArray<T>::ResetAndDestroy()
{
	for (TTInt i = 0; i < iEntryNum; i++)
	{
		delete (*(iPtrArray + i));
	}
	
	memset(iPtrArray, 0, iGranularity * sizeof(T*));

	iEntryNum = 0;
}

template<class T>
T* RTTPointerArray<T>::operator[](TTInt aPos)
{
	TTASSERT(aPos >= 0 && aPos < iEntryNum);
	
	return const_cast<T*>(*(iPtrArray + aPos));
}

template<class T>
T* const RTTPointerArray<T>::operator[](TTInt aPos) const
{
	TTASSERT(aPos >= 0 && aPos < iEntryNum);

	return const_cast<T*>(*(iPtrArray + aPos));
}

#endif // _DEBUG

