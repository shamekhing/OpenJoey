#include "stdafx.h"
#include "yaneShell.h"
#include "yaneFile.h"
#include "../AppFrame/yaneAppInitializer.h"

LRESULT	CShell::Execute(string szExeName,bool bWait){
	//　ファイル実行
	CHAR buf[512];
	::lstrcpy(buf,szExeName.c_str());

	STARTUPINFO startupInfo = { 0 } ;
	PROCESS_INFORMATION ProcessInfo;
	
	if (!::CreateProcess(NULL,buf,NULL,NULL,FALSE,
		CREATE_NO_WINDOW,NULL,NULL,&startupInfo,&ProcessInfo))
		//	CREATE_NO_WINDOWは、MS-DOS窓を作らないオプション
		//	（標準出力を殺したいので）
		return 1;	// unable open..

	if (bWait){
		::WaitForSingleObject(ProcessInfo.hProcess ,INFINITE);
	}

	return 0;
}

LRESULT	CShell::OpenFolder(string szPathName){
	//	フォルダをexplorerで開く

	CHAR buf[512];
	::lstrcpy(buf,"explorer ");
	::lstrcat(buf,szPathName.c_str());

	STARTUPINFO startupInfo = { 0 };
	PROCESS_INFORMATION ProcessInfo;

	if (!::CreateProcess(NULL,buf,NULL,NULL,FALSE,
		0,NULL,NULL,&startupInfo,&ProcessInfo))
		return 1;	// unable open..

	return 0;
}

LRESULT CShell::CopyAndSetWallPaper(string szFileName){
	CFile file;
	if (file.Read(szFileName)!=0) return 1;

	//	システムフォルダにコピー
	string name;
	name =	CFile::GetWindowsDir() + file.GetPureFileName();
	if (file.WriteBack(name)!=0) return 1;
	return SetWallPaper(name);
}

LRESULT CShell::SetWallPaper(string szFileName){
	//	壁紙を設定する

	//	アクティブデスクトップのために、レジストリも変更しなければならない

//	HKEY_CURRENT_USER/Desktop/WindowMetrics/WallPaper
//										   /WallpaperStyle/←0
//	↑このセクションの存在の有無は調べなくてはならない

	return ::SystemParametersInfo(
		SPI_SETDESKWALLPAPER,		//	壁紙の変更
		0,
		(void*)szFileName.c_str(),
		SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE
		//	変更は通知したほうがいいだろう..
	)?0:1;

}

string	CShell::getModuleName() const {
	CHAR szFileName[_MAX_PATH];
	::GetModuleFileName(CAppInitializer::GetInstance(),szFileName,_MAX_PATH);
	return string(szFileName);
}

string	CShell::getModuleMutexName() const {
	//	起動Exe名を、多重起動防止のMutex名とする
	CHAR szFileName[_MAX_PATH];
	::GetModuleFileName(CAppInitializer::GetInstance(),szFileName,_MAX_PATH);
	//	Mutex名には\は使えないので+に置換
	for(int i=0;i<_MAX_PATH;i++){
		if (szFileName[i]=='\\') szFileName[i] = '?';
		// ? はMutex名には使えるがファイル名には出現しないのでＯＫ
		else if (szFileName[i]=='\0') break;
	}
	return string(szFileName);
}

