#include <cppunit/config/SourcePrefix.h>
#include "TTMacrodef.h"
#include "TTTypedef.h"
#include "..\inc\MP3Parsertest.h"
#include "TTMediaInfoProxy.h"
#include "TThread.h"

CPPUNIT_TEST_SUITE_REGISTRATION( MP3Parsertest );

void MP3Parsertest::testMP3Parser()
{
	RTThread tThread;
	TTChar ThreadName[] = "testThread";
	CPPUNIT_ASSERT(TTKErrNone == tThread.Create(ThreadName, Fun, NULL));

	tThread.Close();
}

void* MP3Parsertest::Fun(void*)
{
	CTTActiveScheduler* pScheduler = new CTTActiveScheduler();
	CTTActiveScheduler::Install(pScheduler);

	CTTMediaInfoProxy* pProxy = new CTTMediaInfoProxy();
	CPPUNIT_ASSERT(TTKErrNone == pProxy->Open("C:\\1.mp3"));
	CPPUNIT_ASSERT(TTKErrNone == pProxy->Parse());

	CTTActiveScheduler::Start();

	return NULL;
}




