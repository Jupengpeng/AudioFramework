#pragma once


// CVideoView dialog

class CVideoView : public CDialog
{
	DECLARE_DYNAMIC(CVideoView)

public:
	CVideoView(CWnd* pParent = NULL);   // standard constructor
	virtual ~CVideoView();

	void DrawRect();

// Dialog Data
	enum { IDD = IDD_VIDEOVIEW };

	int mFreq[512];

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
};
