#ifndef __TT_AUDIOPLUGINMANAGER_H__
#define __TT_AUDIOPLUGINMANAGER_H__
#include "TTMediainfoDef.h"
#include "TTMediadef.h"
#include "TTCritical.h"
#include "ttMP3Dec.h"
//#include "ttAACDec.h"

class CTTAudioPluginManager
{
public:
	CTTAudioPluginManager();//单例模式不能在指纹识别的解码的线程中使用，只能重新创建实例
	~CTTAudioPluginManager();

	TTInt32							initPlugin(TTUint aFormat, void* aInitParam, TTBool aDecodeOrEncode = ETTFalse);
	TTInt32							uninitPlugin();
	TTInt32							resetPlugin();
	TTInt32							setInput(TTBuffer *InBuffer);
	TTInt32							process(TTBuffer * OutBuffer, TTAudioFormat* pOutInfo);
	TTInt32							setParam(TTInt32 uParamID, TTPtr pData);
	TTInt32							getParam(TTInt32 uParamID, TTPtr pData);

	/**
	* \fn							void SetPluginPath(const TTChar* aPath)
	* \brief						设置音效插件路径
	* \param[in] aPath				音效插件路径
	*/
	static void						setPluginPath(const TTChar* aPath);
	
private:
	TTInt32							LoadLib ();

	TTAudioCodecAPI					mAudioCodecAPI;
	TTHandle						mHandle;
	TTUint							mFormat;

	RTTCritical						mCritical;

	TTHandle						mLibHandle;
	static TTChar					mPluginPath[256];
	
#ifdef PERFORMANCE_PROFILE
	TTUint64						tAudioDecode;
#endif

};


#endif
