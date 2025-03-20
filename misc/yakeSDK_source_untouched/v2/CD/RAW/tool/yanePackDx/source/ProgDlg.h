// ProgDlg.h : ヘッダ ファイル
// CG: このファイルは「プログレス ダイアログ」コンポーネントにより追加されています。

/////////////////////////////////////////////////////////////////////////////
// CProgressDlg ダイアログ

#ifndef __PROGDLG_H__
#define __PROGDLG_H__
#include "StdAfx.h"
#include "Resource.h"

class CProgressDlg : public CDialog
{
// 構築/消滅
public:
    CProgressDlg(UINT nCaptionID = 0);   // 標準コンストラクタ
    ~CProgressDlg();

    BOOL Create(CWnd *pParent=NULL);

    // キャンセル ボタンのチェック
    BOOL CheckCancelButton();
    // プログレス ダイアログの処理
    void SetStatus(LPCTSTR lpszMessage);
    void SetRange(int nLower,int nUpper);
    int  SetStep(int nStep);
    int  SetPos(int nPos);
    int  OffsetPos(int nPos);
    int  StepIt();
        
// ダイアログ データ
    //{{AFX_DATA(CProgressDlg)
    enum { IDD = CG_IDD_PROGRESS };
    CProgressCtrl	m_Progress;
    //}}AFX_DATA

// オーバーライド
    // ClassWizard は仮想関数のオーバライドを生成します。
    //{{AFX_VIRTUAL(CProgressDlg)
    public:
    virtual BOOL DestroyWindow();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
    //}}AFX_VIRTUAL

// インプリメンテーション
protected:
	UINT m_nCaptionID;
    int m_nLower;
    int m_nUpper;
    int m_nStep;
    
    BOOL m_bCancel;
    BOOL m_bParentDisabled;

    void ReEnableParent();

    virtual void OnCancel();
    virtual void OnOK() {}; 
    void UpdatePercent(int nCurrent);
    void PumpMessages();

    // 生成されたメッセージ マップ関数
    //{{AFX_MSG(CProgressDlg)
    virtual BOOL OnInitDialog();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

#endif // __PROGDLG_H__
