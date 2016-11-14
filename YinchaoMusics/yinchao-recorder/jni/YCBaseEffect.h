#ifndef __YINCHAO_BASE_EFFECT_H
#define __YINCHAO_BASE_EFFECT_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "YCEffectCommon.h"
#include "YCEffectParams.h"
using namespace std;
extern "C"{
#include "sox.h"
};

class CReverbEffect;
class  CBaseEffect
{
	friend CReverbEffect;
public:
	CBaseEffect();
	CBaseEffect(double sampleRate,int channels,int sampleBit);
	virtual ~CBaseEffect();
	virtual void setEffectParams(YCEffectParam *m_params,const char*in_path,const char *out_path);
	virtual void process(short*data,int dataSize);
	virtual void doEffect();
	virtual void destroy();


public: 
	int  initEncodeInfo();
	int  initSignalInfo();
	void init();
private: 


	sox_format_t*					m_out;
	sox_format_t*					m_in;
	sox_effects_chain_t*			m_pEffectChain;
	sox_effect_t*					m_pEffect;
	sox_encodinginfo_t				m_sEncodeInfo;
	sox_signalinfo_t				m_sSignalInfo;
	double							m_sampleRate;
	int								m_channels;
	int								m_sampleBit;

	sox_effect_handler_t			m_sInputhandle;
	sox_effect_handler_t			m_sOutputHandle;

};
#endif __YINCHAO_BASE_EFFECT_H