// YanePackDlg.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "YanePack.h"
#include "YanePackDlg.h"
#include "packfile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CYanePackDlg ダイアログ

CYanePackDlg::CYanePackDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CYanePackDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CYanePackDlg)
		// メモ: この位置に ClassWizard によってメンバの初期化が追加されます。
	//}}AFX_DATA_INIT
	// メモ: LoadIcon は Win32 の DestroyIcon のサブシーケンスを要求しません。
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CYanePackDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CYanePackDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CYanePackDlg, CDialog)
	//{{AFX_MSG_MAP(CYanePackDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_BN_CLICKED(IDC_BUTTON2, OnButton2)
	ON_BN_CLICKED(IDC_BUTTON3, OnButton3)
	ON_BN_CLICKED(IDC_BUTTON4, OnButton4)
	ON_BN_CLICKED(IDC_BUTTON5, OnButton5)
	ON_BN_CLICKED(IDC_BUTTON6, OnButton6)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CYanePackDlg メッセージ ハンドラ

BOOL CYanePackDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// このダイアログ用のアイコンを設定します。フレームワークはアプリケーションのメイン
	// ウィンドウがダイアログでない時は自動的に設定しません。
	SetIcon(m_hIcon, TRUE);			// 大きいアイコンを設定
	SetIcon(m_hIcon, FALSE);		// 小さいアイコンを設定
	
	// TODO: 特別な初期化を行う時はこの場所に追加してください。
	
	return TRUE;  // TRUE を返すとコントロールに設定したフォーカスは失われません。
}

// もしダイアログボックスに最小化ボタンを追加するならば、アイコンを描画する
// コードを以下に記述する必要があります。MFC アプリケーションは document/view
// モデルを使っているので、この処理はフレームワークにより自動的に処理されます。

void CYanePackDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 描画用のデバイス コンテキスト

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// クライアントの矩形領域内の中央
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// アイコンを描画します。
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// システムは、ユーザーが最小化ウィンドウをドラッグしている間、
// カーソルを表示するためにここを呼び出します。
HCURSOR CYanePackDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CYanePackDlg::OnButton1() 
{
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
	CPackFile().Pack();
}

void CYanePackDlg::OnButton2() 
{
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
	CPackFile().Unpack();
}

void CYanePackDlg::OnButton3() 
{
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
	CPackFile().FolderPack();
}


// 02'04'13
void CYanePackDlg::OnButton4()
{
	CPackFile().PurePack();
}

void CYanePackDlg::OnButton5()
{
	CPackFile().FolderPurePack();
}

void CYanePackDlg::OnButton6()
{
	CPackFile().PackOne();
}

