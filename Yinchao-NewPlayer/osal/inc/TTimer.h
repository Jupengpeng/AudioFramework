/**
* File : TTimer.h  
* Created on : 2011-3-1
* Author : hu.cao
* Copyright : Copyright (c) 2011 Shuidushi Software Ltd. All rights reserved.
* Description : CTTimer定义文件
*/

#ifndef __TT_TIMER_H__
#define __TT_TIMER_H__

// INCLUDES
#include "TTActive.h"

class CTTimer : public CTTActive
{
public:
	/**
	* \fn				            CTTimer(TPriority aPriority = EPriorityStandard);
	* \param[in]		aPriority	优先级
	* \brief				        构造函数
	*/
	CTTimer(TPriority aPriority = EPriorityStandard);

	/**
	* \fn				            ~CTTimer()
	* \brief				        析构函数
	*/
	virtual ~CTTimer();

	/**
	* \fn							void DoCancel();
	* \brief						取消AO时操作
	*/
	virtual void					DoCancel();

	/**
	* \fn							void After(TTUint32 aDelayTimer);
	* \brief						同步延时
	* \param[in]		aDelayTimer	时间	
	*/
	void							After(TTRequestStatus& aStatus, TTUint32 aDelayTime);
	

private:
	TTUint64 iCompleteTime;

	friend class CTTActiveScheduler;
};

#endif
