#ifndef __X_RUN_TIME_H__
#define __X_RUN_TIME_H__
#include "STMacrodef.h"
#include "jni.h"
#include "STTypedef.h"

class STRunTime
{
public:
	static void Init(JavaVM* ajvm);
	static STInt AttachCurrentThread();
	static STInt DetachCurrentThread();
	static JNIEnv* GetJNIEnv();
private:

	static JavaVM* iJavaVM;
};
#endif
