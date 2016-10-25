#include <cppunit/config/SourcePrefix.h>
#include "TTMacrodef.h"
#include "TTTypedef.h"
#include "..\inc\Timertest.h"

TTInt nCount = 10;
TTInt nTTActiveTestRunCount = 0;

class CTTActiveTest : public CTTActive
{
public:
	CTTActiveTest(TPriority aPriority)
		: CTTActive(aPriority)
	{
		CTTActiveScheduler::Add(this);
	}

	void DoCancel()
	{

	}

	void Active()
	{
		SetActive();
	}

	void RunL()
	{
		nTTActiveTestRunCount++;
	}
};

class Timer : public CTTimer
{
public:
	Timer(TPriority aPriority)
		: CTTimer(aPriority)
		, iActiveTest(NULL)
	{

	}

	~Timer()
	{
		SAFE_DELETE(iActiveTest);
	}

	void RunL()
	{
		if ((nCount > 0) && (nCount % 2 == 0))
		{
			if (iActiveTest == NULL)
			{
				iActiveTest = new CTTActiveTest(EPriorityHigh);
			}

			TTRequestStatus* status = &(iActiveTest->iStatus);
			iActiveTest->iStatus = TTRequestStatus::ERequestPending;
			iActiveTest->Active();
			TTUser::RequestComplete(status, TTKErrNotFound);
		}

		nCount--;
		if (nCount > 0)
		{
			TTRequestStatus tStatus;
			After(tStatus, 1000000);
		}
		else
		{
			CTTActiveScheduler::Stop();
		}
	}

	CTTActiveTest* iActiveTest;	
};


CPPUNIT_TEST_SUITE_REGISTRATION( Timertest );

Timertest::Timertest(void)
{
}

Timertest::~Timertest(void)
{
}

void Timertest::setUp()
{
	TTChar ThreadName[] = "TimertestThread";
	CPPUNIT_ASSERT(TTKErrNone == iThread.Create(ThreadName, Func, this));
}

void Timertest::tearDown()
{
	iThread.Close();
}

void* Timertest::Func(void* aPtr)
{
	CTTActiveScheduler* pScheduler = new CTTActiveScheduler();
	CTTActiveScheduler::Install(pScheduler);

	Timer* pTimer = new Timer(CTTActive::EPriorityStandard);
	TTRequestStatus aStatus;
	pTimer->After(aStatus, 1000000);

	CTTActiveScheduler::Start();

	delete pTimer;
	delete pScheduler;
	return NULL;
}

void Timertest::testTimer()
{
	while (nCount > 0);

	CPPUNIT_ASSERT(nTTActiveTestRunCount == 5);
	nTTActiveTestRunCount = 0;
	nCount = 10;
}

