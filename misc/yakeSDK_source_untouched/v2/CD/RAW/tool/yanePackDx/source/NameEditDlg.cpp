// NameEditDlg.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "yanepack.h"
#include "NameEditDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// NameEditDlg ダイアログ


NameEditDlg::NameEditDlg(CWnd* pParent /*=NULL*/)
	: CDialog(NameEditDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(NameEditDlg)
	m_NewFileName = _T("");
	//}}AFX_DATA_INIT

	m_bCancel = false;
}


void NameEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(NameEditDlg)
	DDX_Text(pDX, IDC_EDIT1, m_NewFileName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(NameEditDlg, CDialog)
	//{{AFX_MSG_MAP(NameEditDlg)
	ON_EN_CHANGE(IDC_EDIT1, OnChangeEdit1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// NameEditDlg メッセージ ハンドラ

void NameEditDlg::OnCancel() 
{
	// TODO: この位置に特別な後処理を追加してください。
	m_bCancel = true;
	CDialog::OnCancel();
}

void NameEditDlg::OnOK() 
{
	// TODO: この位置にその他の検証用のコードを追加してください
	m_bCancel = false;
	CDialog::OnOK();
}

void NameEditDlg::OnChangeEdit1() 
{
	// TODO: これが RICHEDIT コントロールの場合、コントロールは、 lParam マスク
	// 内での論理和の ENM_CHANGE フラグ付きで CRichEditCrtl().SetEventMask()
	// メッセージをコントロールへ送るために CDialog::OnInitDialog() 関数をオーバー
	// ライドしない限りこの通知を送りません。
	
	// TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
	
}


BOOL NameEditDlg::CheckCancelButton()
{
	return m_bCancel;
}
