#include "STWavPlugin.h"
#include "STDataReaderSelector.h"

STWavPlugin::STWavPlugin()
	: iCurReadPos(0)
{
}

STWavPlugin::~STWavPlugin()
{
	SAFE_DELETE(iFileReader);
}

STInt STWavPlugin::InitPlugin(const STChar* aUrl, const STChar* aParams)
{
	iFileReader = STDataReaderSelector::SelectDataReader(aUrl);
	STInt nErr = iFileReader->Open(aUrl);
	if (nErr == STKErrNone)
	{
		STInt nSampleRate = 44100;
		STInt nChannels = 2;

		iMediaInfo.AddAudioStream(new STAudioInfo(nSampleRate, nChannels, 0));

		iDuration = STInt64(iFileReader->Size())  * 1000 / nSampleRate / nChannels / 2;
	}
	
	return nErr;
}

void STWavPlugin::ResetPlugin()
{
	
}

void STWavPlugin::UnInitPlugin()
{
	STBasePlugin::UnInitPlugin();
	iFileReader->Close();
}

STInt STWavPlugin::StartReading()
{
	iReadStatus = ESTReadStatusReading;
	return STKErrNone;
}

STInt STWavPlugin::Read(STSampleBuffer* aBuffer)
{
	if (iReadStatus == ESTReadStatusReading)
	{
		STInt nSize = aBuffer->Size();

		STInt nReadSize = iFileReader->Read(aBuffer->Ptr(), iCurReadPos, nSize);
		if (nReadSize > 0)
		{
			aBuffer->SetByteOffset(iCurReadPos);
			iCurReadPos += nReadSize;
			if (nReadSize < nSize) 
			{
				memset(aBuffer->Ptr() + nReadSize, 0, nSize - nReadSize);
				iReadStatus = ESTReadStatusComplete;
				return STKErrEof;
			} 

			return STKErrNone;
		} 
		else 
		{
			iReadStatus = ESTReadStatusReadErr;
			return STKErrUnknown;
		}
	} 

	return STKErrNotReady;
}

void STWavPlugin::Seek(STUint aPos)
{
	iCurReadPos = ((STInt64)aPos * iMediaInfo.GetAudioStreamArray()[0]->GetSampleRate() * iMediaInfo.GetAudioStreamArray()[0]->GetChannels() * 2 / 1000) & 0xFFFFFFFC;
}

STInt	STWavPlugin::GetDownloadPercent()
{

	return 100;
}
