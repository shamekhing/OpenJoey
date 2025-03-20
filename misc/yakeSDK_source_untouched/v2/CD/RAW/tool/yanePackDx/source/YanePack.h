// YanePack.h : YANEPACK アプリケーションのメイン ヘッダー ファイルです。
//

#if !defined(AFX_YANEPACK_H__B504411C_6512_11D3_980F_0000E86A3B2F__INCLUDED_)
#define AFX_YANEPACK_H__B504411C_6512_11D3_980F_0000E86A3B2F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// メイン シンボル

/////////////////////////////////////////////////////////////////////////////
// CYanePackApp:
// このクラスの動作の定義に関しては YanePack.cpp ファイルを参照してください。
//

class CYanePackApp : public CWinApp
{
public:
	CYanePackApp();

// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CYanePackApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// インプリメンテーション

	//{{AFX_MSG(CYanePackApp)
		// メモ - ClassWizard はこの位置にメンバ関数を追加または削除します。
		//        この位置に生成されるコードを編集しないでください。
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_YANEPACK_H__B504411C_6512_11D3_980F_0000E86A3B2F__INCLUDED_)
