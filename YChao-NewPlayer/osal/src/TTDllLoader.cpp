// INCLUDES
#include "TTOsalConfig.h"
#include "TTDllLoader.h"


#if (defined __TT_OS_WINDOWS__)//WINDOWS
#include <Windows.h>
void* DllLoad(const char* aPathName)
{
	return LoadLibrary(aPathName);
}

void* DllSymbol(void* aHandle,const char* aSymbol)
{
	return GetProcAddress((HMODULE)aHandle, aSymbol);
}

int DllClose(void* aHandle)
{
	return FreeLibrary((HMODULE)aHandle);
}
#elif (defined __TT_OS_SYMBIAN__)//SYMBIAN
#include <e32std.h>
#include "string.h"

void* DllLoad(const char* aPathName)
{
 	RLibrary* pLibrary = new RLibrary();
// 	TPtr tPtr(NULL, 1024);
// 	tPtr = (const TUint16*)aPathName;
	int nErr = pLibrary->Load(_L("C:\\sys\\bin\\2002B1C1.DLL"));
	if (nErr != KErrNone)
	{
		__ASSERT_ALWAYS(false,User::Panic(_L("DllLoad"), 1));
		ASSERT(false);
		pLibrary->Close();
		return NULL;
	}
	__ASSERT_ALWAYS(false,User::Panic(_L("DllLoad"), 2));
	ASSERT(false);

	return (void*)pLibrary;
}

void* DllSymbol(void* aHandle,const char* aSymbol)
{
	TLibraryFunction pFun = NULL;

	if (strcmp(aSymbol, "InitDecoder") == 0)
	{
		pFun = ((RLibrary*)(aHandle))->Lookup(1);
	}
	else if (strcmp(aSymbol, "FreeDecoder") == 0)
	{
		pFun = ((RLibrary*)(aHandle))->Lookup(2);
	}
	else if (strcmp(aSymbol, "ResetDecoder") == 0)
	{
		pFun = ((RLibrary*)(aHandle))->Lookup(3);
	}	
	else if (strcmp(aSymbol, "ProcessL") == 0)
	{
		pFun = ((RLibrary*)(aHandle))->Lookup(4);
	}
	else if (strcmp(aSymbol, "FormatSupport") == 0)
	{
		pFun = ((RLibrary*)(aHandle))->Lookup(5);
	}

	return (void*)pFun;
}

int DllClose(void* aHandle)
{
	((RLibrary*)(aHandle))->Close();
	//return FreeLibrary((HMODULE)aHandle);
}
#elif (defined __TT_OS_ANDROID__)//ANDROID
#include <dlfcn.h>

void* DllLoad(const char* aPathName)
{
	return dlopen(aPathName, RTLD_NOW);
}

void* DllSymbol(void* aHandle,const char* aSymbol)
{
	return dlsym(aHandle, aSymbol);
}

int DllClose(void* aHandle)
{
	return dlclose(aHandle);
}
#endif


//end of file
