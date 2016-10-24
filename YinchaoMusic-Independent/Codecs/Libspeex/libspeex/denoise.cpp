//#include "stdafx.h"
#include "denoise.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
//#pragma comment(lib,"libspeex.lib") 

//SpeexPreprocessState *Denoise::m_st = NULL;
//SpeexEchoState *Denoise::m_echo_state = NULL;

Denoise::Denoise()
{
	m_st = NULL;
	m_speex_sample_len = 2048;
	//m_echo_state = NULL;
};
Denoise::Denoise(int sampling_rate,int arg_speex_sample_len)
{
	m_st = NULL;
	m_speex_sample_len = arg_speex_sample_len;
	m_sample_rate = sampling_rate;
	//m_echo_state = NULL;

};
Denoise::~Denoise()
{
	Uninit();
};
void Denoise::init(int sampling_rate,int speex_sample_len)
{
// 	int sampling_rate_i = sampling_rate;
// 	int speex_sample_len_i = speex_sample_len;
// 	//speex_sample_len_i = 2048;
// 	//printf("sampling_rate_i = %d,speex_sample_len_i = %d\n",sampling_rate_i,speex_sample_len_i);
// 	m_st = speex_preprocess_state_init(speex_sample_len_i, sampling_rate_i);
// 	int i=1;
// 	speex_preprocess_ctl(m_st, SPEEX_PREPROCESS_SET_DENOISE, &i);
// 	i=-25;
// 	speex_preprocess_ctl(m_st, SPEEX_PREPROCESS_SET_NOISE_SUPPRESS, &i); //�����������˥����dB
// 
// 	//������⡣//����Ч�����õ�����Щ����֡��֡�������Ż�
// 	// 	int vad = 1;  
// 	// 	int vadProbStart = 80;  
// 	// 	int vadProbContinue = 65;  
// 	// 	speex_preprocess_ctl(_st, SPEEX_PREPROCESS_SET_VAD, &vad); //�������  
// 	// 	speex_preprocess_ctl(_st, SPEEX_PREPROCESS_SET_PROB_START , &vadProbStart); //Set probability required for the VAD to go from silence to voice   
// 	// 	speex_preprocess_ctl(_st, SPEEX_PREPROCESS_SET_PROB_CONTINUE, &vadProbContinue); //Set probability required for the VAD to stay in the voice state (integer percent) 
// 	//
// 	i=1;
// 	speex_preprocess_ctl(m_st, SPEEX_PREPROCESS_SET_AGC, &i);
// 	float f=8000*3;
// 	speex_preprocess_ctl(m_st, SPEEX_PREPROCESS_SET_AGC_LEVEL, &f);//��������
// 	//speex_preprocess_ctl(_st, SPEEX_PREPROCESS_SET_AGC_TARGET, &f);
// 	i = 30;
// 	speex_preprocess_ctl(m_st, SPEEX_PREPROCESS_SET_AGC_MAX_GAIN, &i);
// 	i = 12;
// 	speex_preprocess_ctl(m_st, SPEEX_PREPROCESS_SET_AGC_INCREMENT, &i);
// 	i = 0;
// 	speex_preprocess_ctl(m_st, SPEEX_PREPROCESS_SET_AGC_DECREMENT, &i);
// 
// 	i=1;
// 	speex_preprocess_ctl(m_st, SPEEX_PREPROCESS_SET_DEREVERB, &i);
// 	f=0.4;
// 	speex_preprocess_ctl(m_st, SPEEX_PREPROCESS_SET_DEREVERB_DECAY, &f);
// 	f=0.3;
// 	speex_preprocess_ctl(m_st, SPEEX_PREPROCESS_SET_DEREVERB_LEVEL, &f);
// 
// 
// 	//speex_preprocess_ctl(_st, SPEEX_PREPROCESS_GET_ECHO_SUPPRESS , &echo_supp );
// 	//speex_preprocess_ctl(_st, SPEEX_PREPROCESS_GET_ECHO_SUPPRESS_ACTIVE , &echo_supp_active );
// 	//��������//��Ч�����п����ֻ���˷�Ӳ���Դ������������ܡ�
// 	m_echo_state = speex_echo_state_init(speex_sample_len_i,sampling_rate_i*100);//);
// 	//m_echo_state = speex_echo_state_init(speex_sample_len,speex_sample_len*5/*ȡ����ʱ�ӵ�1/3,��100ms�����ݴ�С*/);
// 
// 	//speex_echo_cancel(echo_state, input_frame, echo_frame, output_frame, residue);
// 
// 	speex_echo_ctl(m_echo_state,SPEEX_ECHO_SET_SAMPLING_RATE,&sampling_rate_i);
// 	speex_preprocess_ctl(m_st, SPEEX_PREPROCESS_SET_ECHO_STATE, m_echo_state);
// 	int echo_supp = -50 ;
// 	int echo_supp_active = -60 ;
// 	speex_preprocess_ctl(m_st, SPEEX_PREPROCESS_SET_ECHO_SUPPRESS , &echo_supp );
// 	speex_preprocess_ctl(m_st, SPEEX_PREPROCESS_SET_ECHO_SUPPRESS_ACTIVE , &echo_supp_active );


	m_speex_sample_len = speex_sample_len;
	m_sample_rate = sampling_rate;
	///////////////////////////////
	m_st=speex_preprocess_state_init(m_speex_sample_len, m_sample_rate);
	//m_st=speex_preprocess_state_init(speex_sample_len, sampling_rate);
	int denoise = 1;  
	int noiseSuppress = -25;  
	speex_preprocess_ctl(m_st, SPEEX_PREPROCESS_SET_DENOISE, &denoise); //����  
	speex_preprocess_ctl(m_st, SPEEX_PREPROCESS_SET_NOISE_SUPPRESS, &noiseSuppress); //����������dB  

	int agc = 1;  
	float q=24000;  //����Ϊ��������
	//actually default is 8000(0,32768),here make it louder for voice is not loudy enough by default. 8000  
	speex_preprocess_ctl(m_st, SPEEX_PREPROCESS_SET_AGC, &agc);//����  
	speex_preprocess_ctl(m_st, SPEEX_PREPROCESS_SET_AGC_LEVEL,&q);  
	int vad = 1;  
	int vadProbStart = 80;  
	int vadProbContinue = 65;  
	speex_preprocess_ctl(m_st, SPEEX_PREPROCESS_SET_VAD, &vad); //�������  
	speex_preprocess_ctl(m_st, SPEEX_PREPROCESS_SET_PROB_START , &vadProbStart); //Set probability required for the VAD to go from silence to voice   
	speex_preprocess_ctl(m_st, SPEEX_PREPROCESS_SET_PROB_CONTINUE, &vadProbContinue); //Set probability required for the VAD to stay in the voice state (integer percent)   

}
void Denoise::Uninit()
{
	//speex_echo_state_destroy(m_echo_state);
	speex_preprocess_state_destroy(m_st);
}
void Denoise::Process(char* lin/*ת�����������ת������*/, int len/*����buffer��С�����ֽ�Ϊ��λ*/,char *out/*�ʹ������������������һ��*/)
{

	//memset(out,0,FRAME_SIZE*sizeof(short));  
	//����һ֡16bits������  
	//j++;  
	//int r=fread(in, sizeof(short), FRAME_SIZE, fin);  

	//if (r<FRAME_SIZE)  
	//	break;  

	//��16bits��ֵת��Ϊfloat,�Ա�speex����������湤��  
	spx_int16_t * ptr=(spx_int16_t *)lin;  
	printf("*********************************chenbin**********************start preprocess,");

	if (speex_preprocess_run(m_st, ptr))//Ԥ���� ���˾������ͽ���  
	{  
		printf("*****************************chenbin**************************speech,");  
		//fwrite(lin, sizeof(short), 2048, fout);
		if(out != NULL)
		{
		   memcpy(out,ptr,len);
		}
		
	}  
	else  
	{  
		printf("slience,");  
		//fwrite(lin, sizeof(short), 2048, fout);
        if(out != NULL)
		{
		    memcpy(out,ptr,len);
		    //memset(out,0,len);
		}

	}  
}