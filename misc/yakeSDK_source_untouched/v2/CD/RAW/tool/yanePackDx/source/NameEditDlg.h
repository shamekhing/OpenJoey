#if !defined(AFX_NAMEEDITDLG_H__7EDF4F83_DDE2_4790_A2DD_FF3A1AA3D68F__INCLUDED_)
#define AFX_NAMEEDITDLG_H__7EDF4F83_DDE2_4790_A2DD_FF3A1AA3D68F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NameEditDlg.h : ヘッダー ファイル
//
#include "StdAfx.h"
#include "Resource.h"

/////////////////////////////////////////////////////////////////////////////
// NameEditDlg ダイアログ

class NameEditDlg : public CDialog
{
// コンストラクション
public:
	NameEditDlg(CWnd* pParent = NULL);   // 標準のコンストラクタ

    BOOL CheckCancelButton();
	CString NewFileName(){ return m_NewFileName; }

// ダイアログ データ
	//{{AFX_DATA(NameEditDlg)
	enum { IDD = IDD_DIALOG1 };
	CString	m_NewFileName;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(NameEditDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:

    BOOL m_bCancel;

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(NameEditDlg)
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnChangeEdit1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_NAMEEDITDLG_H__7EDF4F83_DDE2_4790_A2DD_FF3A1AA3D68F__INCLUDED_)
