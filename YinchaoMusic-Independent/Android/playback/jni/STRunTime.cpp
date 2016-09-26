#include "STRunTime.h"
#include "STLog.h"


JavaVM* STRunTime::iJavaVM = NULL;
void STRunTime::Init(JavaVM* ajvm)
{
	iJavaVM = ajvm;
}

STInt STRunTime::AttachCurrentThread()
{
	JNIEnv* pEnv = NULL;
	return iJavaVM->AttachCurrentThread(&pEnv, NULL);
}

STInt STRunTime::DetachCurrentThread()
{
	return iJavaVM->DetachCurrentThread();
}

JNIEnv* STRunTime::GetJNIEnv()
{
	JNIEnv* env = NULL;
	STASSERT(iJavaVM != NULL);

	if (iJavaVM->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK)
	{
		STLOGE("Error");
		return NULL;
	}

	return env;
}
