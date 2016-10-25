#ifndef __AFP_TEST_H__
#define __AFP_TEST_H__
#include "TTTypedef.h"
#include "TThread.h"
#include "TTCritical.h"


typedef struct _TWavHeader
{  
	long rId; 
	long rLen; 
	long wId; 
	long fId; 
	long fLen; 
	unsigned short wFormatTag; 
	unsigned short nChannels; 
	long nSamplesPerSec; 
	long nAvgBytesPerSec; 
	unsigned short nBlockAlign; 
	unsigned short wBitsPerSample; 
	long dId; 
	long wSampleLength;
}TWavHead;


class AFPTest{
public:
	AFPTest();
	~AFPTest();
	TTInt Open(const TTChar* aUrl);
	void Close();

	static void* 	AFPThreadProc(void* aPtr);
	void 			AFPThreadProcL(void* aPtr);

private:
	RTThread		iThreadHandle;
	RTTCritical     iCritical;
	TTBool			iIsStopAFPTest;
	TTChar*			iUrl;
};

#endif
