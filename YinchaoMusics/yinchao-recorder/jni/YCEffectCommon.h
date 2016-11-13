#ifndef __YINCHAO_EFFECT_COMMON_H
#define __YINCHAO_EFFECT_COMMON_H
#include <stdint.h>
typedef int (*readSample)(int16_t*input,int size);
typedef int (*writeSample)(int16_t*out,int size);

typedef struct S_EffectCallBack{
	readSample   effReadSample;
	writeSample  effWriteSample;
};





#endif __YINCHAO_EFFECT_COMMON_H