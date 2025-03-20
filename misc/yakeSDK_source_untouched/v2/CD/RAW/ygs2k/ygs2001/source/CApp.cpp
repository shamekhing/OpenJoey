#include "stdafx.h"

#include "CApp.h"
#include "TScript.h"

///////////////////////////////////////////////////////////////////////////////
//	アプリケーションの初期化
void	CApp::Initialize(){
	m_draw.SetDisplay(false);	//	ウィンドゥモード
//	m_draw.SetDisplay(true);	//	フルスクリーン

	m_fpstime.SetFPS(30);		//	FPSを30に

	//	ソフトウェアマウスカーソル
	m_mouseplane.Load("grp/cursor.bmp");
	m_mouseplane.SetColorKey(RGB(0,255,0));
	m_mouselayer.SetPlane(&m_mouseplane);
}

void	CApp::MainThread(void) {					//	これが実行される
	Initialize();

	TScript sc(this);

	//	イニシャライザ
	LRESULT hr;
	hr = sc.Load("script/gameinit.c",false);
	if (hr>0) return ;			//	コンパイルエラー
	if (hr==0) sc.ReExecute();	//	実行する

	if (sc.Load("script/gamestart.c",false)) return ;	//	コンパイルエラー

	while(IsThreadValid()) {			//	これがValidの間、まわり続ける
		if (sc.ReExecute()==0) break;	//	もういいらしい（笑）

		m_draw.OnDraw();
		m_fpstime.WaitFrame();

//		m_key.Input();
//		if (m_key.IsVKeyPush(0)) break;
	}

}

///////////////////////////////////////////////////////////////////////////////////
//	以下はメインクラス
///////////////////////////////////////////////////////////////////////////////////

//	これがmain windowのためのクラス。
class CAppMainWindow : public CAppBase {	//	アプリケーションクラスから派生
	virtual void MainThread(void){			//	これがワーカースレッド
		CApp().Start();						//	CApp app; app.Start();の意味ね
	}
	//	キャプション変更しとかんとどやされる＾＾
	LRESULT OnPreCreate(CWindowOption& opt){
		opt.caption		= "yaneuraoGameScript2001";
		opt.classname	= "YANEAPPLICATION";
		opt.size_x		= 640;
		opt.size_y		= 480;
		opt.style		= WS_SYSMENU | WS_MINIMIZEBOX | WS_CAPTION;
		return 0;
	}
};

//	言わずと知れたWinMain
int APIENTRY WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
{
	CAppInitializer::Init(hInstance,hPrevInstance,lpCmdLine,nCmdShow);	//	必ず書いてね
	CSingleApp sapp;
	if (sapp.IsValid()) {
		CAppMainWindow().Run();					//	上で定義したメインのウィンドゥを作成
	}
	return 0;
}
