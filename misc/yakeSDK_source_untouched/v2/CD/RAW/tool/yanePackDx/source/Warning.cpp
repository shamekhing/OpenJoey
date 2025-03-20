// Warning.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "yanepack.h"
#include "Warning.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Warning ダイアログ


Warning::Warning(CWnd* pParent /*=NULL*/)
	: CDialog(Warning::IDD, pParent)
{
	//{{AFX_DATA_INIT(Warning)
		// メモ - ClassWizard はこの位置にマッピング用のマクロを追加または削除します。
	//}}AFX_DATA_INIT
}


void Warning::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(Warning)
		// メモ - ClassWizard はこの位置にマッピング用のマクロを追加または削除します。
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(Warning, CDialog)
	//{{AFX_MSG_MAP(Warning)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Warning メッセージ ハンドラ

void Warning::OnCancel() 
{
	// TODO: この位置に特別な後処理を追加してください。
	m_bCancel = true;
	CDialog::OnCancel();
}

void Warning::OnOK() 
{
	// TODO: この位置にその他の検証用のコードを追加してください
	m_bCancel = false;
	CDialog::OnOK();
}

BOOL Warning::CheckCancelButton()
{
	return m_bCancel;
}
