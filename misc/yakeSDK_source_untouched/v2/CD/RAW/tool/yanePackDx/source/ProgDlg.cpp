//  ProgDlg.cpp : インプリメンテーション ファイル
// CG: このファイルは「プログレス ダイアログ」コンポーネントにより追加されています。

#include "stdafx.h"
#include "resource.h"
#include "ProgDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProgressDlg ダイアログ

CProgressDlg::CProgressDlg(UINT nCaptionID)
{
	m_nCaptionID = CG_IDS_PROGRESS_CAPTION;
	if (nCaptionID != 0)
		m_nCaptionID = nCaptionID;

    m_bCancel=FALSE;
    m_nLower=0;
    m_nUpper=100;
    m_nStep=10;
    //{{AFX_DATA_INIT(CProgressDlg)
    // メモ: ClassWizard は、この位置にメンバの初期化コードを追加します。
    //}}AFX_DATA_INIT
    m_bParentDisabled = FALSE;
}

CProgressDlg::~CProgressDlg()
{
    if(m_hWnd!=NULL)
      DestroyWindow();
}

BOOL CProgressDlg::DestroyWindow()
{
    ReEnableParent();
    return CDialog::DestroyWindow();
}

void CProgressDlg::ReEnableParent()
{
    if(m_bParentDisabled && (m_pParentWnd!=NULL))
      m_pParentWnd->EnableWindow(TRUE);
    m_bParentDisabled=FALSE;
}

BOOL CProgressDlg::Create(CWnd *pParent)
{
    // ダイアログの実際の親ウィンドウを取得します。
    m_pParentWnd = CWnd::GetSafeOwner(pParent);

    // m_bParentDisabled は、このダイアログが破棄された時に、親ウィンドウを
    // 再び有効にするために使用します。従って、この時点で親ウィンドウがすでに
    // 有効な場合のみ、この変数に TRUE を設定します。

    if((m_pParentWnd!=NULL) && m_pParentWnd->IsWindowEnabled())
    {
      m_pParentWnd->EnableWindow(FALSE);
      m_bParentDisabled = TRUE;
    }

    if(!CDialog::Create(CProgressDlg::IDD,pParent))
    {
      ReEnableParent();
      return FALSE;
    }

    return TRUE;
}

void CProgressDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CProgressDlg)
    DDX_Control(pDX, CG_IDC_PROGDLG_PROGRESS, m_Progress);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CProgressDlg, CDialog)
    //{{AFX_MSG_MAP(CProgressDlg)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CProgressDlg::SetStatus(LPCTSTR lpszMessage)
{
    ASSERT(m_hWnd); // ダイアログが作成される以前に、この関数を呼び出しては
                    // いけません。OnInitDialog からの呼び出しは可能です。
    CWnd *pWndStatus = GetDlgItem(CG_IDC_PROGDLG_STATUS);

    // スタティック テキスト コントロールの存在を確認します。
    ASSERT(pWndStatus!=NULL);
    pWndStatus->SetWindowText(lpszMessage);
}

void CProgressDlg::OnCancel()
{
    m_bCancel=TRUE;
}

void CProgressDlg::SetRange(int nLower,int nUpper)
{
    m_nLower = nLower;
    m_nUpper = nUpper;
    m_Progress.SetRange(nLower,nUpper);
}
  
int CProgressDlg::SetPos(int nPos)
{
    PumpMessages();
    int iResult = m_Progress.SetPos(nPos);
    UpdatePercent(nPos);
    return iResult;
}

int CProgressDlg::SetStep(int nStep)
{
    m_nStep = nStep; // 後でパーセンテージの計算を行う際に利用します。
    return m_Progress.SetStep(nStep);
}

int CProgressDlg::OffsetPos(int nPos)
{
    PumpMessages();
    int iResult = m_Progress.OffsetPos(nPos);
    UpdatePercent(iResult+nPos);
    return iResult;
}

int CProgressDlg::StepIt()
{
    PumpMessages();
    int iResult = m_Progress.StepIt();
    UpdatePercent(iResult+m_nStep);
    return iResult;
}

void CProgressDlg::PumpMessages()
{
    // ダイアログを使用する前に Create() を呼んでください。
    ASSERT(m_hWnd!=NULL);

    MSG msg;
    // ダイアログ メッセージの処理
    while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
      if(!IsDialogMessage(&msg))
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);  
      }
    }
}

BOOL CProgressDlg::CheckCancelButton()
{
    // ペンディング中のメッセージの処理
    PumpMessages();

    // m_bCancel を FALSE に戻すことにより
    // ユーザがキャンセル ボタンを押すまで、CheckCancelButton は
    // FALSE を返します。このため、CheckCancelButton を呼び出した
    // 後にオペレーションを継続することが可能になります。
    // もし m_bCancel が TRUE に設定されたままだと、次回以降の
    // CheckCancelButton の呼び出しは常に TRUE を返してしまいます。

    BOOL bResult = m_bCancel;
    m_bCancel = FALSE;

    return bResult;
}

void CProgressDlg::UpdatePercent(int nNewPos)
{
    CWnd *pWndPercent = GetDlgItem(CG_IDC_PROGDLG_PERCENT);
    int nPercent;
    
    int nDivisor = m_nUpper - m_nLower;
    ASSERT(nDivisor>0);  // m_nLower は m_nUpper 未満でなければいけません。

    int nDividend = (nNewPos - m_nLower);
    ASSERT(nDividend>=0);   // 現在位置は m_nLower 以上でなければいけません。

    nPercent = nDividend * 100 / nDivisor;

    // プログレス コントロールは折り返し可能なので、それに応じてパーセンテージも
    // 折り返すようにします。ただし、剰余を取る際、100% を 0% としてはいけません。
    if(nPercent!=100)
      nPercent %= 100;

    // パーセンテージを表示します。
    CString strBuf;
    strBuf.Format(_T("%d%c"),nPercent,_T('%'));

	CString strCur; // 現在のパーセンテージを取得
    pWndPercent->GetWindowText(strCur);

	if (strCur != strBuf)
		pWndPercent->SetWindowText(strBuf);
}
    
/////////////////////////////////////////////////////////////////////////////
// CProgressDlg メッセージ ハンドラ

BOOL CProgressDlg::OnInitDialog() 
{
    CDialog::OnInitDialog();
    m_Progress.SetRange(m_nLower,m_nUpper);
    m_Progress.SetStep(m_nStep);
    m_Progress.SetPos(m_nLower);

	CString strCaption;
	VERIFY(strCaption.LoadString(m_nCaptionID));
    SetWindowText(strCaption);

    return TRUE;  
}
