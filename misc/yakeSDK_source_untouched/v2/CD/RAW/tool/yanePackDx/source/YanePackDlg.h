// YanePackDlg.h : ヘッダー ファイル
//

#if !defined(AFX_YANEPACKDLG_H__B504411E_6512_11D3_980F_0000E86A3B2F__INCLUDED_)
#define AFX_YANEPACKDLG_H__B504411E_6512_11D3_980F_0000E86A3B2F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CYanePackDlg ダイアログ

class CYanePackDlg : public CDialog
{
// 構築
public:
	CYanePackDlg(CWnd* pParent = NULL);	// 標準のコンストラクタ

// ダイアログ データ
	//{{AFX_DATA(CYanePackDlg)
	enum { IDD = IDD_YANEPACK_DIALOG };
	CButton	m_FNameList;
	//}}AFX_DATA

	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CYanePackDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV のサポート
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:
	HICON m_hIcon;

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CYanePackDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButton1();
	afx_msg void OnButton2();
	afx_msg void OnButton3();
	afx_msg void OnButton4();
	afx_msg void OnButton5();
	afx_msg void OnButton6();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_YANEPACKDLG_H__B504411E_6512_11D3_980F_0000E86A3B2F__INCLUDED_)
