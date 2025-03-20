//
//	yaneShell.h :
//
//		シェルコマンド実行用
//

#ifndef __yaneShell_h__
#define __yaneShell_h__

class CShell {
public:
	LRESULT	Execute(string szExeName,bool bWait=false); //　ファイル実行
	///	外部コマンドを実行します。bWait==trueの場合は、そのコマンドの終了を待ちます。

	LRESULT	OpenFolder(string szPathName);	//	フォルダをexplorerで開く
	///	指定のフォルダを開きます。

	LRESULT SetWallPaper(string szFileName);
	///	指定のファイルを壁紙にします。yanePack/Ex/Dxで圧縮されている
	///	ファイルは不可です。

	LRESULT CopyAndSetWallPaper(string szFileName);
	///	指定のファイルをWindowsフォルダにコピーし、それを壁紙にします。
	///	yanePack/yanePackExで圧縮されているファイルでも可能です。

	string getModuleMutexName() const;
	///	起動Exe名から多重起動防止用のMutex名に変換して返す

	string getModuleName() const;
	///	起動モジュール名を返す(full path)
};

#endif