#include <cppunit/config/SourcePrefix.h>
#include "TTMacrodef.h"
#include "TTTypedef.h"
#include "TTArray.h"
#include "../inc/ArrayTest.h"
#include <time.h>

CPPUNIT_TEST_SUITE_REGISTRATION( Arraytest );

Arraytest::Arraytest(void)
{
}

Arraytest::~Arraytest(void)
{
}

void Arraytest::setUp()
{

}

void Arraytest::tearDown()
{

}

void Arraytest::testArray()
{
	class TTArrayItem
	{
	public:
		TTInt    iIdx;
		TTInt	 iIdx2;
		~TTArrayItem()
		{

		};
	};

	RTTPointerArray<TTArrayItem> rPointerArray(5);

	TTArrayItem* pItem1 = new TTArrayItem();
	TTArrayItem* pItem2 = new TTArrayItem();

	TTArrayItem* pItem3 = new TTArrayItem();

	TTArrayItem* pItem4 = new TTArrayItem();
	TTArrayItem* pItem5 = new TTArrayItem();
	TTArrayItem* pItem6 = new TTArrayItem();
	TTArrayItem* pItem7 = new TTArrayItem();

	rPointerArray.Append(pItem1);
	rPointerArray.Append(pItem2);
	rPointerArray.Append(pItem3);
	rPointerArray.Append(pItem4);
 	rPointerArray.Append(pItem5);
 	rPointerArray.Append(pItem6);
	rPointerArray.Append(pItem7);

	for (TTInt i = 0; i < 15; i++)
	{
		TTArrayItem* pItem = new TTArrayItem();
		rPointerArray.Append(pItem);
	}

	TTArrayItem* pItem8 = new TTArrayItem();
	rPointerArray.Insert(pItem8, 0);
	CPPUNIT_ASSERT(pItem8 == rPointerArray[0]);
	CPPUNIT_ASSERT(pItem1 == rPointerArray[1]);

	CPPUNIT_ASSERT(pItem2 == rPointerArray[2]);
	CPPUNIT_ASSERT(pItem3 == rPointerArray[3]);
	CPPUNIT_ASSERT(pItem4 == rPointerArray[4]);
	CPPUNIT_ASSERT(pItem5 == rPointerArray[5]);
	CPPUNIT_ASSERT(pItem6 == rPointerArray[6]);
	CPPUNIT_ASSERT(pItem7 == rPointerArray[7]);
	CPPUNIT_ASSERT(23 == rPointerArray.Count());

	CPPUNIT_ASSERT(2 == rPointerArray.Find(pItem2));

	rPointerArray.Remove(5);
	CPPUNIT_ASSERT(22 == rPointerArray.Count());

	CPPUNIT_ASSERT(pItem6 == rPointerArray[5]);

	rPointerArray.ResetAndDestroy();
	CPPUNIT_ASSERT(0 == rPointerArray.Count());
	rPointerArray.Close();
}






