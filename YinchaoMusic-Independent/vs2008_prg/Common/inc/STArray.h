#ifndef __ST_ARRAY_H__
#define __ST_ARRAY_H__

// INCLUDES
#include <stdlib.h>
#include <string.h>
#include "STMacrodef.h"
#include "STTypedef.h"


static const STInt KDefaultGranularity = 10;
static const STInt KGranularityIncPace = 5;

template<class T>
class STPointerArray
{
public:
	
	/**
	* \fn                       STPointerArray()
	* \brief                    构造函数
	*/
	STPointerArray();

	/**
	* \fn                       ~STPointerArray()
	* \brief                    析构函数
	*/
	~STPointerArray();

	/**
	* \fn                       ~STPointerArray()
	* \brief                    析构函数
	* \param[in]  aGranularity	粒度		
	*/
	STPointerArray(STInt aGranularity);

public:
	/**
	* \fn                       STInt Append(const T* aEntry);
	* \brief                    添加
	* \param[in]  aEntry		表项实体
	* \return					操作状态
	*/
	STInt						Append(const T* aEntry);

	/**
	* \fn                       void Close();
	* \brief                    退出前调用
	*/
	void						Close();
	
	/**
	* \fn                       STInt Find(const T* aEntry);
	* \brief                    查找某一项，只比较指针
	* \param[in]  aEntry		表项实体
	* \return					操作状态，或者项索引
	*/
	STInt						Find(const T* aEntry);

	/**
	* \fn                       STInt Insert(const T* aEntry, STInt aPos);
	* \brief                    插入某项
	* \param[in]  aEntry		表项实体
	* \param[in]  aPos			插入的位置
	* \return					操作状态
	*/
    STInt						Insert(const T* aEntry, STInt aPos);

	/**
	* \fn                       Remove(STInt aIndex)
	* \brief                    删除某项
	* \param[in]  aIndex		表项索引
	*/
	void						Remove(STInt aIndex);

	/**
	* \fn                       void Reset()
	* \brief                    复位，表项数为0， 不删除实体
	*/
	void						Reset();

	/**
	* \fn                       T* operator[](STInt aPos); 
	* \brief				    获取某项
	* \param[in]  aPos			表项位置
	* \return					实体
	*/
	T* operator[](STInt aPos); 

	/**
	* \fn                       T* const operator[](STInt aPos) const; 
	* \brief				    获取某项
	* \param[in]  aPos			表项位置
	* \return					实体
	*/
	T* const operator[](STInt aPos) const; 

	/**
	* \fn                       void ResetAndDestroy()
	* \brief                    复位，表项数为0， 删除实体
	*/
	void						ResetAndDestroy();

	/**
	* \fn                       STInt Count();
	* \brief                    获取表项数
	* \return					表项数
	*/
	STInt						Count() const;

private:
	void						ReAllocBuffer();

private:
	STInt		iEntryNum;
	const T** 	iPtrArray;
	STInt	    iGranularity;
};

template<class T>
STPointerArray<T>::~STPointerArray()
{
	Close();
}

template<class T>
STPointerArray<T>::STPointerArray(STInt aGranularity)
{
	STASSERT(aGranularity > 0);
	iPtrArray = (const T**)(malloc(aGranularity * sizeof(T*)));
	iGranularity = aGranularity;
	iEntryNum = 0;
}

template<class T>
STPointerArray<T>::STPointerArray()
{
	iPtrArray = (const T**)(malloc(KDefaultGranularity * sizeof(T*)));
	iGranularity = KDefaultGranularity;	
	iEntryNum = 0;
}

template<class T>
STInt STPointerArray<T>::Append(const T* aEntry)
{	
	if (iEntryNum >= iGranularity)
		ReAllocBuffer();

	*(iPtrArray + iEntryNum) = aEntry;
	iEntryNum++;
	
	return STKErrNone;
}

template<class T>
void STPointerArray<T>::Close()
{
	free((void*)iPtrArray);
	iPtrArray = NULL;
	Reset();
}

template<class T>
STInt STPointerArray<T>::Find(const T *aEntry)
{
	for (STInt i = 0; i < iEntryNum; i++)
	{
		if ((*(iPtrArray + i)) == aEntry)
		{
			return i;
		}
	}
	
	return STKErrNotFound;
}

template<class T>
STInt STPointerArray<T>::Insert(const T* aEntry, STInt aPos)
{
	STASSERT(aPos >= 0 && aPos <= iEntryNum);

	if (iEntryNum >= iGranularity)
		ReAllocBuffer();

	if (aPos != iEntryNum)
	{
		memmove((void*)(iPtrArray + aPos + 1),(void*)(iPtrArray + aPos), sizeof(T*) * (iEntryNum - aPos));
	}

	iEntryNum++;

	*(iPtrArray + aPos) = aEntry;

	return STKErrNone;
}

template<class T>
void STPointerArray<T>::ReAllocBuffer()
{
	iGranularity += KGranularityIncPace;

	T** pTempEntry = (T**)(malloc(iGranularity * sizeof(T*)));

	STASSERT(pTempEntry != NULL);

	memcpy((void*)(pTempEntry), (void*)(iPtrArray), sizeof(T*) * iEntryNum);

	free((void*)iPtrArray);
	iPtrArray = (const T**)pTempEntry;
}

template<class T>
void STPointerArray<T>::Remove(STInt aIndex)
{
	STASSERT(aIndex >= 0 && aIndex < iEntryNum);
	
	if (aIndex != iEntryNum - 1)
		memmove((void*)(iPtrArray + aIndex), (void*)(iPtrArray + aIndex + 1),  sizeof(T*) * (iEntryNum - aIndex - 1));

	iEntryNum--;
}

template<class T>
void STPointerArray<T>::Reset()
{
	iEntryNum = 0;
}

template<class T>
STInt STPointerArray<T>::Count() const
{
	return iEntryNum;
}

template<class T>
void STPointerArray<T>::ResetAndDestroy()
{
	for (STInt i = 0; i < iEntryNum; i++)
	{
		delete (*(iPtrArray + i));
	}
	
	memset(iPtrArray, 0, iGranularity * sizeof(T*));

	iEntryNum = 0;
}

template<class T>
T* STPointerArray<T>::operator[](STInt aPos)
{
	STASSERT(aPos >= 0 && aPos < iEntryNum);
	
	return const_cast<T*>(*(iPtrArray + aPos));
}

template<class T>
T* const STPointerArray<T>::operator[](STInt aPos) const
{
	STASSERT(aPos >= 0 && aPos < iEntryNum);

	return const_cast<T*>(*(iPtrArray + aPos));
}

#endif 
