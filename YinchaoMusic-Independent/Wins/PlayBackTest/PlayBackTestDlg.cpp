
// PlayBackTestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PlayBackTest.h"
#include "PlayBackTestDlg.h"
#include "afxdialogex.h"
#include <io.h>
#include <fcntl.h>

static const STInt ID_TIMER_SHOW_PROGRESS = 1;
static const STInt ID_TIMER_PLAY = 2;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


char* W2C(const wchar_t* pw, char* pc)
{
	*pc++ = *pw;
	*pc = *pw>>8;
	return 0;
}

char *wstr2cstr(char *pcstr,const wchar_t *pwstr, size_t len)
{
	int nlength=wcslen(pwstr);

	//获取转换后的长度
	int nbytes = WideCharToMultiByte( 0, // specify the code page used to perform the conversion
		0,         // no special flags to handle unmapped characters
		pwstr,     // wide character string to convert
		nlength,   // the number of wide characters in that string
		NULL,      // no output buffer given, we just want to know how long it needs to be
		0,
		NULL,      // no replacement character given
		NULL );    // we don't want to know if a character didn't make it through the translation

	// make sure the buffer is big enough for this, making it larger if necessary
	if(nbytes>len)   nbytes=len;

	// 通过以上得到的结果，转换unicode 字符为ascii 字符
	WideCharToMultiByte( 0, // specify the code page used to perform the conversion
		0,         // no special flags to handle unmapped characters
		pwstr,   // wide character string to convert
		nlength,   // the number of wide characters in that string
		pcstr, // put the output ascii characters at the end of the buffer
		nbytes,                           // there is at least this much space there
		NULL,      // no replacement character given
		NULL );

	return pcstr ;


	// 	char* pTemp = pcstr;
	// 
	// 	if (pcstr!= NULL && pwstr != NULL)
	// 	{
	// 		size_t wstr_len = wcslen(pwstr);
	// 
	// 		len = (len > wstr_len) ? wstr_len : len;
	// 
	// 		while (--len)
	// 		{
	// 			W2C(pwstr, pcstr);
	// 			pwstr++;
	// 			pcstr +=2;
	// 		}
	// 		*pcstr = '\0';
	// 	}
	// 
	// 	return pTemp ;

}

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CPlayBackTestDlg dialog


void InitConsole()
{
	int nRet= 0;
	FILE* fp;
	AllocConsole();
	nRet= _open_osfhandle((long)GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT);
	fp = _fdopen(nRet, "w");
	*stdout = *fp;
	setvbuf(stdout, NULL, _IONBF, 0);
}


CPlayBackTestDlg::CPlayBackTestDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPlayBackTestDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPlayBackTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_PROGRESS, m_ctrlSlider);
}

BEGIN_MESSAGE_MAP(CPlayBackTestDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_PLAY, &CPlayBackTestDlg::OnBnClickedPlay)
	ON_BN_CLICKED(ID_EXIT, &CPlayBackTestDlg::OnBnClickedExit)
	ON_BN_CLICKED(IDC_PAUSE, &CPlayBackTestDlg::OnBnClickedPause)
	ON_BN_CLICKED(IDC_RESUME, &CPlayBackTestDlg::OnBnClickedResume)
	ON_BN_CLICKED(IDC_STOP, &CPlayBackTestDlg::OnBnClickedStop)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_PROGRESS, &CPlayBackTestDlg::OnNMReleasedcaptureProgress)
	ON_WM_TIMER(ID_TIMER_SHOW_PROGRESS, &CPlayBackTestDlg::OnTimer)
	ON_WM_TIMER(ID_TIMER_PLAY, &CPlayBackTestDlg::OnTimer)	
	ON_BN_CLICKED(IDC_BUTTON1, &CPlayBackTestDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CPlayBackTestDlg message handlers

BOOL CPlayBackTestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	m_ctrlSlider.SetRange(0,1000);
	// TODO: Add extra initialization here
	InitConsole();
	
	iMediaPlayer = new STMediaPlayer(*this);	

	SetTimer(ID_TIMER_SHOW_PROGRESS, 1000, NULL);
	SetTimer(ID_TIMER_PLAY, 1000, NULL);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPlayBackTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CPlayBackTestDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CPlayBackTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CPlayBackTestDlg::OnBnClickedPlay()
{
#ifdef __ST_DEBUG__
	wchar_t* filename = L"C:\\Documents and Settings\\hucao\\桌面\\MUSIC\\test.aac";
#else
	wchar_t* filename = L"E:\\whistle.aac";
//wchar_t* filename = L"E:\\whistle.aac";
#endif
	//	wchar_t* filename = L"C:\\1.flac";


	//wchar_t* filename = (wchar_t*)(LPCTSTR)(iFileArray.at(iCurPlayIdx));

	char *pcstr = (char *)malloc(sizeof(char)*(2 * wcslen(filename)+1));
	memset(pcstr , 0 , 2 * wcslen(filename)+1 );
	wstr2cstr(pcstr,filename,2 * wcslen(filename)+1) ;

	printf("SetDataSource: %s\n", pcstr);
	//if (STKErrNone == iMediaPlayer->SetDataSource(pcstr, ""))
	//char path[] = "E:\\";
	//char param[] = "background_name=whistle.aac&origin_name=一万个舍不得.aac&support_record=true";
	if (STKErrNone == iMediaPlayer->SetDataSource(pcstr, NULL))
	{	
		iMediaPlayer->Play();	
	}

	delete pcstr;
	pcstr = NULL;
}


void CPlayBackTestDlg::OnBnClickedExit()
{
	// TODO: Add your control notification handler code here
	iMediaPlayer->Stop();
	SAFE_RELEASE(iMediaPlayer);
	OnCancel();
}

void CPlayBackTestDlg::PlayerNotifyEvent(STNotifyMsgId aMsg, STInt aError)
{
	printf("aMsg:%d,aError:%d\n", aMsg, aError);
}


void CPlayBackTestDlg::OnBnClickedPause()
{
	// TODO: Add your control notification handler code here
	iMediaPlayer->Pause();
}


void CPlayBackTestDlg::OnBnClickedResume()
{
	// TODO: Add your control notification handler code here
	iMediaPlayer->Resume();
}


void CPlayBackTestDlg::OnBnClickedStop()
{
	// TODO: Add your control notification handler code here
	iMediaPlayer->Stop();
}

void CPlayBackTestDlg::OnNMReleasedcaptureProgress( NMHDR *pNMHDR, LRESULT *pResult )
{
	STUint nDuration = iMediaPlayer->Duration();
	if (nDuration != 0)
	{
		STInt64 nPos = nDuration;
		nPos *= m_ctrlSlider.GetPos();
		nPos /= 1000;
		iMediaPlayer->Seek(nPos);
		printf("CurPos:%d\n",iMediaPlayer->GetPosition());
	}
}

void CPlayBackTestDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (ID_TIMER_SHOW_PROGRESS == nIDEvent)
	{	
		if (EStatusPlaying == iMediaPlayer->GetPlayStatus())
		{
			STUint nDuration = iMediaPlayer->Duration();
			if (nDuration != 0)
			{
				STUint nCurPos = iMediaPlayer->GetPosition();			
				STInt64 nTemp = nCurPos;
				nTemp *= 1000;				
				m_ctrlSlider.SetPos(nTemp / nDuration);
			}
			else
			{
				m_ctrlSlider.SetPos(0);
			}
		}
	}
	else if (ID_TIMER_PLAY == nIDEvent)
	{
// 		if (EStatusStoped == iMediaPlayer->GetPlayStatus())
// 		{
// 			OnBnClickedPlay();
// 		}
	}
}


void CPlayBackTestDlg::OnBnClickedButton1()
{
// 	wchar_t* filename = L"C:\\Vitas_星星.wav";
// 	//	wchar_t* filename = L"C:\\1.flac";
// 
// 
// 	//wchar_t* filename = (wchar_t*)(LPCTSTR)(iFileArray.at(iCurPlayIdx));
// 
// 	char *pcstr = (char *)malloc(sizeof(char)*(2 * wcslen(filename)+1));
// 	memset(pcstr , 0 , 2 * wcslen(filename)+1 );
// 	wstr2cstr(pcstr,filename,2 * wcslen(filename)+1) ;
// 
// 	STMediaPlayer* mediaPlayer = new STMediaPlayer(*this);
// 	mediaPlayer->SetDataSource(pcstr, NULL);
// 	mediaPlayer->Play();
}
