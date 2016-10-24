#ifndef __ST_INTERFACE_H__
#define __ST_INTERFACE_H__

#include "STMacrodef.h"
#include "STTypedef.h"

class ISTInterface
{

public: // Constructors and destructor

	ISTInterface(): iRefCount(1){};

	virtual ~ISTInterface(){};

	/**
	* \fn							STInt AddRef()
	* \brief						增加接口的引用计数
	*								接口对象创建时(Constructor)，或任何对象引用此接口时(QueryInterface)，都需要增加引用计数
	* \return						当前的引用计数
	*/
	virtual	STInt					AddRef()
	{
		++ iRefCount;
		return iRefCount;
	}

	/**
	* \fn							STInt Release()
	* \brief						释放接口
	*								首先减少引用计数，然后判断计数是否为0，为0时，表示此接口不再被任何对象引用，调用delete this销毁此接口对象
	* \return						当前的引用计数
	*/
	virtual	STInt					Release()
	{
		--iRefCount;
		if (0 == iRefCount)
		{
			delete this;
			return 0;
		}
		else
		{
			return iRefCount;
		}
	}

	/**
	* \fn							STInt QueryInterface(STUint32 aInterfaceID, void** aInterfacePtr)
	* \brief						请求接口
	* \param	aInterfaceID[in]	接口ID
	* \param	aInterfacePtr[in]	接口指针
	* \return						STKErrNone: 成功
	*								STKErrNotSupport: 不支持此接口
	*/
	virtual	STInt					QueryInterface(STUint32 /*aInterfaceID*/, void** /*aInterfacePtr*/)
	{
		return STKErrNotSupported;
	}


private:

	STInt							iRefCount;	/**< 引用计数 */
};

#endif 
// End of File
