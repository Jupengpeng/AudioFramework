#include "YCBaseEffect.h"

int main()
{

	YCEffectParam *pYCEffectParam= new YCEffectParam();
	CBaseEffect *pBaseEffect = new CBaseEffect(44100,1,16);
	pBaseEffect->setEffectParams(pYCEffectParam,"test.pcm","output.pcm");
	pBaseEffect->doEffect();
	pBaseEffect->destroy();


}