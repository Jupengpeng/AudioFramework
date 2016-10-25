#include <cppunit/config/SourcePrefix.h>
#include "TTMacrodef.h"
#include "TTTypedef.h"
#include "TThread.h"
#include "..\inc\Threadtest.h"


CPPUNIT_TEST_SUITE_REGISTRATION( Threadtest );

Threadtest::Threadtest(void)
{
}

Threadtest::~Threadtest(void)
{
}

void Threadtest::setUp()
{
}

void Threadtest::tearDown()
{
	CPPUNIT_ASSERT(TTKErrNone == iThread.Close());	
}

void Threadtest::testThread()
{
	TTChar ThreadName[] = "testThread";
	CPPUNIT_ASSERT(TTKErrNone == iThread.Create(ThreadName, fun, NULL));
}

void* Threadtest::fun(void *p)
{	
	return NULL;
}




