#ifdef 	_LINUX_ANDROID
#ifndef __TTJniEnvUtil_H__
#define __TTJniEnvUtil_H__

class CJniEnvUtil
{
protected:
	bool 			m_fNeedDetach;
	JavaVM 			*mJavaVM;
	JNIEnv 			*m_pEnv;

	CJniEnvUtil(const CJniEnvUtil&);
	CJniEnvUtil& operator=(const CJniEnvUtil&);
public:
	CJniEnvUtil(JavaVM *pvm);
	~CJniEnvUtil();

	JNIEnv* getEnv() { return m_pEnv; } 
};
#endif
#endif
