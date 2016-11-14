#include <malloc.h>
#include <string.h>
#include <jni.h>
#include <android/log.h>
#include <stdio.h>
#include "IAudioManager.h"
#include "AndroidAudioManager.h"
#include "com_yinchao_media_recoder_YCAudioRecoder.h"
#include<assert.h>

JavaVM* gJVM = NULL;
static const char* const kClassPathName="com/yinchao/media/recoder/YCAudioRecoder";
static const char* const kClassFieldName = "mNativeAudioManagerPara";

static sox_format_t * in, * out; /* input and output files */

/* The function that will be called to input samples into the effects chain.
 * In this example, we get samples to process from a SoX-openned audio file.
 * In a different application, they might be generated or come from a different
 * part of the application. */
static int input_drain(
    sox_effect_t * effp, sox_sample_t * obuf, size_t * osamp)
{
  (void)effp;   /* This parameter is not needed in this example */

  /* ensure that *osamp is a multiple of the number of channels. */
  *osamp -= *osamp % effp->out_signal.channels;
	LOGI2("read");
  /* Read up to *osamp samples into obuf; store the actual number read
   * back to *osamp */
  *osamp = sox_read(in, obuf, *osamp);

  /* sox_read may return a number that is less than was requested; only if
   * 0 samples is returned does it indicate that end-of-file has been reached
   * or an error has occurred */
  if (!*osamp && in->sox_errno)
    fprintf(stderr, "%s: %s\n", in->filename, in->sox_errstr);
  return *osamp? SOX_SUCCESS : SOX_EOF;
}

/* The function that will be called to output samples from the effects chain.
 * In this example, we store the samples in a SoX-opened audio file.
 * In a different application, they might perhaps be analysed in some way,
 * or displayed as a wave-form */
static int output_flow(sox_effect_t *effp , sox_sample_t const * ibuf,
    sox_sample_t * obuf, size_t * isamp, size_t * osamp)
{
  /* Write out *isamp samples */
  size_t len = sox_write(out, ibuf, *isamp);
	LOGI2("write");
  /* len is the number of samples that were actually written out; if this is
   * different to *isamp, then something has gone wrong--most often, it's
   * out of disc space */
  if (len != *isamp) {
    fprintf(stderr, "%s: %s\n", out->filename, out->sox_errstr);
    return SOX_EOF;
  }

  /* Outputting is the last `effect' in the effect chain so always passes
   * 0 samples on to the next effect (as there isn't one!) */
  *osamp = 0;

  (void)effp;   /* This parameter is not needed in this example */

  return SOX_SUCCESS; /* All samples output successfully */
}

/* A `stub' effect handler to handle inputting samples to the effects
 * chain; the only function needed for this example is `drain' */
static sox_effect_handler_t const * input_handler(void)
{
  static sox_effect_handler_t handler = {
    "input", NULL, SOX_EFF_MCHAN, NULL, NULL, NULL, input_drain, NULL, NULL, 0
  };
  return &handler;
}

/* A `stub' effect handler to handle outputting samples from the effects
 * chain; the only function needed for this example is `flow' */
static sox_effect_handler_t const * output_handler(void)
{
  static sox_effect_handler_t handler = {
    "output", NULL, SOX_EFF_MCHAN, NULL, NULL, output_flow, NULL, NULL, NULL, 0
  };
  return &handler;
}

/*
 * Reads input file, applies vol & flanger effects, stores in output file.
 * E.g. example1 monkey.au monkey.aiff
 */
int test()
{
	LOGI2("test1");
  sox_effects_chain_t * chain;
  sox_effect_t * e;
  sox_signalinfo_t signal;
  sox_encodinginfo_t encode;
  char * vol[] = {"3dB"};
  
  memset(&signal,0,sizeof(signal));
  memset(&encode,0,sizeof(encode));

  signal.channels =1;
  signal.rate =44100;
  signal.precision =16;

  encode.bits_per_sample =16;
  //encode.encoding =SOX_ENCODING_ALAW;
  encode.encoding= SOX_ENCODING_SIGN2;
  /* All libSoX applications must start by initialising the SoX library */
  sox_init();

  /* Open the input file (with default parameters) */
  in = sox_open_read("/sdcard/test.pcm", &signal, &encode, "raw");
	LOGI2("test2");
  /* Open the output file; we must specify the output signal characteristics.
   * Since we are using only simple effects, they are the same as the input
   * file characteristics */
  out = sox_open_write("/sdcard/temp.pcm", &signal, &encode, "raw", NULL, NULL);
	LOGI2("test3");
  /* Create an effects chain; some effects need to know about the input
   * or output file encoding so we provide that information here */
  chain = sox_create_effects_chain(&encode, &encode);

  /* The first effect in the effect chain must be something that can source
   * samples; in this case, we have defined an input handler that inputs
   * data from an audio file */
  e = sox_create_effect(input_handler());
  /* This becomes the first `effect' in the chain */
  sox_add_effect(chain, e, &signal, &signal);

  /* Create the `vol' effect, and initialise it with the desired parameters: */
  e = sox_create_effect(sox_find_effect("vol"));
  sox_effect_options(e, 1, vol) ;
  /* Add the effect to the end of the effects processing chain: */
  sox_add_effect(chain, e, &signal, &signal);

  /* Create the `flanger' effect, and initialise it with default parameters: */
  e = sox_create_effect(sox_find_effect("flanger"));
  sox_effect_options(e, 0, NULL);
  /* Add the effect to the end of the effects processing chain: */
  sox_add_effect(chain, e, &signal, &signal);

  /* The last effect in the effect chain must be something that only consumes
   * samples; in this case, we have defined an output handler that outputs
   * data to an audio file */
  e = sox_create_effect(output_handler());
  sox_add_effect(chain, e, &signal, &signal);

  /* Flow samples through the effects processing chain until EOF is reached */
  sox_flow_effects(chain, NULL, NULL);
	LOGI2("test4");
  /* All done; tidy up: */
  sox_delete_effects_chain(chain);
  sox_close(out);
  sox_close(in);
  sox_quit();
  return 0;
}


void JNICALL Java_com_yinchao_media_recoder_YCAudioRecoder_initRecord
	(JNIEnv *jni_env, jobject thiz, jint sampleRate, jint channels, jint sampleBit){
	IAudioManager *pAudioManager = new AndroidAudioManager();
	jni_env->GetJavaVM(&gJVM);
	pAudioManager->setAudioFormat(sampleRate,channels,sampleBit);
	pAudioManager->SetNativeWindow((void*)gJVM);
	pAudioManager->Init();

	jclass className = jni_env->FindClass(kClassPathName);
	jfieldID AudioManagerPara = jni_env->GetFieldID(className,kClassFieldName, "I"); 
	jni_env->SetIntField(thiz,AudioManagerPara, (long)pAudioManager);
	jni_env->DeleteLocalRef(className);

}

void JNICALL Java_com_yinchao_media_recoder_YCAudioRecoder_startRecord
	(JNIEnv *jni_env, jobject thiz, jint context){
		IAudioManager* pAudioManager = (AndroidAudioManager*)context;
		if (pAudioManager == NULL)
		{
			return;
		}
		test();
		//pAudioManager->start();
}



void JNICALL Java_com_yinchao_media_recoder_YCAudioRecoder_stopRecord
	(JNIEnv *jni_env, jobject thiz, jint context){
		//start recording
		IAudioManager* pAudioManager = (AndroidAudioManager*)context;
		if (pAudioManager == NULL)
		{
			return;
		}
		pAudioManager->stop();

}
