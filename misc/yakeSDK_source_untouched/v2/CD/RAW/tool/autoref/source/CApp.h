
#ifndef __CApp_h__
#define __CApp_h__

//#include "../yaneSDK/yaneSDK.h"
#include "winnetwk.h"
#include "shlobj.h"

class CApp : public CAppFrame {
public:

	CFastDraw*	GetDraw(void)	{ return& m_Draw; }
	CKey*		GetKey(void)	{ return& m_Key;  }

protected:
	//	‚±‚¢‚Â‚ªƒƒCƒ“‚Ë
	void	MainThread(void);		  //  ‚±‚ê‚ªÀs‚³‚ê‚é

	CFastDraw	m_Draw;
	CKey		m_Key;

private:
	void _SHFree(ITEMIDLIST* pidl);
	static int CALLBACK SHBrowseProc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData);
	UINT GetOpenFolderName(string& Buffer, string DefaultFolder, string Title);
};

#endif
