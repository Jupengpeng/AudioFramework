#ifndef __ST_MEDIA_INFO_H__
#define __ST_MEDIA_INFO_H__
#include "STTypedef.h"
#include "STMacrodef.h"
#include "STArray.h"
class STAudioInfo
{
public:
	STAudioInfo(STInt aSampleRate, STInt aChannels, STInt aIndex) : iSampleRate(aSampleRate), iChannels(aChannels), iIndex(aIndex)
	{
	}

	STInt				GetSampleRate()
	{
		return iSampleRate;
	}

	STInt				GetChannels()
	{
		return iChannels;
	}

	STInt 				GetIndex()
	{
		return iIndex;
	}

private:
	STInt				iSampleRate;
	STInt				iChannels;
	STInt				iIndex;
};

class STVideoInfo
{

};

class STMediaInfo
{
public:
	const STPointerArray<STAudioInfo>& GetAudioStreamArray() const
	{
		return iAudioStreamArray;
	}

	void AddAudioStream(STAudioInfo* aAudio)
	{
		iAudioStreamArray.Append(aAudio);
	}

	STVideoInfo* GetVideoInfo()
	{
		return iVideoInfo;
	}

	void 		SetVideoInfo(STVideoInfo* aVideoInfo)
	{
//malloc
	}

	~STMediaInfo()
	{
		iAudioStreamArray.ResetAndDestroy();
		iAudioStreamArray.Close();
		SAFE_DELETE(iVideoInfo);
	}

	STMediaInfo():iVideoInfo(NULL)
	{
	};

	void Reset()
	{
		for (STInt i = iAudioStreamArray.Count() - 1; i >= 0; i--)
		{
			delete iAudioStreamArray[i];
		}
		iAudioStreamArray.Reset();
		SAFE_DELETE(iVideoInfo);
	}
private:
	STPointerArray<STAudioInfo> 		iAudioStreamArray;
	STVideoInfo*						iVideoInfo;
};

#endif
