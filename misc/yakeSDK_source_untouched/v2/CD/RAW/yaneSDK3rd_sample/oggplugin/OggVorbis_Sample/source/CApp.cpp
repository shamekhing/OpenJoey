#include "stdafx.h"
#include "CApp.h"

void CApp::MainThread()
{
	CSoundFactory manage;
	manage.GetSoundParameter()->SetGlobalFocus(true);
	manage.GetSoundParameter()->GetStreamFactory()->GetPlugInMap()->Write("ogg", "CVorbisStream");
	manage.SetStreamPlay(true);
	smart_ptr<ISound> bgm = manage.CreateSound();
	bgm->SetLoopPlay(true);
	bgm->Open("hanya-n.ogg");
}

//	言わずと知れたWinMain
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	CAppInitializer init(hInstance,hPrevInstance,lpCmdLine,nCmdShow);
	//	必ず書いてね

	// ここでロードする
	CObjectCreater* pCreater = CObjectCreater::GetObj();
	pCreater->LoadPlugIn("plugin/enraogg.dll");

	CSingleApp sapp;
	if (sapp.IsValid()) {
		CAppMainWindow().Run();
		//	上で定義したメインのウィンドゥを作成
	}

	// ここでレリース(笑)する
	// こうしないと、CAppFrameのデストラクタで落ちる事になる
	// （DLLを解放すると、そこから来たオブジェクトが不正になる）
	pCreater->ReleasePlugIn("yaneSDK/plugin/enraogg.dll");

	return 0;
}
