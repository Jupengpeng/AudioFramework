#include <cppunit/config/SourcePrefix.h>
#include "TTMacrodef.h"
#include "TTFileReader.h"
#include "..\inc\Readertest.h"
#include <windows.h>

TTBool bAsyncReadComplete = ETTFalse;

CPPUNIT_TEST_SUITE_REGISTRATION( Readertest );

Readertest::Readertest(void)
{
}

Readertest::~Readertest(void)
{
}

void Readertest::setUp()
{
	TTChar ThreadName[] = "ReaderTest";
	iTestThread.Create(ThreadName, ThreadProc, this);
}

void Readertest::tearDown()
{
	iTestThread.Close();
}

void Readertest::testread()
{	
	while(!bAsyncReadComplete);
	bAsyncReadComplete = ETTFalse;
}

char *w2c(char *pcstr,const wchar_t *pwstr, size_t len)
{
	int nlength=wcslen(pwstr);

	//获取转换后的长度
	int nbytes = WideCharToMultiByte( 0, // specify the code page used to perform the conversion
		0,         // no special flags to handle unmapped characters
		pwstr,     // wide character string to convert
		nlength,   // the number of wide characters in that string
		NULL,      // no output buffer given, we just want to know how long it needs to be
		0,
		NULL,      // no replacement character given
		NULL );    // we don't want to know if a character didn't make it through the translation

	// make sure the buffer is big enough for this, making it larger if necessary
	if(nbytes>len)   nbytes=len;

	// 通过以上得到的结果，转换unicode 字符为ascii 字符
	WideCharToMultiByte( 0, // specify the code page used to perform the conversion
		0,         // no special flags to handle unmapped characters
		pwstr,   // wide character string to convert
		nlength,   // the number of wide characters in that string
		pcstr, // put the output ascii characters at the end of the buffer
		nbytes,                           // there is at least this much space there
		NULL,      // no replacement character given
		NULL );

	return pcstr ;

}

void* Readertest::ThreadProc(void* aPtr)
{
	CTTActiveScheduler* pScheduler = new CTTActiveScheduler();
	CTTActiveScheduler::Install(pScheduler);

	ITTDataReader* pReader = new CTTFileReader();
	pReader->SetObserver((Readertest*)aPtr);
	wchar_t filename[10] = L"C:\\1.ape";

	char *pcstr = (char *)malloc(sizeof(char)*(2 * wcslen(filename)+1));
	memset(pcstr , 0 , 2 * wcslen(filename)+1 );
	w2c(pcstr,filename,2 * wcslen(filename)+1) ;

	CPPUNIT_ASSERT(TTKErrNone == pReader->Open(pcstr));
	TTUint8* p = new TTUint8[1024];
	CPPUNIT_ASSERT(pReader->ReadSync(p, 1024, 1024) == 1024);
	CPPUNIT_ASSERT(pReader->ReadSync(p, 2048, 1024) == 1024);

	pReader->ReadAsync(p, 3096, 1024);
	CTTActiveScheduler::Start();

	
	pReader->Release();
	delete pScheduler;
	return NULL;
}

void Readertest::ReadComplete(TTInt aReadSize, TTUint8* aReadBuffer)
{
	TTASSERT(aReadSize == 1024);

	bAsyncReadComplete = ETTTrue;
	
	CTTActiveScheduler::Stop();
}

