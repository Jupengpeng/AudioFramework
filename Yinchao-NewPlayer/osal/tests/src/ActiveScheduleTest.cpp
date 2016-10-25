#include <cppunit/config/SourcePrefix.h>
#include "TTMacrodef.h"
#include "TTTypedef.h"
#include "..\inc\ActiveScheduleTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION( ActiveScheduleTest );


class CTTTestActive5 : public CTTActive
{
public:
	CTTTestActive5(CTTActive::TPriority aPriority)
		: CTTActive(aPriority)
	{
		CTTActiveScheduler::Add(this);
	}
	
	void Active()
	{
		CTTActive::SetActive();
	}

	void RunL()
	{
		CPPUNIT_ASSERT(!iActive1Processed && iActive2Processed && !iActive3Processed && iActive4Processed && !iActive5Processed);
		iActive5Processed = ETTTrue;

	}
	void DoCancel()
	{

	}
};

class CTTTestActive1 : public CTTActive
{
public:
	CTTTestActive1(CTTActive::TPriority aPriority)
		: CTTActive(aPriority)
		, iTestActive(NULL)
	{
		CTTActiveScheduler::Add(this);
	}

	~CTTTestActive1()
	{
		delete iTestActive;

	}

	void Active()
	{
		CTTActive::SetActive();
	}

	void RunL()
	{
		if (iTestActive == NULL)
		{
			CPPUNIT_ASSERT(iStatus.Int() == TTKErrNotFound);
 			iTestActive = new CTTTestActive5(CTTActive::EPriorityStandard);	
 			iTestActive->Active();

			TTRequestStatus* status = &(iTestActive->iStatus);
 			iTestActive->iStatus = TTRequestStatus::ERequestPending;
 			TTUser::RequestComplete(status, TTKErrNone);

 			TTRequestStatus* tStatus = &iStatus;
	  		iStatus = TTRequestStatus::ERequestPending;
 			SetActive();
 			TTUser::RequestComplete(tStatus, TTKErrNone);
		}
		else
		{
			CPPUNIT_ASSERT(!iActive1Processed && iActive2Processed && !iActive3Processed && iActive4Processed && iActive5Processed);
			iActive1Processed = ETTTrue;
		}
	}
	void DoCancel()
	{

	}

private:
	CTTTestActive5* iTestActive;
};

class CTTTestActive2 : public CTTActive
{
public:
	CTTTestActive2(CTTActive::TPriority aPriority)
		: CTTActive(aPriority)
	{
		CTTActiveScheduler::Add(this);

	}

	void Active()
	{
		CTTActive::SetActive();
	}

	void RunL()
	{
		CPPUNIT_ASSERT(!iActive1Processed && !iActive2Processed && !iActive3Processed && !iActive4Processed && !iActive5Processed);
		iActive2Processed = ETTTrue;
	}
	void DoCancel()
	{

	}
};

class CTTTestActive3 : public CTTActive
{
public:
	CTTTestActive3(CTTActive::TPriority aPriority)
		: CTTActive(aPriority)
	{
		CTTActiveScheduler::Add(this);
	}

	void Active()
	{
		CTTActive::SetActive();
	}
	
	void RunL()
	{
		//CPPUNIT_ASSERT(EFalse);//多线程出错能收集？
		CPPUNIT_ASSERT(iActive1Processed && iActive2Processed && !iActive3Processed && iActive4Processed && iActive5Processed);
		CTTActiveScheduler::Stop();
		iActive3Processed = ETTTrue;
	}
	void DoCancel()
	{

	}
};

class CTTTestActive4 : public CTTActive
{
public:
	CTTTestActive4(CTTActive::TPriority aPriority)
		: CTTActive(aPriority)
	{
		CTTActiveScheduler::Add(this);
	}

	void Active()
	{
		CTTActive::SetActive();
	}

	void RunL()
	{
		CPPUNIT_ASSERT(!iActive1Processed && iActive2Processed && !iActive3Processed && !iActive4Processed && !iActive5Processed);
		iActive4Processed = ETTTrue;
	}
	void DoCancel()
	{

	}
};


ActiveScheduleTest::ActiveScheduleTest(void)
{
}

ActiveScheduleTest::~ActiveScheduleTest(void)
{
}

void ActiveScheduleTest::setUp()
{
	TTChar ThreadName[] = "ActiveSchedulerTestThread";
	CPPUNIT_ASSERT(TTKErrNone == iThread.Create(ThreadName, Func, this, 50));
}

void* ActiveScheduleTest::Func(void* aPtr)
{
	CTTActiveScheduler* pScheduler = new CTTActiveScheduler();
	CTTActiveScheduler::Install(pScheduler);

	CTTTestActive1* pActive1 = new CTTTestActive1(CTTActive::EPriorityLow);
	CPPUNIT_ASSERT(pActive1->IsAdded());
	CTTTestActive2* pActive2 = new CTTTestActive2(CTTActive::EPriorityHigh);
	CPPUNIT_ASSERT(pActive2->IsAdded());
	CTTTestActive3* pActive3 = new CTTTestActive3(CTTActive::EPriorityIdle);
	CPPUNIT_ASSERT(pActive3->IsAdded());
	CTTTestActive4* pActive4 = new CTTTestActive4(CTTActive::EPriorityUserInput);
	CPPUNIT_ASSERT(pActive4->IsAdded());

	TTRequestStatus* status = &(pActive1->iStatus);
	pActive1->iStatus = TTRequestStatus::ERequestPending;
	pActive1->Active();
	TTUser::RequestComplete(status, TTKErrNotFound);

	status = &(pActive2->iStatus);
	pActive2->iStatus = TTRequestStatus::ERequestPending;
	pActive2->Active();
 	TTUser::RequestComplete(status, TTKErrNone);

	status = &(pActive3->iStatus);
	pActive3->iStatus = TTRequestStatus::ERequestPending;
	pActive3->Active();
	TTUser::RequestComplete(status, TTKErrNone);

	status = &(pActive4->iStatus);
	pActive4->iStatus = TTRequestStatus::ERequestPending;
	pActive4->Active();
	TTUser::RequestComplete(status, TTKErrNone);
 	
	CTTActiveScheduler::Start();

	delete pActive1;
	delete pActive2;
	delete pActive3;
	delete pActive4;
	delete pScheduler;
	return NULL;
}

void ActiveScheduleTest::tearDown()
{
 	iThread.Close();
}

void ActiveScheduleTest::testActiveSchedule()
{
	while (!(iActive1Processed && iActive2Processed && iActive3Processed && iActive4Processed && iActive5Processed))
	{
		TTInt n = 0;
		n++;
	}

	//CPPUNIT_ASSERT(EFalse);//出错 测试用例结束，不继续执行？？？

	iActive1Processed = ETTFalse;
	iActive2Processed = ETTFalse;
	iActive3Processed = ETTFalse;
	iActive4Processed = ETTFalse;
	iActive5Processed = ETTFalse;	
}

