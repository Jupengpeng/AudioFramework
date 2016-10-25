
#ifndef __TT_DLL_LOADER_H__
#define __TT_DLL_LOADER_H__

#if __cplusplus
extern "C" {
#endif

void* DllLoad(const char* aPathName);

void* DllSymbol(void* aHandle,const char* aSymbol);

int DllClose(void* aHandle);

#if __cplusplus
}  // extern "C"
#endif

#endif //__TT_DLL_LOADER_H__
