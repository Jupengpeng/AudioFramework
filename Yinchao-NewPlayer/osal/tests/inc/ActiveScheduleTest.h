#ifndef __TT_AO_TEST_H__
#define __TT_AO_TEST_H__

#include "TTActive.h"
#include <cppunit/extensions/HelperMacros.h>
#include "TTActiveScheduler.h"
#include "TThread.h"

static TTBool				iActive1Processed = ETTFalse;
static TTBool				iActive2Processed = ETTFalse;
static TTBool				iActive3Processed = ETTFalse;
static TTBool				iActive4Processed = ETTFalse;
static TTBool				iActive5Processed = ETTFalse;	

class ActiveScheduleTest : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( ActiveScheduleTest );
	CPPUNIT_TEST( testActiveSchedule );
	CPPUNIT_TEST_SUITE_END();
public:
	ActiveScheduleTest(void);
	~ActiveScheduleTest(void);
public:
	void setUp();
	void tearDown();
protected:
	void testActiveSchedule();

	static void* Func(void* aPtr);


public:
	RTThread			iThread;
		
};

#endif