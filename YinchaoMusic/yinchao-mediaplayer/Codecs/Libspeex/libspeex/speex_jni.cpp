#include <jni.h>        
#include <string.h>    
#include <unistd.h>             
#include <speex/speex.h>
#include <speex/speex_preprocess.h>  
#include <speex/speex_echo.h>  

// static int codec_open = 0;         
// static int dec_frame_size;    
// static int enc_frame_size;       
// static SpeexBits ebits, dbits;    
// void *enc_state;    
// void *dec_state;

static SpeexPreprocessState *_st = NULL;
static int speex_sample_len = 2048;
static SpeexEchoState * echo_state = NULL;
//bool b_speex_init = false;
/*******************************************/
extern "C"
JNIEXPORT void JNICALL ReduceNoiseSpeexInit(JNIEnv *env, jobject obj,jint sampling_rate,jint speex_sample_len = 2048)
{
	int sampling_rate_i = (int)sampling_rate;
	int speex_sample_len_i = (int)speex_sample_len;
	speex_sample_len_i = 2048;
	//printf("sampling_rate_i = %d,speex_sample_len_i = %d\n",sampling_rate_i,speex_sample_len_i);
	_st = speex_preprocess_state_init(speex_sample_len_i, sampling_rate_i);
	int i=1;
	speex_preprocess_ctl(_st, SPEEX_PREPROCESS_SET_DENOISE, &i);
	i=-25;
	speex_preprocess_ctl(_st, SPEEX_PREPROCESS_SET_NOISE_SUPPRESS, &i); //设置噪声最大衰减的dB

	//静音检测。//测试效果不好导致有些人声帧丢帧，后续优化
	// 	int vad = 1;  
	// 	int vadProbStart = 80;  
	// 	int vadProbContinue = 65;  
	// 	speex_preprocess_ctl(_st, SPEEX_PREPROCESS_SET_VAD, &vad); //静音检测  
	// 	speex_preprocess_ctl(_st, SPEEX_PREPROCESS_SET_PROB_START , &vadProbStart); //Set probability required for the VAD to go from silence to voice   
	// 	speex_preprocess_ctl(_st, SPEEX_PREPROCESS_SET_PROB_CONTINUE, &vadProbContinue); //Set probability required for the VAD to stay in the voice state (integer percent) 
	//
	i=1;
	speex_preprocess_ctl(_st, SPEEX_PREPROCESS_SET_AGC, &i);
	float f=8000*3;
	speex_preprocess_ctl(_st, SPEEX_PREPROCESS_SET_AGC_LEVEL, &f);
	//speex_preprocess_ctl(_st, SPEEX_PREPROCESS_SET_AGC_TARGET, &f);
	i = 30;
	speex_preprocess_ctl(_st, SPEEX_PREPROCESS_SET_AGC_MAX_GAIN, &i);
	i = 12;
	speex_preprocess_ctl(_st, SPEEX_PREPROCESS_SET_AGC_INCREMENT, &i);
	i = 0;
	speex_preprocess_ctl(_st, SPEEX_PREPROCESS_SET_AGC_DECREMENT, &i);

	i=1;
	speex_preprocess_ctl(_st, SPEEX_PREPROCESS_SET_DEREVERB, &i);
	f=.4;
	speex_preprocess_ctl(_st, SPEEX_PREPROCESS_SET_DEREVERB_DECAY, &f);
	f=.3;
	speex_preprocess_ctl(_st, SPEEX_PREPROCESS_SET_DEREVERB_LEVEL, &f);


	//speex_preprocess_ctl(_st, SPEEX_PREPROCESS_GET_ECHO_SUPPRESS , &echo_supp );
	//speex_preprocess_ctl(_st, SPEEX_PREPROCESS_GET_ECHO_SUPPRESS_ACTIVE , &echo_supp_active );
	//回声消除//看效果，有可能手机麦克风硬件自带回声消除功能。
	//echo_state = speex_echo_state_init(speex_sample_len,speex_sample_len*100//);
	echo_state = speex_echo_state_init(speex_sample_len,speex_sample_len*5/*取反射时延的1/3,即100ms的数据大小*/);

	//speex_echo_cancel(echo_state, input_frame, echo_frame, output_frame, residue);

	speex_echo_ctl(echo_state,SPEEX_ECHO_SET_SAMPLING_RATE,&sampling_rate);
	speex_preprocess_ctl(_st, SPEEX_PREPROCESS_SET_ECHO_STATE, echo_state);
	int echo_supp = -50 ;
	int echo_supp_active = -60 ;
	speex_preprocess_ctl(_st, SPEEX_PREPROCESS_SET_ECHO_SUPPRESS , &echo_supp );
	speex_preprocess_ctl(_st, SPEEX_PREPROCESS_SET_ECHO_SUPPRESS_ACTIVE , &echo_supp_active );
	//return (jint)1;
};

extern "C"
JNIEXPORT void JNICALL DeNoiseSpeexUninit(JNIEnv *env, jobject obj)
{
	speex_echo_state_destroy(echo_state);
	speex_preprocess_state_destroy(_st);
};

extern "C"
JNIEXPORT void JNICALL DenoiseProcess(JNIEnv *env, jobject obj, jshortArray lin, jint len/*输入buffer大小，short为单位*/,jshortArray out)
{
	//env->GetShortArrayRegion(lin, 0, len, buffer);
	jshort *inBuffer = new jshort[len];
	env->GetShortArrayRegion(lin, 0, (jsize)len, inBuffer);

	spx_int16_t * ptr=(spx_int16_t *)inBuffer;
	
	if (speex_preprocess_run(_st, ptr))//预处理降噪  
	{  
		//printf("speech,");  
		//fwrite(in, sizeof(short), FRAME_SIZE, fout3);
		//memcpy(dst,src,len);//拷贝,后续可以直接用src返回，看手机效果再优化吧
		 //env->SetShortArrayRegion(lin, offset + i*enc_frame_size, enc_frame_size, buffer);  
		 env->SetShortArrayRegion(out, 0, (jsize)len,    
			 inBuffer);    
	}  
	else   //打开了静音检测的话
	{  
		//printf("slience,");  
		//fwrite(out, sizeof(short), FRAME_SIZE, fout3);
		//memset(dst,0,len);//直接返回0，静音
	}
	delete []inBuffer;
	//return true;
}






