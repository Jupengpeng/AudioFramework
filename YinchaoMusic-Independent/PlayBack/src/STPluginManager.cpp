#include "STOSConfig.h"
#include "STPluginManager.h"
#include "STWavPlugin.h"
#include "STAacPlugin.h"
#include "STUrlUtils.h"
#include "STParamKeys.h"
#include "STKVPair.h"
#include "STLog.h"

static const STInt KMAXExtSize = 16;

typedef struct MediaFormatMap_s
{
	const STChar*					iMediaFormatExt;
	STPluginManager::MediaFormatId	iMediaFormatId;
} MediaFormatMap_t;

static const MediaFormatMap_t KMediaFormatMap[] = { {"AAC", STPluginManager::EMediaExtIdAAC},
													{"MP4", STPluginManager::EMediaExtIdM4A},
													{"M4A", STPluginManager::EMediaExtIdM4A},
													{"MP3", STPluginManager::EMediaExtIdMP3},
													{"WAV", STPluginManager::EMediaExtIdWAV},
													{"WMA", STPluginManager::EMediaExtIdWMA}
												  };

ISTPluginItf* STPluginManager::SelectPlugin(const STChar* aUrl, const STChar* aParams)
{	
	STInt nErr = STUrlUtils::GetParam(aParams, ParamKeyBackgroundSourceName, iTempBuffer, ParamKeySeparator, KMaxPathLength);
	STPluginManager::MediaFormatId tId = nErr != STKErrNone ? STPluginManager::GetMediaFormatId(aUrl) : STPluginManager::GetMediaFormatId(iTempBuffer);
	if (tId == EMediaExtIdAAC)
	{
		return new STAacPlugin();
	}
	else if (tId == EMediaExtIdWAV)
	{
		STLOGE("EMediaExtIdWAV");
		return new STWavPlugin();
	}

	return NULL;
}

STPluginManager::STPluginManager()
{

}

STPluginManager::~STPluginManager()
{
	
}

STPluginManager::MediaFormatId STPluginManager::GetMediaFormatId(const STChar* aUrl)
{
	if (aUrl != NULL && strlen(aUrl) > 0)
	{
		STChar tExtStr[KMAXExtSize];
		STUrlUtils::ParseExtension(aUrl, tExtStr);

		for (STInt i = sizeof(KMediaFormatMap) / sizeof(MediaFormatMap_t) - 1; i >= 0; --i)
		{
			if (strcmp(tExtStr, KMediaFormatMap[i].iMediaFormatExt) == 0)
			{
				return KMediaFormatMap[i].iMediaFormatId;			
			}
		}
	}

	return EMediaExtIdNone;
}
