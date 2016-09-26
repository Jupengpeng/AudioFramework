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

	void Process(char* lin/*ת�����������ת������*/, int len/*����buffer��С���ֽ�Ϊ��λ*/,char *out/*�ʹ������������������һ��*/);
public:
	SpeexPreprocessState *m_st;
	int m_speex_sample_len;//����ÿһ֡�ĳ��� Ĭ��Ϊ2048 ��short
	int m_sample_rate;//������
	//SpeexEchoState * m_echo_state;

};
#endif
