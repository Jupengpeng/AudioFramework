#ifndef __TT_READER_TEST_H__
#define __TT_READER_TEST_H__

#include "TTDataReaderItf.h"
#include <cppunit/extensions/HelperMacros.h>
#include "TThread.h"

class Readertest : public CPPUNIT_NS::TestFixture, public ITTDataReaderObserver
{
	CPPUNIT_TEST_SUITE( Readertest );
	CPPUNIT_TEST( testread );
	CPPUNIT_TEST_SUITE_END();
public:
	Readertest(void);
	~Readertest(void);
public:
	void setUp();
	void tearDown();
protected:
	void testread();

	static void* ThreadProc(void* aPtr);

	void ReadComplete(TTInt aReadSize, TTUint8* aReadBuffer);

private:
	RTThread			iTestThread;
};

#endif