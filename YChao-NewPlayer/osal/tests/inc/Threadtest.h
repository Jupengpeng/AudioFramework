#ifndef __TT_THREAD_TEST_H__
#define __TT_THREAD_TEST_H__

#include "TThread.h"
#include <cppunit/extensions/HelperMacros.h>

class Threadtest : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( Threadtest );
	CPPUNIT_TEST( testThread );
	CPPUNIT_TEST_SUITE_END();
public:
	Threadtest(void);
	~Threadtest(void);
public:
	void setUp();
	void tearDown();
protected:
	void testThread();

public:
	static void* fun(void *p);

private:
	RTThread		iThread;
};

#endif