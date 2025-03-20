//	packfile.h
//		programmed by yaneurao(M.Isozak) '99/9/5
//

#ifndef __PACKFILE_H__
#define __PACKFILE_H__

#include "stdafx.h"

#include <string>
using namespace std;

class CPackFile {
public:
	void	Pack();
	void	Unpack();
	void	FolderPack();

	void	PurePack();			// 02'04'13
	void	FolderPurePack();	// ただ上のをコピペして圧縮処理を削っただけです（＾＾；）

	void	PackOne();			// 02'04'20	圧縮ファイルと非圧縮ファイルを一まとめに

protected:

	UINT	GetOpenFolderName(string& Buffer, string DefaultFolder, string Title);
static	int CALLBACK SHBrowseProc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData);
static	void	_SHFree(ITEMIDLIST* pidl);
};

#endif