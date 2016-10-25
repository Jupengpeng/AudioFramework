#include "STOpenSLESEngine.h"
#include "STMacroDef.h"
#include "STTypeDef.h"

STOpenSLESEngine::STOpenSLESEngine() 
{
	Init();
}

STOpenSLESEngine::~STOpenSLESEngine() 
{
	if (iOutputMixObject != NULL) {
		(*iOutputMixObject)->Destroy(iOutputMixObject);
		iOutputMixObject = NULL;
		iOutputMixEnvironmentalReverb = NULL;
	}

	if (iEngineObject != NULL)
	{
		(*iEngineObject)->Destroy(iEngineObject);
		iEngineObject = NULL;
		iEngineItf = NULL;
	}
}

SLObjectItf STOpenSLESEngine::GetEngineObject()
{
	return iEngineObject;
}

SLEngineItf	STOpenSLESEngine::GetEngineItf()
{
	return iEngineItf;
}

SLObjectItf STOpenSLESEngine::GetOutputMixObjectItf()
{
	return iOutputMixObject;
}

SLEnvironmentalReverbItf STOpenSLESEngine::GetOutputMixEnvironmentalReverbItf()
{
	return iOutputMixEnvironmentalReverb;
}

void STOpenSLESEngine::Init()
{
	SLresult result = slCreateEngine(&iEngineObject, 0, NULL, 0, NULL, NULL);
	STASSERT(SL_RESULT_SUCCESS == result);
	
	result = (*iEngineObject)->Realize(iEngineObject, SL_BOOLEAN_FALSE);// realize the engine
	STASSERT(SL_RESULT_SUCCESS == result);
	
	result = (*iEngineObject)->GetInterface(iEngineObject, SL_IID_ENGINE, &iEngineItf);// get the engine interface, which is needed in order to create other objects
	STASSERT(SL_RESULT_SUCCESS == result);
	
	const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};// create output mix, with environmental reverb specified as a non-required interface
	const SLboolean req[1] = {SL_BOOLEAN_FALSE};
	result = (*iEngineItf)->CreateOutputMix(iEngineItf, &iOutputMixObject, 1, ids, req);
	STASSERT(SL_RESULT_SUCCESS == result);
	
	
	result = (*iOutputMixObject)->Realize(iOutputMixObject, SL_BOOLEAN_FALSE);// realize the output mix
	STASSERT(SL_RESULT_SUCCESS == result);
	
	// get the environmental reverb interface
	// this could fail if the environmental reverb effect is not available,
	// either because the feature is not present, excessive CPU load, or
	// the required MODIFY_AUDIO_SETTINGS permission was not requested and granted
    result = (*iOutputMixObject)->GetInterface(iOutputMixObject, SL_IID_ENVIRONMENTALREVERB,
                                              &iOutputMixEnvironmentalReverb);
    
    const SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;
    if (SL_RESULT_SUCCESS == result) {
        result = (*iOutputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                                                                                   iOutputMixEnvironmentalReverb, &reverbSettings);
    }
	// ignore unsuccessful result codes for environmental reverb, as it is optional for this example
}