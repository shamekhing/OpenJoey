#include "stdafx.h"
#include <direct.h>
#include <shellapi.h>
#include "CApp.h"
#include "CScnConvert.h"

void	CApp::MainThread(void) {
	GetDraw()->SetDisplay(false);

	CFPSTimer t;
	t.SetFPS(10);

	string src_path;
	{
		UINT ret = GetOpenFolderName(src_path, CFile::GetCurrentDir().c_str(),
			"ヘッダファイルがあるフォルダを選択して下さい.\n"
			"サブフォルダ中のヘッダも検索対象です.\n"
			);
		if (ret==IDOK){
			CStringScanner::Replace(src_path,"\\","/");
			if(*src_path.rbegin()!='/') { src_path += '/'; }
		} else {
			return;
		}
	}
	string dst_path;
	{
		string temp = src_path;  CStringScanner::Replace(temp,"/","\\");
		UINT ret = GetOpenFolderName(dst_path, temp.c_str(),
			"ドキュメントを書き出すフォルダを選択して下さい.\n"
			"そのフォルダ中にdocフォルダを作って書き出します.\n"
			);
		if (ret==IDOK){
			CStringScanner::Replace(dst_path,"\\","/");
			if(*dst_path.rbegin()!='/') { dst_path += '/'; }
		} else {
			return;
		}
	}

	CTextFastPlane text;
	text.GetFont()->SetText("autoref　version 1.00");
	text.GetFont()->SetSize(20);
	text.UpdateTextAA();

	CDbg().Out("変換処理を開始します");

	string src_filename;
	vector<string> astrSrcFilename;
	CDir dir;
	dir.Reset();
	dir.SetPath(src_path);
	dir.SetFindFile("*.h");
	while (dir.FindFile(src_filename)==0){
		astrSrcFilename.push_back(src_filename);
	}
	dir.Reset();
	dir.SetPath(src_path);
	dir.SetFindFile("*.hpp");
	while (dir.FindFile(src_filename)==0){
		astrSrcFilename.push_back(src_filename);
	}

	GetDraw()->GetSecondary()->Clear();
	GetDraw()->GetSecondary()->BltNatural(&text,0,0);
	GetDraw()->OnDraw();
	{
		CScnConvert scn;
		scn.Convert(src_path, astrSrcFilename, dst_path + "doc");
	}
	CDbg().Out("すべての変換処理が終わりました");
	CDbg().Out("ウィンドゥの右上の×印で終了させてください");

	//	ウィンドゥの右上の×印で閉じてやー
	while (IsThreadValid()) {
		GetDraw()->GetSecondary()->Clear();
		GetDraw()->GetSecondary()->BltNatural(&text,0,0);
		GetDraw()->OnDraw();
		t.WaitFrame();
	}
//*/
}

//	これがmain windowのためのクラス。
class CAppMainWindow : public CAppBase {	//	アプリケーションクラスから派生
	virtual void MainThread(void){			//	これがワーカースレッド
		CApp().Start();						//	CApp app; app.Start();の意味ね
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

//  フォルダ選択ダイアログの起動
UINT CApp::GetOpenFolderName(string& Buffer, string DefaultFolder, string Title)
{
    BROWSEINFO bi;
    ITEMIDLIST* pidl;
    char szSelectedFolder[MAX_PATH];

    ZeroMemory(&bi, sizeof(BROWSEINFO));
    bi.hwndOwner = CAppInitializer::GetHWnd();
    //  コールバック関数を指定
    bi.lpfn   = CApp::SHBrowseProc;
    //  デフォルトで選択させておくフォルダを指定
    bi.lParam = (LPARAM)DefaultFolder.c_str();
    //  タイトルの指定
    bi.lpszTitle = Title.c_str();//"フォルダを選択してください";
    //  フォルダダイアログの起動
    pidl = SHBrowseForFolder(&bi);
    if(pidl)
    {
        //  選択されたフォルダ名を取得
        SHGetPathFromIDList(pidl, szSelectedFolder);
        _SHFree(pidl);
        if((DWORD)lstrlen(szSelectedFolder) < MAX_PATH){
            Buffer = szSelectedFolder;
		}else{
			Buffer = "";
		}
        //  フォルダが選択された
        return IDOK;
    }
    //  フォルダは選択されなかった
    return IDCANCEL;
}

/////////////////////////////////////////////////////////////////////////////
//  SHBrowseForFolder()用コールバック関数

int CALLBACK CApp::SHBrowseProc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    if(uMsg == BFFM_INITIALIZED && lpData)
    {
        //  デフォルトで選択させるパスの指定
        SendMessage( hWnd, BFFM_SETSELECTION, TRUE, lpData);
    }
    return 0;
}

/////////////////////////////////////////////////////////////////////////////
//  システムが確保したITEMIDLISTを開放しなければならない

void CApp::_SHFree(ITEMIDLIST* pidl)
{
    IMalloc* pMalloc;
    SHGetMalloc(&pMalloc);
    if (pMalloc)
    {
        pMalloc->Free(pidl);
        pMalloc->Release();
    }
}
