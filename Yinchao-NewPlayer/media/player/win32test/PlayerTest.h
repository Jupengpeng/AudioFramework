// PlayerTest.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CPlayerTestApp:
// �йش����ʵ�֣������ PlayerTest.cpp
//

class CPlayerTestApp : public CWinApp
{
public:
	CPlayerTestApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CPlayerTestApp theApp;