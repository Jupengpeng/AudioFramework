/**
* File        : TTDirEnum.h 
* Created on  : 2011-5-9
* Author      : hu.cao
* Copyright   : Copyright (c) 2010 Shuidushi Software Ltd. All rights reserved.
* Description : CTTDirEnum系统时间声明文件
*/

#ifndef __TT_DIRENUM_H__
#define __TT_DIRENUM_H__
#include "TTArray.h"

class CTTDirEnum
{
public:
	static TTInt EnumDir(RTTPointerArray<TTChar>& aPathArray, const TTChar* aFileDir, const TTChar* aWildCard);
};

#endif 

//end of file
