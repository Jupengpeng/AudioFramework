#ifndef __ST_OPENSLES_ENGINE_H__
#define __ST_OPENSLES_ENGINE_H__
#include "STLog.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

class STOpenSLESEngine
{
public:
	STOpenSLESEngine();
	~STOpenSLESEngine();
	
public:
	/**
	* \fn                       void GetEngineObject()
	* \brief                    获取EngineObject
	* \return 					返回：EngineObject实例
	*/
	SLObjectItf					GetEngineObject();
	
	/**
	* \fn                       void GetEngineItf()
	* \brief                    获取 EngineItf
	* \return 					返回：EngineItf实例
	*/
	SLEngineItf					GetEngineItf();
	
	/**
	* \fn                       void GetOutputMixObjectItf()
	* \brief                    获取 OutputMixObjectItf
	* \return 					返回：OutputMixObjectItf实例
	*/
	SLObjectItf					GetOutputMixObjectItf();
	
	/**
	* \fn                       void GetOutputMixEnvironmentalReverbItf()
	* \brief                    获取 EnvironmentalReverbItf
	* \return 					返回：EnvironmentalReverbItf实例
	*/
	SLEnvironmentalReverbItf 	GetOutputMixEnvironmentalReverbItf();
	
private:
	void 						Init();
	
private:
	SLObjectItf                  iEngineObject;
	SLEngineItf                  iEngineItf;
	SLObjectItf                  iOutputMixObject;
	SLEnvironmentalReverbItf     iOutputMixEnvironmentalReverb;
};
#endif