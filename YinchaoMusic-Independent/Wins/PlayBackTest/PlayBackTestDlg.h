
// PlayBackTestDlg.h : header file
//
#include "STMediaPlayer.h"

#pragma once


// CPlayBackTestDlg dialog
class CPlayBackTestDlg : public CDialogEx, public ISTMediaPlayerObserver
{
// Construction
public:
	CPlayBackTestDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_PLAYBACKTEST_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedPlay();
	afx_msg void OnBnClickedExit();
	afx_msg void OnNMReleasedcaptureProgress(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTimer(UINT_PTR nIDEvent);

private:
	void							PlayerNotifyEvent(STNotifyMsgId aMsg, STInt aError);

private:
	ISTMediaPlayerItf*				iMediaPlayer;
	CSliderCtrl						m_ctrlSlider;
public:
	afx_msg void OnBnClickedPause();
	afx_msg void OnBnClickedResume();
	afx_msg void OnBnClickedStop();
	afx_msg void OnBnClickedButton1();
};
