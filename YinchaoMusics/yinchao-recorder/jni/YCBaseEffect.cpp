#include "YCBaseEffect.h"

CBaseEffect::CBaseEffect(double sampleRate,int channels,int sampleBit)
{
	m_sampleRate = sampleRate;
	m_channels = channels;
	m_sampleBit =sampleBit;
	memset(&m_sSignalInfo,0,sizeof(m_sSignalInfo));
	memset(&m_sEncodeInfo,0,sizeof(m_sEncodeInfo));
	init();
}
CBaseEffect::CBaseEffect()
{


}
CBaseEffect::~CBaseEffect()
{

}

void CBaseEffect::process(short*data,int dataSize)
{

}

void CBaseEffect::doEffect(){
	sox_flow_effects(m_pEffectChain, NULL, NULL);
}

void CBaseEffect::destroy()
{
	sox_delete_effects_chain(m_pEffectChain);
	sox_quit();
}

void CBaseEffect::init()
{
	initEncodeInfo();
	initSignalInfo();
}

int CBaseEffect::initEncodeInfo()
{
	do 
	{
		m_sEncodeInfo.encoding = SOX_ENCODING_SIGN2;
		m_sEncodeInfo.bits_per_sample =m_sampleBit;
		return 1;
	} while (0);
	return 0;
}

int CBaseEffect::initSignalInfo()
{
	do{
		m_sSignalInfo.rate = m_sampleRate;
		m_sSignalInfo.channels = m_channels;
		m_sSignalInfo.precision =m_sampleBit;
		return 1;
	}while(0);
	return 0;
}

void CBaseEffect::setEffectParams(YCEffectParam *m_params,const char*in_path,const char *out_path)
{

	char * args[10];
	sox_init();

	m_in = sox_open_read(in_path, &m_sSignalInfo, &m_sEncodeInfo, "raw");
	m_out = sox_open_write(out_path, &m_sSignalInfo, NULL, "raw", NULL, NULL);
	m_pEffectChain = sox_create_effects_chain(&m_sEncodeInfo, &m_sEncodeInfo);


	//set input params
	m_pEffect = sox_create_effect(sox_find_effect("input"));
	args[0] = (char *)m_in;
	sox_effect_options(m_pEffect, 1, args);
	sox_add_effect(m_pEffectChain, m_pEffect, &m_sSignalInfo, &m_sSignalInfo);
	free(m_pEffect);

	//add effect params

	//set output params
	m_pEffect = sox_create_effect(sox_find_effect("output"));
	args[0] = (char *)m_out;
	sox_effect_options(m_pEffect, 1, args);
	sox_add_effect(m_pEffectChain, m_pEffect, &m_sSignalInfo, &m_sSignalInfo);
	free(m_pEffect);
}


