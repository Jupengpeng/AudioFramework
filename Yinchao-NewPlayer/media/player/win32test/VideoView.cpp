// VideoView.cpp : implementation file
//

#include "stdafx.h"
#include "PlayerTest.h"
#include "VideoView.h"


// CVideoView dialog

IMPLEMENT_DYNAMIC(CVideoView, CDialog)

CVideoView::CVideoView(CWnd* pParent /*=NULL*/)
	: CDialog(CVideoView::IDD, pParent)
{
	memset(mFreq, 0, 512 * sizeof(int));
}

CVideoView::~CVideoView()
{
}

void CVideoView::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

void CVideoView::DrawRect()
{
	PAINTSTRUCT ps;
	CDC * hdc = BeginPaint(&ps);

	RECT rc;
	GetClientRect(&rc);
	HBRUSH hBrush = ::CreateSolidBrush(RGB(255,255,255));
	FillRect(GetDC()->GetSafeHdc(), &rc, hBrush);
	DeleteObject(hBrush);
	EndPaint(&ps);

	CClientDC dc(this);//ѡ����ǰ��ͼ����
	CPen pen(PS_SOLID,1,0xff0000);//��һ֧��ɫ��ϸΪ1�ı�
	dc.SelectObject(&pen);//��pen�ŵ�dc��

	int* pFreq = mFreq;
	int nBottom = 200;
	int nLeft = 80;
	for (int n = 0; n < 32; n++)
	{		 
		if (*(pFreq + n) < 0)
		{
			int x = 0;
			x++;
		}
		dc.Rectangle(nLeft, nBottom - (*(pFreq + n)), nLeft + 4, nBottom);
		nLeft += 5;
	}
}

BEGIN_MESSAGE_MAP(CVideoView, CDialog)
END_MESSAGE_MAP()


// CVideoView message handlers
