#ifndef __TT_MP3PARSER_TEST_H__
#define __TT_MP3PARSER_TEST_H__

#include <cppunit/extensions/HelperMacros.h>


class MP3Parsertest : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( MP3Parsertest );
	CPPUNIT_TEST( testMP3Parser );
	CPPUNIT_TEST_SUITE_END();
public:
	void setUp(){};
	void tearDown(){};
protected:
	void testMP3Parser();
	static void* Fun(void*);
};

#endif