//
//	アプリケーションイニシャライザ
//		yaneuraoGameSDK 2ndのためのイニシャライザ。
//		気にいらなければ自由にカスタマイズして使ってくださいな＾＾
//

#ifndef __yaneAppInitializer_h__
#define __yaneAppInitializer_h__

#include "../Window/yaneWinHook.h"

class CAppCallbackList {
public:
	void	RegistInitCallback(const smart_ptr<function_callback>& p){
		GetInitCallbackList()->push_back(p);
	}
	void	RegistExitCallback(const smart_ptr<function_callback>& p){
		GetExitCallbackList()->push_back(p);
	}

	void	InitCallback();	//	RegistInitCallbackされた関数を順番に呼び出す
	void	ExitCallback();	//	RegistExitCallbackされた関数を順番に呼び出す

	smart_list_ptr<function_callback> * GetInitCallbackList()
	{ return &m_listInitFunc;}
	smart_list_ptr<function_callback> * GetExitCallbackList()
	{ return &m_listExitFunc;}

protected:
	//	初期化時コールバック用
	smart_list_ptr<function_callback> m_listInitFunc;
	//	終了時のコールバック用
	smart_list_ptr<function_callback> m_listExitFunc;
};

class CAppInitializer {
/**
	アプリケーション初期化のためのクラス

	コンストラクタにWinMainからそのパラメータをそのまま渡してね。

	非ローカルstaticオブジェクトによるコールバック予約が出来ます。
		RegistInitCallback(コールバックを予約する関数);
		RegistExitCallback(コールバックを予約する関数);
	とすれば、コンストラクタとデストラクタにおいて、その関数に
	自動的にコールバックされます。（複数登録可）

*/
public:
	///	WinMainからそのパラメータをそのまま渡してね。
	CAppInitializer(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow);

	///	このデストラクタでRegistExitCallbackしておいた関数が呼び出される
	~CAppInitializer();

	///	singletonを通じて取得する
	static HINSTANCE GetInstance()		{ return GetObj()->GetInstance2(); }
	static HINSTANCE GetPrevInstance()	{ return GetObj()->GetPrevInstance2(); }
	static LPSTR	 GetCmdLine()		{ return GetObj()->GetCmdLine2(); }
	static int		 GetCmdShow()		{ return GetObj()->GetCmdShow2(); }

	///		Init呼び出しのあとにコールバックして欲しいものを登録しておく
	static void	RegistInitCallback(const smart_ptr<function_callback>& p)
	{	GetCallbackObj()->RegistInitCallback(p); }

	///		終了時にコールバックして欲しいものを登録しておく
	static void	RegistExitCallback(const smart_ptr<function_callback>& p)
	{	GetCallbackObj()->RegistExitCallback(p); }

	//////////////////////////////////////////////////////////////

	///		singletonオブジェクトの取得
	static CAppInitializer* GetObj() { return m_vObj; }

	///		WinMainで渡しておいたパラメータの取得
	HINSTANCE GetInstance2()		{ return m_hInstance; }
	HINSTANCE GetPrevInstance2()	{ return m_hPrevInstance; }
	LPSTR	 GetCmdLine2()			{ return m_lpCmdLine; }
	int		 GetCmdShow2()			{ return m_nCmdShow; }

protected:

	//	instance parameter...
	HINSTANCE	m_hInstance;
	HINSTANCE	m_hPrevInstance;
	LPSTR		m_lpCmdLine;
	int			m_nCmdShow;

private:
	//	singletonオブジェクト
	static CAppInitializer* m_vObj;

	static singleton<CAppCallbackList>	m_AppCallBackObj;
	static CAppCallbackList* GetCallbackObj()
	{ return m_AppCallBackObj.get(); }
};

#endif
