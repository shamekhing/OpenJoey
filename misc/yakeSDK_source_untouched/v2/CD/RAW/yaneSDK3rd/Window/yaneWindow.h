// yaneWindow.h
//	window base class
//

#ifndef __yaneWindow_h__
#define __yaneWindow_h__

#include "yaneWinHook.h"

class CWindowOption {
/**
	class CWindow で指定するためのウィンドゥオプション
*/
public:
	string	caption;	///	キャプション
	string	classname;	///	クラス名(captionと同じでも良い)

	int		size_x;		///	横方向のサイズ
	int		size_y;		///	縦方向のサイズ
	LONG	style;		///	ウィンドゥスタイルの追加指定	

	bool	bCentering;	///	ウィンドゥは画面全体に対してセンタリングして表示か

	CWindowOption() {
		/**
			ディフォルトでこれ。必要があれば書き換えるべし
			caption = "あぷりちゃん";
			classname = "YANEAPPLICATION";
			style = WS_VISIBLE | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU;
			size_x = 640; size_y = 480;
			bCentering = true;
		*/
		caption = "あぷりちゃん";
		classname = "YANEAPPLICATION";
		style = WS_VISIBLE | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU;
		size_x = 640; size_y = 480;
		bCentering = true;
	}
};

class IWindow {
public:
	virtual LRESULT		Create(CWindowOption& opt,HWND lpParent=NULL)=0;
	virtual HWND		GetHWnd()const=0;
	virtual LRESULT		SetWindowPos(int x,int y)=0;
	virtual void		ChangeWindowStyle()=0;
	virtual LRESULT		Resize(int sx,int sy)=0;
	virtual void		SetSize(int sx,int sy)=0;
	virtual void		UseMouseLayer(bool bUse)=0;
	virtual void		ShowCursor(bool bShow)=0;
	virtual bool		IsShowCursor()=0;
	virtual CWindowOption* GetWindowOption()=0;
	volatile virtual bool		IsMinimized()=0;
	virtual CWinHookList* GetHookList()=0;
	virtual void		ClearAllHook()=0;

	virtual ~IWindow(){}
};

class CWindow : public IWinHook , public IWindow{
/**
	窓を生成するクラスです。このクラスのインスタンス一つが、
	窓一つに対応します。マルチウィンドゥのサポートはやや甘いです。
	（どうせフルスクリーンになるとDirectDrawを用いて描画している以上、
	どうしようも無いという話もある…）

	☆　描画のために

	class CFastDrawを使った描画を行なう場合、
	そちらも参照してください。CFastDrawを使っているときに
	画面サイズを変更する場合、CFastDraw::ChangeDisplayModeを
	呼び出してください。

*/
public:
	/// 窓を作る。親ウィンドウが存在するのならば最後のパラメータで指定すること
	LRESULT		Create(CWindowOption& opt,HWND lpParent=NULL);

	HWND		GetHWnd()const { return m_hWnd; }	///	HWNDを返す

	LRESULT		SetWindowPos(int x,int y);			///	ウィンドゥを移動

	void		ChangeWindowStyle();
	///	現在のスクリーンモード（フルスクリーン or ウィンドゥモード）用に
	///	WindowStyleを変更する

	LRESULT		Resize(int sx,int sy);				///	窓のリサイズ

	static	void	ChangeScreen(bool bFullScr) { g_bFullScreen = bFullScr; }
	/**
		フルスクリーン⇔ウィンドゥモードの切り替えが発生した場合、
		この関数を呼び出したあとそれぞれのウィンドゥに関して
		ChangeWindowStyleを呼び出すこと
		ただし、フルスクリーン時のマルチウィンドゥはDirectDrawを使う関係上、
		非サポート
	*/

	void		SetSize(int sx,int sy);
	/**
		窓のリサイズ（設定のみで実際に変更はしない）
		なぜこんなものが必要になるかというと、Resize⇒ChangeWindowStyleと
		実行すると２回ウィンドゥサイズの変更を行なうことになって、
		そのモーションが見えて困るから。
		SetSize　⇒　ChangeWindowStyleならば安全。
	*/

	///		MouseLayer
	void	UseMouseLayer(bool bUse);
	///	ソフトウェアカーソルのためにカーソルを消す
	void	ShowCursor(bool bShow);
	///	ハードウェアマウスカーソルの表示／非表示
	bool	IsShowCursor();
	///	ハードウェアマウスカーソルの表示／非表示を取得

	///		Property
	DWORD		m_dwFillColor;		///	背景色

	CWindowOption* GetWindowOption() { return &m_opt; }
	///	Windowオプションの取得。ウィンドゥ生成前ならば書き換えても良い。

	volatile bool	IsMinimized() { return m_bMinimized; }
	///	最小化されているか？

	///	メッセージをフックするためのポインタリスト
	///	（class IWinHook のベクタ）
	CWinHookList* GetHookList() { return &m_HookPtrList; }

	///	メッセージをフックするためのポインタリストをクリアする
	void	ClearAllHook() { m_HookPtrList.Clear(); }

	CWindow();
	virtual ~CWindow();

	/**
		 メニューの存在をチェックし、メニューがあるのならば
		内部フラグ(m_bUseMenu)を更新する。
		動的にメニューを取り外ししたときに、このクラスに
		それを反映させるのに使う
	*/
	void CheckMenu() {
		if(m_hWnd!=NULL&&::GetMenu(m_hWnd)!=NULL){
			m_bUseMenu = true;
		}else{
			m_bUseMenu = true;
		}
	}

	///		その他
	static void	GetScreenSize(int &x,int &y);
	///	現在の画面全体のサイズの取得

	static bool	IsFullScreen() { return g_bFullScreen; }
	///	フルスクリーンか？

protected:
	HWND		m_hWnd;				//	ウィンドゥハンドル
	bool		m_bFullScreen;		//	現在どちらのモードに合わせて窓を作っているのか？
	CWindowOption	m_opt;			//	ウィンドゥオプション
	bool		m_bCentering;		//	ウィンドゥは画面全体に対してセンタリングして表示か(default:true)

	static LRESULT CALLBACK gWndProc(HWND,UINT,WPARAM,LPARAM);
	LRESULT Dispatch(HWND,UINT,WPARAM,LPARAM);	//	windows message dispatcher

	LRESULT		Initializer();	//	起動後、一度だけウィンドゥクラスを登録する
	//	フックしているすべてのインスタンスへのチェイン
	CWinHookList	m_HookPtrList;

	////////////////////////////////////////////////////////////////////////////
	//	マウスカーソルのOn/Offは、ウィンドゥに対する属性なので
	//	ウィンドゥクラスが担うべき
//	void	InnerShowCursor(bool bShow);
	bool	m_bShowCursor;			//	マウスカーソルの表示状態
	bool	m_bUseMouseLayer;		//	ソフトウェアマウスカーソルを使うか？

	//	ウィンドゥサイズのadjust
	void	InnerAdjustWindow(RECT&,CWindowOption&);

	//	メニュー付きウィンドゥか？
	bool	m_bUseMenu;

	//	ChangeWindowStyle⇔SetSizeのフラグ
	bool	m_bResized;

	//	Windows基本窓タイプを作成したときに、メッセージハンドラを
	//	フックするので、その関数ポインタを保存しておく必要がある。
	WNDPROC	m_pWndProc;

	//	最小化されているか？
	volatile bool	m_bMinimized;

	//	一回目の生成かどうか
	static bool	m_bFirstUserClass;
	bool IsWindowsClassName(const string& szClassName);
	//	Windowsで用意されているWindowクラス名かどうかを調べる

/*
	//	メニューが確実に存在することを事前に伝えておけば
	//	生成時に正確なウィンドゥサイズがいきなり求まる
	void		UseMenu(bool bUseMenu) { m_bUseMenu = bUseMenu; }
*/	//	自動的に判定するようにした

	static	bool g_bFullScreen;			//	フルスクリーンモードなのか？

	// override from CWinHook
	virtual LRESULT WndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);

};

#endif
