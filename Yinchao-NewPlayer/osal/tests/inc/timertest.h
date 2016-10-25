#ifndef __TT_AO_TEST_H__
#define __TT_AO_TEST_H__

#include <cppunit/extensions/HelperMacros.h>
#include <TTimer.h>
#include <TThread.h>


class Timertest : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( Timertest );
	CPPUNIT_TEST( testTimer );
	CPPUNIT_TEST_SUITE_END();
public:
	Timertest(void);
	~Timertest(void);
public:
	void setUp();
	void tearDown();
protected:
	void testTimer();

private:
	static void* Func(void* aPtr);

private:
	RTThread	iThread;
};

#endif