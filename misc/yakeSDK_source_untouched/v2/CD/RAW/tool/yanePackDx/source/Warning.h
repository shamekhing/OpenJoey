#if !defined(AFX_WARNING_H__0E9BA302_A328_4ABF_BBCC_E9684233C69D__INCLUDED_)
#define AFX_WARNING_H__0E9BA302_A328_4ABF_BBCC_E9684233C69D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Warning.h : ヘッダー ファイル
//
#include "StdAfx.h"
#include "Resource.h"

/////////////////////////////////////////////////////////////////////////////
// Warning ダイアログ

class Warning : public CDialog
{
// コンストラクション
public:
	Warning(CWnd* pParent = NULL);   // 標準のコンストラクタ
    BOOL CheckCancelButton();

// ダイアログ データ
	//{{AFX_DATA(Warning)
	enum { IDD = IDD_DIALOG2 };
		// メモ: ClassWizard はこの位置にデータ メンバを追加します。
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(Warning)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
    BOOL m_bCancel;

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(Warning)
	virtual void OnCancel();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_WARNING_H__0E9BA302_A328_4ABF_BBCC_E9684233C69D__INCLUDED_)
