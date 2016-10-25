#ifndef __TT_ARRAY_TEST_H__
#define __TT_ARRAY_TEST_H__

#include <cppunit/extensions/HelperMacros.h>
#include "TTArray.h"

class Arraytest : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( Arraytest );
	CPPUNIT_TEST( testArray );
	CPPUNIT_TEST_SUITE_END();
public:
	Arraytest(void);
	~Arraytest(void);
public:
	void setUp();
	void tearDown();
protected:
	void testArray();
};

#endif