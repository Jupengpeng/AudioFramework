#ifndef DENOISE_H_
#define DENOISE_H_
//#include "stdafx.h"
#include <speex/speex.h>  
#include <speex/speex_preprocess.h>  
#include <speex/speex_echo.h>   

class Denoise
{
public:
	Denoise();
	Denoise(int sampling_rate,int arg_speex_sample_len);
	virtual ~Denoise();
public:
	void init(int sampling_rate,int speex_sample_len);
	void Uninit();

	void Process(char* lin/*转入参数，即是转出参数*/, int len/*输入buffer大小，字节为单位*/,char *out/*和处理过后的输入参数内容一样*/);
public:
	SpeexPreprocessState *m_st;
	int m_speex_sample_len;//处理每一帧的长度 默认为2048 个short
	int m_sample_rate;//采样率
	//SpeexEchoState * m_echo_state;

};
#endif
