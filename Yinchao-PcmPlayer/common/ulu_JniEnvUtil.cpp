#ifdef 	_LINUX_ANDROID
#include <stdio.h>
#include <jni.h>
#include "ulu_JniEnvUtil.h"


CJniEnvUtil::CJniEnvUtil(JavaVM *pvm)
: m_fNeedDetach(false)
, mJavaVM(pvm)
, m_pEnv(NULL)
{
	switch (mJavaVM->GetEnv((void**)&m_pEnv, JNI_VERSION_1_6)) { 
		case JNI_OK: 
			break; 
		case JNI_EDETACHED: 
			m_fNeedDetach = true;
			if (mJavaVM->AttachCurrentThread(&m_pEnv, NULL) != 0) { 
				//ULULOGE("callback_handler: failed to attach current thread");
				break;
			} 			
			break; 
		case JNI_EVERSION: 
			//ULULOGE("Invalid java version"); 
			break;
		}
}

CJniEnvUtil::~CJniEnvUtil()
{
	if (m_fNeedDetach) 
		 mJavaVM->DetachCurrentThread(); 
}

 #endif
