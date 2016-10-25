#include <stdio.h>
#include <jni.h>
#include "TTJniEnvUtil.h"

#define  LOG_TAG    "CJniEnvUtil"
#include "TTLog.h"

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
				LOGE("callback_handler: failed to attach current thread");
				break;
			} 			
			break; 
		case JNI_EVERSION: 
			LOGE("Invalid java version"); 
			break;
		}
}

CJniEnvUtil::~CJniEnvUtil()
{
	if (m_fNeedDetach) 
		 mJavaVM->DetachCurrentThread(); 
}
 
