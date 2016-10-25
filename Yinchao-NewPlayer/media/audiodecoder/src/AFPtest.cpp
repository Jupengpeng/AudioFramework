#include "AFPTest.h"
#include "TTLog.h"
#include "TTAudioDecoder.h"
#include "TTMediaBuffer.h"
#include "TTOSALConfig.h"
#ifdef __TT_OS_WINDOWS__
static const TTChar* KWAVPath = "f:\\D_Work\\Project_Android\\Information\\MuiscTest\\Íõ·Æ-ÖÂÇà´º_result.wav";
#elif (defined __TT_OS_ANDROID__)
static const TTChar* KWAVPath = "/sdcard/test.pcm";
#else 
#endif

AFPTest::AFPTest() {
	iUrl = NULL;
	iCritical.Create();
	iIsStopAFPTest = ETTFalse;
}
AFPTest::~AFPTest() {
	Close();
	iCritical.Destroy();
}

TTInt AFPTest::Open(const TTChar* aUrl) {
	iCritical.Lock();
	iIsStopAFPTest = ETTFalse;
	TTASSERT(iUrl == NULL);
	iUrl = (TTChar*)malloc(strlen(aUrl) + 1);
	strcpy(iUrl, aUrl);
	iCritical.UnLock();
	iThreadHandle.Create("AFPTest",	AFPThreadProc, this, 0, ETTFalse);
	return TTKErrNone;
}

void AFPTest::Close() {
	iCritical.Lock();
	iIsStopAFPTest = ETTTrue;
	SAFE_DELETE(iUrl);
	iCritical.UnLock();

	iThreadHandle.Close();	
}

void* AFPTest::AFPThreadProc(void* aPtr) {
	AFPTest* pTest = reinterpret_cast<AFPTest*>(aPtr);
	pTest->AFPThreadProcL(NULL);
	return NULL;
}

void AFPTest::AFPThreadProcL(void* aPtr) {
	CTTAudioDecoder *pAudioDecoder = new CTTAudioDecoder();
	iCritical.Lock();
	TTASSERT(iUrl != NULL);

	///test////
	FILE* infile = NULL;
	FILE* pFile = NULL;

	if ( (infile = fopen( iUrl, "rb")) == NULL)
	{	
		LOGE ("Error : Not able to open input file \n") ;
		return ;
	};

	if ( (pFile = fopen( KWAVPath, "wb+")) == NULL)
	{	
		LOGE ("Error : Not able to creat target file '%s'\n") ;
		return ;
	};
	TWavHead wavehead;
	fread(&wavehead,1,sizeof(TWavHead),infile);
	wavehead.nSamplesPerSec = 8000;
	wavehead.nChannels = 1;
	fwrite (&wavehead,1, sizeof(TWavHead),pFile);
	fclose(infile);

	//= fopen(KWAVPath, "wb+");


	///test////
	TTInt nErr = pAudioDecoder->Open(iUrl,8000,1);
	iCritical.UnLock();

	if (nErr == TTKErrNone)
	{
		TTInt16* pDataBuffer = (TTInt16*)malloc(450 * KILO * sizeof(TTInt16));

		//FILE* pFile = fopen(KWAVPath, "wb+");
		TTInt nFilledLength = 0;

		do
		{
			iCritical.Lock();
			if (iIsStopAFPTest) {
				iCritical.UnLock();
				break;
			}
			iCritical.UnLock();

			nFilledLength = pAudioDecoder->DecodeToBuffer(pDataBuffer, 450 * KILO);

			if (nFilledLength > 0)
			{
				fwrite(pDataBuffer, 1, nFilledLength * sizeof(TTInt16), pFile);
			}
		}while (nFilledLength > 0);

		free(pDataBuffer);
		fclose(pFile);
	}

	pAudioDecoder->Close();
	LOGI("CTTAudioDecoder DECODE OVER");
	delete pAudioDecoder;
}

