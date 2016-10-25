// PlayerTestDlg.h : 头文件
//
#include "TTMediaPlayerItf.h"

#include <vector>
#include "afxcmn.h"
#include "afxwin.h"
#include "TTCritical.h"
#include "VideoView.h"
using namespace std;
#pragma once

class AFPTest;

#define WM_STOPMESSAGE (WM_USER+100)
#define WM_DOWNLOADMESSAGE (WM_USER+101)
#define WM_PREPAREDMESSAGE (WM_USER+102)
#define WM_BUFFERSTARTMESSAGE (WM_USER+103)
#define WM_BUFFERENDMESSAGE (WM_USER+104)

// CPlayerTestDlg 对话框
class CPlayerTestDlg : public CDialog, public ITTMediaPlayerObserver
{
// 构造
public:
	CPlayerTestDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_PLAYERTEST_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()


public:
	void PlayerNotifyEvent(TTNotifyMsg aMsg, TTInt aArg1, TTInt aArg2, const TTChar* aArg3);
	

private:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedStop();
	afx_msg void OnBnClickedExit();
	afx_msg void OnBnClickedPause();
	afx_msg void OnBnClickedResume();
	afx_msg void OnNMReleasedcaptureProgress(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT OnStopMessage(WPARAM w,LPARAM l);
	afx_msg LRESULT OnDownloadMessage(WPARAM w,LPARAM l);
	afx_msg LRESULT OnPreparedMessage(WPARAM w,LPARAM l);
	afx_msg LRESULT OnBufferStartMessage(WPARAM w,LPARAM l);
	afx_msg LRESULT OnBufferEndMessage(WPARAM w,LPARAM l);

	bool CreateWnd ();
	void FlushWnd ();
private:
	bool			m_Started;
	ITTMediaPlayer*	iMediaPlayer;


	enum TTTestStatus
	{
		ETestStartPlay,
		ETestPause,
		ETestResume,
		ETestStop,
		ETestReStart
	};

	enum TTTestStatus2
	{
		ETestStatus2PauseResume = 0
		,ETestStatus2PauseResumePaused = 1
		,ETestStatus2Seek = 2
		,ETestStatus2PauseSeekResume = 3
		,ETestStatus2PauseSeekResumePaused = 4
		,ETestStatus2PauseSeekResumeSeeked = 5
		,ETestStatus2Next = 6
	};

	TTTestStatus	iTestStaus;

	CSliderCtrl m_ctrlSlider;
	CSliderCtrl m_ctrlVolume;
	unsigned int iDuration;
	unsigned int iMediaType;
	TTInt16		 iLevelRange[2];
	TTInt		 iEffectInit;
	TTInt		 iOpenSync;
	int* pFreq;

	TTInt16* mWave;
	int* mFreq;

	CVideoView*		m_WView;
	HWND			m_hView;
	RECT			m_rcView;

	afx_msg void OnNMReleasedcaptureSlider(NMHDR *pNMHDR, LRESULT *pResult);
	void TravelFolder(CString strDir, int nDepth);

	static LRESULT CALLBACK ViewWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


	std::vector<CString> iFileArray;
	int					 iCurPlayIdx;
	afx_msg void OnBnClickedPre();
	afx_msg void OnBnClickedNext();

	int					 iTestCycleDelay;
	TTTestStatus2		 iTestStaus2;
public:
	~CPlayerTestDlg();
	afx_msg void OnBnClickedRadioEqPop();
	afx_msg void OnBnClickedRadioEqFlat();
	afx_msg void OnBnClickedRadioEqMetal();
	afx_msg void OnBnClickedRadioEqRock();
	afx_msg void OnBnClickedRadioEqJazz();
	afx_msg void OnNMReleasedcaptureSlider2(NMHDR *pNMHDR, LRESULT *pResult);

private:
	afx_msg void OnBnClickedCheckBassBoost();
	afx_msg void OnBnClickedCheckVirtualizer();
	afx_msg void OnBnClickedCheckReverb();

private:
	void DisableEqPresetCtrls();

	void InitEq();

	void SetEqBandLevel(TTInt16 aBand, TTInt aPos);

	void InitVolume();

	void InitBassBoost();

	void InitTrebleBoost();

	void InitVirtualizer();

	void InitReverb();

	CButton m_ctrlEqPresetPop;
	CButton m_ctrlEqPresetFlat;
	CButton m_ctrlEqPresetMetal;
	CButton m_ctrlEqPresetRock;
	CButton m_ctrlEqPresetJazz;
	CSliderCtrl m_ctrlEqBand1;
	CSliderCtrl m_ctrlEqBand2;
	CSliderCtrl m_ctrlEqBand3;
	CSliderCtrl m_ctrlEqBand4;
	CSliderCtrl m_ctrlEqBand5;
	CSliderCtrl m_ctrlEqBand6;
	CSliderCtrl m_ctrlEqBand7;
	CSliderCtrl m_ctrlEqBand8;
	CSliderCtrl m_ctrlEqBand9;
	CSliderCtrl m_ctrlEqBand10;
public:
	afx_msg void OnNMReleasedcaptureSlider3(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMReleasedcaptureSlider4(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMReleasedcaptureSlider5(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMReleasedcaptureSlider6(NMHDR *pNMHDR, LRESULT *pResult);
private:
	unsigned int mPrePos;
public:
	bool mBassBoostEnable;
	bool mTrebleBoostEnable;
	CSliderCtrl m_ctrlBassBoost;
	bool mVirtualizerEnable;
	CSliderCtrl m_ctrlVirtualizer;
	afx_msg void OnNMReleasedcaptureBassBoost(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMReleasedcaptureVirtualzer(NMHDR *pNMHDR, LRESULT *pResult);
public:
	bool mReverbEnable;
	afx_msg void OnBnClickedRadioReverbSmallRoom();
	afx_msg void OnBnClickedRadioReverbMediumRoom();
	afx_msg void OnBnClickedRadioReverbLargeRoom();
	afx_msg void OnBnClickedRadioReverbMediumHall();
	afx_msg void OnBnClickedRadioReverbLargeHall();
	afx_msg void OnBnClickedRadioReverbPlate();
public:
	afx_msg void OnBnClickedCheckTreble();
public:
	afx_msg void OnNMReleasedcaptureSldTreble(NMHDR *pNMHDR, LRESULT *pResult);
public:
	CSliderCtrl m_ctrlTrebleBoost;
	afx_msg void OnNMReleasedcaptureSlider10(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMReleasedcaptureSlider11(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMReleasedcaptureSlider12(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMReleasedcaptureSlider13(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMReleasedcaptureSlider14(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBtnEqReset();
};
