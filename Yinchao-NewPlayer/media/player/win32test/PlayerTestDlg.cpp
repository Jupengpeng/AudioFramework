// PlayerTestDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "PlayerTest.h"
#include "PlayerTestDlg.h"
#include <io.h>
#include <fcntl.h>
#include "Normalizer.c"
#include "TTMediaPlayer.h"
#include "TTSleep.h"
//#include "TTMediaBuffer.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif

TTBool gAudioEffectLowDelay = ETTTrue;

//#define __TEST_DELETEMEDIAPLAY__//每次删除实例,否则删除
#define __PlayComplete__

TTInt ID_TIMER_TEST_PLAY = 5;
TTInt ID_TIMER_SHOW_PROGRESS = 6;
TTInt ID_TIMER_SHOW_FREQ = 7;
TTInt ID_TIMER_TEST_HYBRID = 8;



static CRITICAL_SECTION gCriticalSection;

enum EQ_ID
{
	EQ_Normal, 
	EQ_Classical,
	EQ_Dance,
	EQ_Flat,
	EQ_Folk,
	EQ_HeavyMetal,
	EQ_HipHop,
	EQ_Jazz,
	EQ_Rock,
	EQ_Pop
};

enum Reverb_ID
{
	Reverb_None,
	Reverb_SmallRoom,
	Reverb_MediumRoom,
	Reverb_LargeRoom,
	Reverb_MediumHall,
	Reverb_LargeHall,
	Reverb_Plate
};

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

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

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{

}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CPlayerTestDlg 对话框

CPlayerTestDlg::CPlayerTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPlayerTestDlg::IDD, pParent)
	, iDuration(0)
	, iMediaType(0)
	, iEffectInit(0)
	, mPrePos(0)
	, m_WView(NULL)
	, m_hView(NULL)
	, mBassBoostEnable(false)
	, mTrebleBoostEnable(false)
	, mVirtualizerEnable(false)
	, mReverbEnable(false)
	, iOpenSync(0)
{
	iTestStaus2 = ETestStatus2PauseResume;
	iTestCycleDelay = 0;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	iMediaPlayer = new CTTMediaPlayer(this, "..\\..\\..\\win32libs");

	m_Started = false;

	iTestStaus = ETestReStart;

	//iEqualizer = new CTTAudioEffect(&EQUALIZER_UUID_);

	pFreq = NULL;
}

void CPlayerTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_PROGRESS, m_ctrlSlider);
	DDX_Control(pDX, IDC_SLIDER1, m_ctrlVolume);


	DDX_Control(pDX, IDC_SLIDER2, m_ctrlEqBand1);
	DDX_Control(pDX, IDC_RADIO1, m_ctrlEqPresetPop);
	DDX_Control(pDX, IDC_RADIO2, m_ctrlEqPresetFlat);
	DDX_Control(pDX, IDC_RADIO3, m_ctrlEqPresetMetal);
	DDX_Control(pDX, IDC_RADIO4, m_ctrlEqPresetRock);
	DDX_Control(pDX, IDC_RADIO5, m_ctrlEqPresetJazz);
	DDX_Control(pDX, IDC_SLIDER3, m_ctrlEqBand2);
	DDX_Control(pDX, IDC_SLIDER4, m_ctrlEqBand3);
	DDX_Control(pDX, IDC_SLIDER5, m_ctrlEqBand4);
	DDX_Control(pDX, IDC_SLIDER6, m_ctrlEqBand5);
	DDX_Control(pDX, IDC_SLIDER7, m_ctrlBassBoost);
	DDX_Control(pDX, IDC_SLIDER8, m_ctrlVirtualizer);
	DDX_Control(pDX, IDC_SLD_TREBLE, m_ctrlTrebleBoost);
	DDX_Control(pDX, IDC_SLIDER10, m_ctrlEqBand6);
	DDX_Control(pDX, IDC_SLIDER11, m_ctrlEqBand7);
	DDX_Control(pDX, IDC_SLIDER12, m_ctrlEqBand8);
	DDX_Control(pDX, IDC_SLIDER13, m_ctrlEqBand9);
	DDX_Control(pDX, IDC_SLIDER14, m_ctrlEqBand10);
}

BEGIN_MESSAGE_MAP(CPlayerTestDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &CPlayerTestDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_STOP, &CPlayerTestDlg::OnBnClickedStop)
	ON_WM_TIMER(ID_TIMER_TEST_PLAY, &CPlayerTestDlg::OnTimer)
	ON_WM_TIMER(ID_TIMER_SHOW_PROGRESS, &CPlayerTestDlg::OnTimer)
	ON_WM_TIMER(ID_TIMER_SHOW_FREQ, &CPlayerTestDlg::OnTimer)
	ON_WM_TIMER(ID_TIMER_TEST_HYBRID, &CPlayerTestDlg::OnTimer)

	ON_BN_CLICKED(IDC_Exit, &CPlayerTestDlg::OnBnClickedExit)
	ON_BN_CLICKED(IDC_PAUSE, &CPlayerTestDlg::OnBnClickedPause)
	ON_BN_CLICKED(IDC_RESUME, &CPlayerTestDlg::OnBnClickedResume)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_PROGRESS, &CPlayerTestDlg::OnNMReleasedcaptureProgress)
	ON_WM_ERASEBKGND()
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER1, &CPlayerTestDlg::OnNMReleasedcaptureSlider)
	ON_BN_CLICKED(IDC_PRE, &CPlayerTestDlg::OnBnClickedPre)
	ON_BN_CLICKED(IDC_NEXT, &CPlayerTestDlg::OnBnClickedNext)
	ON_BN_CLICKED(IDC_RADIO1, &CPlayerTestDlg::OnBnClickedRadioEqPop)
	ON_BN_CLICKED(IDC_RADIO2, &CPlayerTestDlg::OnBnClickedRadioEqFlat)
	ON_BN_CLICKED(IDC_RADIO3, &CPlayerTestDlg::OnBnClickedRadioEqMetal)
	ON_BN_CLICKED(IDC_RADIO4, &CPlayerTestDlg::OnBnClickedRadioEqRock)
	ON_BN_CLICKED(IDC_RADIO5, &CPlayerTestDlg::OnBnClickedRadioEqJazz)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER2, &CPlayerTestDlg::OnNMReleasedcaptureSlider2)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER3, &CPlayerTestDlg::OnNMReleasedcaptureSlider3)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER4, &CPlayerTestDlg::OnNMReleasedcaptureSlider4)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER5, &CPlayerTestDlg::OnNMReleasedcaptureSlider5)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER6, &CPlayerTestDlg::OnNMReleasedcaptureSlider6)
	ON_BN_CLICKED(IDC_CHECK1, &CPlayerTestDlg::OnBnClickedCheckBassBoost)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER7, &CPlayerTestDlg::OnNMReleasedcaptureBassBoost)
	ON_BN_CLICKED(IDC_CHECK3, &CPlayerTestDlg::OnBnClickedCheckVirtualizer)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER8, &CPlayerTestDlg::OnNMReleasedcaptureVirtualzer)
	ON_BN_CLICKED(IDC_CHECK2, &CPlayerTestDlg::OnBnClickedCheckReverb)
	ON_BN_CLICKED(IDC_RADIO6, &CPlayerTestDlg::OnBnClickedRadioReverbSmallRoom)
	ON_BN_CLICKED(IDC_RADIO7, &CPlayerTestDlg::OnBnClickedRadioReverbMediumRoom)
	ON_BN_CLICKED(IDC_RADIO8, &CPlayerTestDlg::OnBnClickedRadioReverbLargeRoom)
	ON_BN_CLICKED(IDC_RADIO9, &CPlayerTestDlg::OnBnClickedRadioReverbMediumHall)
	ON_BN_CLICKED(IDC_RADIO10, &CPlayerTestDlg::OnBnClickedRadioReverbLargeHall)
	ON_BN_CLICKED(IDC_RADIO11, &CPlayerTestDlg::OnBnClickedRadioReverbPlate)
	ON_BN_CLICKED(IDC_CHECK_TREBLE, &CPlayerTestDlg::OnBnClickedCheckTreble)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLD_TREBLE, &CPlayerTestDlg::OnNMReleasedcaptureSldTreble)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER10, &CPlayerTestDlg::OnNMReleasedcaptureSlider10)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER11, &CPlayerTestDlg::OnNMReleasedcaptureSlider11)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER12, &CPlayerTestDlg::OnNMReleasedcaptureSlider12)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER13, &CPlayerTestDlg::OnNMReleasedcaptureSlider13)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER14, &CPlayerTestDlg::OnNMReleasedcaptureSlider14)
	ON_BN_CLICKED(IDC_BTN_EQ_RESET, &CPlayerTestDlg::OnBnClickedBtnEqReset)
	ON_MESSAGE(WM_STOPMESSAGE, OnStopMessage)
	ON_MESSAGE(WM_STOPMESSAGE, OnDownloadMessage)
	ON_MESSAGE(WM_PREPAREDMESSAGE, OnPreparedMessage)
	ON_MESSAGE(WM_BUFFERSTARTMESSAGE, OnBufferStartMessage)
	ON_MESSAGE(WM_BUFFERENDMESSAGE, OnBufferEndMessage)
END_MESSAGE_MAP()

static TTInt KWaveSamples = 512;
// CPlayerTestDlg 消息处理程序

BOOL CPlayerTestDlg::OnInitDialog()
{
	InitializeCriticalSection(&gCriticalSection);
	InitConsole();

	//_CrtSetBreakAlloc(158635);


	//TravelFolder(CString(_T("E:\\测试集\\全歌曲格式\\m4a")),0);//84
	//TravelFolder(CString(_T("E:\\测试集\\ALAC")),0);//84
	//TravelFolder(CString(_T("E:\\测试集\\全歌曲格式\\ape\\立体声")),0);//84
	//TravelFolder(CString(_T("E:\\测试集\\全歌曲格式\\wav\\立体声\\1.41M")),0);//84
	//TravelFolder(CString(_T("C:\\temp")),0);//84
	//TravelFolder(CString(_T("E:\\测试集\\歌曲\\320")),0);//84
	
	TravelFolder(CString(_T("D:\\Music")),0);//84
	
	
	//iAfpTest = new AFPTest();
	


	//iCurPlayIdx = 10;
	iCurPlayIdx = 0;

	//TTASSERT(iFileArray.size() > 0);

	//HANDLE pHandle = GetCurrentThread();
	//::SetThreadPriority(pHandle, THREAD_PRIORITY_ABOVE_NORMAL);
 	//SetTimer(ID_TIMER_TEST_PLAY, 1000, NULL);	
	SetTimer(ID_TIMER_SHOW_PROGRESS, 1000, NULL);
	SetTimer(ID_TIMER_SHOW_FREQ, 50, NULL);
	//SetTimer(ID_TIMER_TEST_HYBRID, 5000, NULL);


	CDialog::OnInitDialog();

	m_ctrlSlider.SetRange(0,1000);

	m_ctrlVolume.SetRange(0,100);
	m_ctrlVolume.SetPos(50);
	
	m_ctrlEqBand1.SetRange(0, 100);
	m_ctrlEqBand1.SetPos(50);
	m_ctrlEqBand2.SetRange(0, 100);
	m_ctrlEqBand2.SetPos(50);
	m_ctrlEqBand3.SetRange(0, 100);
	m_ctrlEqBand3.SetPos(50);
	m_ctrlEqBand4.SetRange(0, 100);
	m_ctrlEqBand4.SetPos(50);
	m_ctrlEqBand5.SetRange(0, 100);
	m_ctrlEqBand5.SetPos(50);
	m_ctrlEqBand6.SetRange(0, 100);
	m_ctrlEqBand6.SetPos(50);
	m_ctrlEqBand7.SetRange(0, 100);
	m_ctrlEqBand7.SetPos(50);
	m_ctrlEqBand8.SetRange(0, 100);
	m_ctrlEqBand8.SetPos(50);
	m_ctrlEqBand9.SetRange(0, 100);
	m_ctrlEqBand9.SetPos(50);
	m_ctrlEqBand10.SetRange(0, 100);
	m_ctrlEqBand10.SetPos(50);

	m_ctrlEqPresetFlat.SetCheck(TRUE);
	m_ctrlTrebleBoost.SetRange(0, 1000);
	m_ctrlBassBoost.SetRange(0, 1000);
	m_ctrlVirtualizer.SetRange(0, 1000);

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_rcView.left = 185;
	m_rcView.top = 5;
	m_rcView.right = 185 + 320;
	m_rcView.bottom = 5 + 240;
	CreateWnd();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

CPlayerTestDlg::~CPlayerTestDlg()
{
	//SAFE_DELETE(iAfpTest);
	SAFE_DELETE(iMediaPlayer);
	delete m_WView;

	DeleteCriticalSection(&gCriticalSection);
}

LRESULT CPlayerTestDlg::OnStopMessage(WPARAM w,LPARAM l)
{
//	OnBnClickedStop();
	iMediaPlayer->Pause();
	iMediaPlayer->SetPosition(0);
	iMediaPlayer->Resume();
//	OnBnClickedOk();
	return 0;
}

LRESULT CPlayerTestDlg::OnDownloadMessage(WPARAM w,LPARAM l)
{
	return 0;
}

LRESULT CPlayerTestDlg::OnBufferStartMessage(WPARAM w,LPARAM l)
{
	BeginWaitCursor();
	printf("BeginWaitCursor");
	return 0;
}

LRESULT CPlayerTestDlg::OnBufferEndMessage(WPARAM w,LPARAM l)
{
	EndWaitCursor();
	printf("EndWaitCursor");
	return 0;
}

LRESULT CPlayerTestDlg::OnPreparedMessage(WPARAM w,LPARAM l)
{
	iMediaPlayer->Play();
	EndWaitCursor();
	return 0;
}

void CPlayerTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CPlayerTestDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}

	CClientDC dc(this);//选定当前画图环境
	CPen pen(PS_SOLID,1,0xff0000);//做一支红色粗细为1的笔
	dc.SelectObject(&pen);//将pen放到dc上

	if (pFreq != NULL)
	{
		TTInt nBottom = 150;
		TTInt nLeft = 10;
		for (TTInt n = 0; n < 32; n++)
		{		 
			if (*(pFreq + n) < 0)
			{
				int x = 0;
				x++;
			}
			dc.Rectangle(nLeft, nBottom - (*(pFreq + n)), nLeft + 4, nBottom);
			nLeft += 5;
		}

		pFreq = NULL;
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
//
HCURSOR CPlayerTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CPlayerTestDlg::OnBnClickedOk()
{
	if (iCurPlayIdx >= (int)(iFileArray.size()))
	{
		iCurPlayIdx = 0;
	}	

	if (iCurPlayIdx < 0)
	{
		iCurPlayIdx = iFileArray.size() - 1;
	}

	//wprintf_s(_T("\n+++++Playing %d\n"), iCurPlayIdx);

	//const wchar_t* filename = L"E:\\2test.mp3";
	//const wchar_t* filename = L"E:\\home\\music.mp3";
	const wchar_t* filename = L"E:\\home\\Ocean.mp3";
 	//wchar_t* filename = (wchar_t*)(LPCTSTR)(iFileArray.at(iCurPlayIdx));
// 	
	char *pcstr = (char *)malloc(sizeof(char)*(2 * wcslen(filename)+1));
	memset(pcstr , 0 , 2 * wcslen(filename)+1 );
	wstr2cstr(pcstr,filename,2 * wcslen(filename)+1);

	char *pcstr1 = "http://audio.yinchao.cn/uploadfiles/2016/03/10/201603101147391457581659.mp3";
				//"http://oen.cye.yymommy.com/defc86e3077c73b8/1416819671/mp3_128_6/c1/1f/c1c2e16313e0312988a13fe68ed3111f.mp3?s=t";
		        //"http://oen.cye.yymommy.com/mp3_190_0/dd/fc/dd8668e52d1ac1939ab8dd92551b3efc.mp3?k=382b0c0e5648304e&t=1414226907";
				//	"http://nie.dfe.yymommy.com/mp3_190_2/5d/ca/5dfb15661068e11d4899787a0d14a4ca.mp3?k=24bd7435bcadad36&t=1414226907";
				// "http://oen.cye.yymommy.com/mp3_190_3/a6/15/a69e73f6637068767eadca5a637b2615.mp3?k=fc5d5f9789f2a58b&t=1414226907";
				// "http://nie.dfe.yymommy.com/mp3_190_10/8f/69/8fc819abd050c58675a012a4d1005b69.mp3?k=8af7a38c75a4f83f&t=1414226907";
				// "http://101.44.1.119/mp4files/6113000002209481/mv.hotmusique.com/mv_2_5/2f/e9/2fda4032ad2caa97bc39e7f16a5e85e9.mp4";
				// "http://mv.hotmusique.com/mv_2_5/56/27/566f093d54c197cd79e7e898b579d927.mp4?k=b51c2abe60a376d4&t=1415758216";
				// "http://mv.hotmusique.com/mv_2_5/9a/c4/9a8b64f5a3d7432a1d1e188d4ca62bc4.mp4?k=5338a430f39c74ed&t=1415758216";
				// "http://mv.hotmusique.com/mv_2_5/6c/b5/6c5700df18cf5c4bf225e4636e2198b5.mp4?k=df5a37a6cdae4714&t=1415758216";
				// "http://mv.hotmusique.com/mv_2_5/09/d2/0923a28ae4f0f3725e3ff3110f7a0dd2.mp4?k=97c473aa403cf34d&t=1415945281";
				// "http://mv.hotmusique.com/mv_2_5/0f/62/0fb2faa9f82a73a1ac53a0cf553ad062.mp4?k=08565fd82e4ea69c&t=1415945281";
				// "http://mv.hotmusique.com/mv_2_5/15/e7/15189917c0dabfb00a354deb05397ae7.mp4?k=6f0e23636a230ac7&t=1416017258";
				// "http://mv.hotmusique.com/mv_2_5/b5/0f/b53e1fae3e424c6a95da6295e9bc180f.mp4?k=a3a2bd734c783f53&t=1416017258";
		

	printf("SetDataSource: %s\n", pcstr);

	iMediaPlayer->SetView(m_hView);
	iOpenSync=1;
	if(iOpenSync) 
	{	
		if (TTKErrNone == iMediaPlayer->SetDataSourceSync(pcstr1))
		{	
			//iMediaPlayer->SetPlayRange(3611000, 3703000);
			iMediaPlayer->SetPosition(4510);

			iMediaPlayer->Play();

			printf("Play SetDataSourceSync");		
		}
	}
	else
	{
		if (TTKErrNone == iMediaPlayer->SetDataSourceAsync(pcstr, 1))
		{	
			//iMediaPlayer->SetPlayRange(3611000, 3703000);
			//iMediaPlayer->SetPosition(4510);

			
			BeginWaitCursor();
			printf("Play SetDataSourceAsync");		
		}
	}


	delete pcstr;
	pcstr = NULL;
}

void CPlayerTestDlg::PlayerNotifyEvent(TTNotifyMsg aMsg, TTInt aArg1, TTInt aArg2, const TTChar* aArg3)
{
	//TTASSERT(aError == 0);
	if (aMsg == ENotifyUpdateDuration)
	{
		iDuration = iMediaPlayer->Duration();
		printf("Notify Duration:%dm:%ds:%dms\n", iDuration/1000/60, (iDuration / 1000) % 60, iDuration%1000);
	}
	printf("aError:%d->", aArg1);
	if ((aMsg == ENotifyComplete) && (aArg1 == TTKErrNone))
	{
		EnterCriticalSection(&gCriticalSection);
		iCurPlayIdx++;
		m_Started = false;
		iDuration = 0;	
		printf("Start? :%d\n", m_Started);
		LeaveCriticalSection(&gCriticalSection);

		FlushWnd();

		PostMessage(WM_STOPMESSAGE, 0, 0); 
	}

	if((aMsg == ENotifyCacheCompleted) && (aArg1 == TTKErrNone))
	{
		PostMessage(WM_DOWNLOADMESSAGE, 0, 0); 
	}

 	if (aMsg == ENotifyPlay && aArg1 == TTKErrNone)
 	{
	//	CTTAudioEffectManager::Config(CTTAudioEffectManager::iChannels, CTTAudioEffectManager::iSampleRate);
 		//m_ctrlVolume.SetPos(iMediaPlayer->GetVolume() * 100 / 65535);
		if(iEffectInit == 1) {
			iLevelRange[0]= -1500;
			iLevelRange[1]=1500;
			InitEq();
			InitBassBoost();                                                                                                        
			InitTrebleBoost();
			InitVolume();
			InitVirtualizer();
			InitReverb();
		}
 	}

	if (aMsg == ENotifyPrepare && aArg1 == TTKErrNone) {
		iMediaType = aArg2;
		if(iOpenSync == 0)
		{
			PostMessage(WM_PREPAREDMESSAGE, 0, 0); 	
		}
	}

	if (aMsg == ENotifyBufferingStart && aArg1 == TTKErrNone) {
		PostMessage(WM_BUFFERSTARTMESSAGE, 0, 0); 	
	}

	if (aMsg == ENotifyBufferingDone && aArg1 == TTKErrNone) {
		PostMessage(WM_BUFFERENDMESSAGE, 0, 0); 		
	}
}

void CPlayerTestDlg::OnBnClickedStop()
{
	if (TTKErrNone == iMediaPlayer->Stop())	
	{
		//SAFE_RELEASE(iMediaPlayer);					
		//if (iMediaPlayer == NULL)
		//{
		//	iMediaPlayer = new CTTMediaPlayer(this, "..\\..\\..\\win32libs");
		//}

		EnterCriticalSection(&gCriticalSection);
		m_Started = false;
		LeaveCriticalSection(&gCriticalSection);
	}

	m_ctrlSlider.SetPos(0);

	FlushWnd();
}

void CPlayerTestDlg::OnBnClickedExit()
{
	if (TTKErrNone == iMediaPlayer->Stop(true))	
	{
		EnterCriticalSection(&gCriticalSection);
		m_Started = false;
		LeaveCriticalSection(&gCriticalSection);

		while(iMediaPlayer->GetPlayStatus() != EStatusStoped)
		{
			TTSleep(2);
		}

	

		SAFE_RELEASE(iMediaPlayer);
		OnCancel();
	}
}

void CPlayerTestDlg::OnBnClickedPause()
{
	iMediaPlayer->Pause();
	// TODO: 在此添加控件通知处理程序代码
}

void CPlayerTestDlg::OnBnClickedResume()
{
	iMediaPlayer->Resume();
	// TODO: 在此添加控件通知处理程序代码
}

void CPlayerTestDlg::OnNMReleasedcaptureProgress(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (iDuration == 0)
		iDuration = iMediaPlayer->Duration();
	if (iDuration != 0)
	{
		TTInt64 nPos = iDuration;
		nPos *= m_ctrlSlider.GetPos();
		nPos /= 1000;
		TTInt nPercent = iMediaPlayer->BufferedPercent(nPercent);
		printf("\nSeekToPos:%d\n",nPos);
		iMediaPlayer->SetPosition(nPos);
		printf("CurPos:%d\n",iMediaPlayer->GetPosition());
	}
}

void CPlayerTestDlg::OnTimer(UINT_PTR nIDEvent)
{
	if(nIDEvent == ID_TIMER_TEST_PLAY)
	{
#ifndef __PlayComplete__
// 		TTInt n = 0;
// 		n++;
		if (iMediaPlayer != NULL)
		{
			switch (iMediaPlayer->GetPlayStatus())
			{
			case EStatusStarting:
				break;

			case EStatusPlaying:
				{
					if (iTestStaus == ETestStartPlay)
					{
						iMediaPlayer->Pause();
						iTestStaus = ETestPause;
					}
					else
					{
						if (iTestStaus == ETestResume)
						{
							OnBnClickedStop();
							iTestStaus = ETestReStart;
						}
						else
						{
							TTInt n = 0;
							n++;
						}
					}
				}
				break;

			case EStatusPaused:
				{
					ASSERT(iTestStaus == ETestPause);
					iMediaPlayer->Resume();
					iTestStaus = ETestResume;
				}
				break;

			case EStatusStoped:
				{
					ASSERT(iTestStaus == ETestReStart);

#ifdef __TEST_DELETEMEDIAPLAY__				
					SAFE_RELEASE(iMediaPlayer);					
					iMediaPlayer = new CTTMediaPlayer(this);					
#endif	
					OnBnClickedOk();
					iTestStaus = ETestStartPlay;
				}
				break;

// 			case EStatusPlayComplete:
// 				break;

			default:
				ASSERT(false);				
			}	
		}
#else
		if (!m_Started)
		{
#ifdef __TEST_DELETEMEDIAPLAY__
			SAFE_RELEASE(iMediaPlayer);					
#endif
			if (iMediaPlayer == NULL)
			{
				iMediaPlayer = new CTTMediaPlayer(this, "..\\..\\..\\win32libs");
			}
			
			OnBnClickedOk();
		}

#endif
	}
	else if (nIDEvent == ID_TIMER_SHOW_PROGRESS)
	{
		if (EStatusPlaying == iMediaPlayer->GetPlayStatus())
		{
			if (iDuration == 0)
			{
				iDuration = iMediaPlayer->Duration();
			}

			if (iDuration != 0)
			{
				TTUint nCurPos = iMediaPlayer->GetPosition();

				if (mPrePos > nCurPos)
				{
					printf("nCurPos:%d, nPrePos:%d\n",nCurPos, mPrePos);
				}
				mPrePos = nCurPos;
				TTInt64 nTemp = nCurPos;
				nTemp *= 1000;				
				m_ctrlSlider.SetPos(nTemp / iDuration);
			

				// 			if (nCurPos > (iDuration * 2 / 3))
				// 			{
				// 				iMediaPlayer->SetPosition(iDuration / 3);
				// 			}
			}
			else
			{
				m_ctrlSlider.SetPos(0);
			}
		}
	}
	else if (nIDEvent == ID_TIMER_SHOW_FREQ)
	{
		if(!(iMediaType & 2) && (iMediaType & 1)) {	
			TTInt16 freq[512];
			TTInt16 wave[512 * 2];
			if (TTKErrNone == iMediaPlayer->GetCurrentFreqAndWave(freq, wave, 512))
			{
				if (0 == NormalizeFreqBin(m_WView->mFreq, 32, freq, 512))
				{
					m_WView->DrawRect();
				}
			}
		}
	}
	else if (nIDEvent == ID_TIMER_TEST_HYBRID)
	{
		if (iTestCycleDelay == 0)
		{
			switch (iTestStaus2)
			{
			case ETestStatus2Next:
				{
					OnBnClickedNext();
					iTestStaus2 = ETestStatus2PauseResumePaused;
				}
				break;

			case ETestStatus2PauseResume:
				{
					printf("ToPaused\n");
					iMediaPlayer->Pause();
					iTestStaus2 = ETestStatus2PauseResumePaused;
				}
				break;

			case ETestStatus2PauseResumePaused:
				{
					printf("ToPlay\n");
					iMediaPlayer->Resume();
					iTestStaus2 = ETestStatus2Seek;				
				}
				break;

			case ETestStatus2Seek:
				{
					printf("ToSeek\n");
					srand((unsigned)time(NULL));
					TTUint nPos = rand()%(300000);
					printf("ToSeek:%d\n", nPos);
					iMediaPlayer->SetPosition(nPos);
					iTestStaus2 = ETestStatus2PauseSeekResume;
				}
				break;
			case ETestStatus2PauseSeekResume:
				{
					printf("ToPause\n");
					iMediaPlayer->Pause();
					iTestStaus2 = ETestStatus2PauseSeekResumePaused;
				}
				break;

			case ETestStatus2PauseSeekResumePaused:
				{
					srand((unsigned)time(NULL));
					TTUint nPos = TTUint(rand()%(300000));
					printf("ToSeek:%d\n", nPos);
					iMediaPlayer->SetPosition(nPos);
					iTestStaus2 = ETestStatus2PauseSeekResumeSeeked;
				}
				break;

			case ETestStatus2PauseSeekResumeSeeked:
				{
					printf("ToPlay\n");
					iMediaPlayer->Resume();
					iTestStaus2 = ETestStatus2Next;	
				}
				break;
			}

			srand((unsigned)time(NULL));
			iTestCycleDelay = TTUint(rand()%10);
		}
		else
		{
			iTestCycleDelay--;
		}
	}
}

void CPlayerTestDlg::OnNMReleasedcaptureSlider(NMHDR *pNMHDR, LRESULT *pResult)
{
	int nPos = m_ctrlVolume.GetPos();
	int volume = (100 - nPos) * 65535 / 100;
	iMediaPlayer->SetVolume(volume, volume);
}

void CPlayerTestDlg::TravelFolder(CString strDir, int nDepth)
{
	CFileFind filefind;                                         //声明CFileFind类型变量
	CString strWildpath = strDir + _T("\\*.*");     //所有文件都列出。
	if(filefind.FindFile(strWildpath, 0))	                   //开始检索文件
	{
		BOOL bRet = TRUE;
		while(bRet)
		{
			bRet = filefind.FindNextFile();                 //枚举一个文件
			if(filefind.IsDots())                                 //如果是. 或 .. 做下一个
				continue;
			for (int i = 0; i < nDepth; i ++)                 //层次空格打印
			{
				//TRACE(_T("    "));
			}
			if(!filefind.IsDirectory())                          //不是子目录，把文件名打印出来
			{
				CString strTextOut = strDir + CString(_T("\\")) + filefind.GetFileName();
				//TRACE(_T("file = %s\r\n"), strTextOut);
				iFileArray.push_back(strTextOut);
			}
			else                                                    //如果是子目录，递归调用该函数
			{
				CString strTextOut = strDir + CString(_T("\\")) + filefind.GetFileName();
				//TRACE(_T("dir = %s\r\n"), strTextOut);
				TravelFolder(strTextOut, nDepth + 1);//递归调用该函数打印子目录里的文件
			}
		}
		printf("FileNum :%d\n", iFileArray.size());
		filefind.Close();
	}
}

void CPlayerTestDlg::OnBnClickedPre()
{
	iDuration = 0;
	iCurPlayIdx--;	
	OnBnClickedOk();
	// TODO: 在此添加控件通知处理程序代码
}

void CPlayerTestDlg::OnBnClickedNext()
{
	iDuration = 0;
	iCurPlayIdx++;
	OnBnClickedOk();
	// TODO: 在此添加控件通知处理程序代码
}

void CPlayerTestDlg::OnBnClickedRadioEqPop()
{

}

void CPlayerTestDlg::OnBnClickedRadioEqFlat()
{

}

void CPlayerTestDlg::OnBnClickedRadioEqMetal()
{

}



void CPlayerTestDlg::OnBnClickedRadioEqRock()
{

}

void CPlayerTestDlg::OnBnClickedRadioEqJazz()
{

}

void CPlayerTestDlg::DisableEqPresetCtrls()
{
	m_ctrlEqPresetPop.SetCheck(FALSE);
	m_ctrlEqPresetFlat.SetCheck(FALSE);
	m_ctrlEqPresetMetal.SetCheck(FALSE);
	m_ctrlEqPresetRock.SetCheck(FALSE);
	m_ctrlEqPresetJazz.SetCheck(FALSE);
}

void CPlayerTestDlg::InitVolume()
{

}

void CPlayerTestDlg::InitEq()
{

}

void CPlayerTestDlg::SetEqBandLevel(TTInt16 aBand, TTInt aPos)
{

}

void CPlayerTestDlg::InitBassBoost()
{

}

void CPlayerTestDlg::InitTrebleBoost()
{

}

void CPlayerTestDlg::InitVirtualizer()
{

}

void CPlayerTestDlg::InitReverb()
{
	

}

void CPlayerTestDlg::OnNMReleasedcaptureSlider2(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	SetEqBandLevel(0, m_ctrlEqBand1.GetPos());
}

void CPlayerTestDlg::OnNMReleasedcaptureSlider3(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	SetEqBandLevel(1, m_ctrlEqBand2.GetPos());
}

void CPlayerTestDlg::OnNMReleasedcaptureSlider4(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	SetEqBandLevel(2, m_ctrlEqBand3.GetPos());
}

void CPlayerTestDlg::OnNMReleasedcaptureSlider5(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	SetEqBandLevel(3, m_ctrlEqBand4.GetPos());
}

void CPlayerTestDlg::OnNMReleasedcaptureSlider6(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	SetEqBandLevel(4, m_ctrlEqBand5.GetPos());
}


void CPlayerTestDlg::OnNMReleasedcaptureSlider10(NMHDR *pNMHDR, LRESULT *pResult)
{
	SetEqBandLevel(5, m_ctrlEqBand6.GetPos());
	*pResult = 0;
}

void CPlayerTestDlg::OnNMReleasedcaptureSlider11(NMHDR *pNMHDR, LRESULT *pResult)
{
	SetEqBandLevel(6, m_ctrlEqBand7.GetPos());
	*pResult = 0;
}

void CPlayerTestDlg::OnNMReleasedcaptureSlider12(NMHDR *pNMHDR, LRESULT *pResult)
{
	SetEqBandLevel(7, m_ctrlEqBand8.GetPos());
	*pResult = 0;
}

void CPlayerTestDlg::OnNMReleasedcaptureSlider13(NMHDR *pNMHDR, LRESULT *pResult)
{
	SetEqBandLevel(8, m_ctrlEqBand9.GetPos());
	*pResult = 0;
}

void CPlayerTestDlg::OnNMReleasedcaptureSlider14(NMHDR *pNMHDR, LRESULT *pResult)
{
	SetEqBandLevel(9, m_ctrlEqBand10.GetPos());
	*pResult = 0;
}


void CPlayerTestDlg::OnBnClickedCheckBassBoost()
{

}

void CPlayerTestDlg::OnNMReleasedcaptureBassBoost(NMHDR *pNMHDR, LRESULT *pResult)
{

}

void CPlayerTestDlg::OnBnClickedCheckVirtualizer()
{

}

void CPlayerTestDlg::OnNMReleasedcaptureVirtualzer(NMHDR *pNMHDR, LRESULT *pResult)
{

}

bool  CPlayerTestDlg::CreateWnd ()
{
	m_WView = new CVideoView();
	m_WView->Create(IDD_VIDEOVIEW, this);

	m_WView->MoveWindow(&m_rcView,false);
	m_WView->ShowWindow(SW_SHOW);

	m_hView = m_WView->GetSafeHwnd();

	return true;
}

void  CPlayerTestDlg::FlushWnd ()
{
	PAINTSTRUCT ps;
	CDC * hdc = m_WView->BeginPaint(&ps);

	RECT rc;
	m_WView->GetClientRect(&rc);

	HBRUSH hBrush = ::CreateSolidBrush(RGB(0,0,0));
	FillRect(m_WView->GetDC()->GetSafeHdc(), &rc, hBrush);
	DeleteObject(hBrush);

	m_WView->EndPaint(&ps);
}

void CPlayerTestDlg::OnBnClickedCheckReverb()
{

}

void CPlayerTestDlg::OnBnClickedRadioReverbSmallRoom()
{

}

void CPlayerTestDlg::OnBnClickedRadioReverbMediumRoom()
{

}

void CPlayerTestDlg::OnBnClickedRadioReverbLargeRoom()
{

}

void CPlayerTestDlg::OnBnClickedRadioReverbMediumHall()
{

}

void CPlayerTestDlg::OnBnClickedRadioReverbLargeHall()
{

}

void CPlayerTestDlg::OnBnClickedRadioReverbPlate()
{


}

void CPlayerTestDlg::OnBnClickedCheckTreble()
{


}

void CPlayerTestDlg::OnNMReleasedcaptureSldTreble(NMHDR *pNMHDR, LRESULT *pResult)
{

}

void CPlayerTestDlg::OnBnClickedBtnEqReset()
{
	m_ctrlEqBand1.SetPos(50);
	m_ctrlEqBand2.SetPos(50);
	m_ctrlEqBand3.SetPos(50);
	m_ctrlEqBand4.SetPos(50);
	m_ctrlEqBand5.SetPos(50);
	m_ctrlEqBand6.SetPos(50);
	m_ctrlEqBand7.SetPos(50);
	m_ctrlEqBand8.SetPos(50);
	m_ctrlEqBand9.SetPos(50);
	m_ctrlEqBand10.SetPos(50);

	SetEqBandLevel(0, m_ctrlEqBand1.GetPos());
	SetEqBandLevel(1, m_ctrlEqBand2.GetPos());
	SetEqBandLevel(2, m_ctrlEqBand3.GetPos());
	SetEqBandLevel(3, m_ctrlEqBand4.GetPos());
	SetEqBandLevel(4, m_ctrlEqBand5.GetPos());
	SetEqBandLevel(5, m_ctrlEqBand6.GetPos());
	SetEqBandLevel(6, m_ctrlEqBand7.GetPos());
	SetEqBandLevel(7, m_ctrlEqBand8.GetPos());
	SetEqBandLevel(8, m_ctrlEqBand9.GetPos());
	SetEqBandLevel(9, m_ctrlEqBand10.GetPos());
}
