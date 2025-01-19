#include "stdafx.h"
#include "yaneDebugWindow.h"

LRESULT CDebugWindow::OnPreCreate(CWindowOption &opt){		//	override from CAppBase

	opt.caption		= "でばっぐ表示～";
	opt.classname	= "LISTBOX";
	opt.size_x		= 300;
	opt.size_y		= 200;
	opt.style		=	WS_VISIBLE | WS_CAPTION | LBS_NOINTEGRALHEIGHT | WS_VSCROLL
					| WS_SIZEBOX									//	サイズは可変
					| WS_MAXIMIZEBOX | WS_MINIMIZEBOX| WS_SYSMENU;	//	これを入れると、ウィンドウの×印がつく。

	m_bIdle = true; // Idleモードにする

	return 0;
}

void	CDebugWindow::Clear(void){
	::SendMessage(GetHWnd(),LB_RESETCONTENT,0,0L);
}

void __cdecl CDebugWindow::Out( LPSTR fmt, ... ) {
	HWND hWnd = GetHWnd();
	if (hWnd == NULL) return ;
	
	CHAR	buf[512];
	::wvsprintf(buf,fmt,(LPSTR)(&fmt+1) );
	
	int n = strlen(buf) - 1;
	if (buf[n] == '\n') buf[n] = '\0'; // 改行を潰す

	::SendMessage(hWnd,LB_ADDSTRING,0,(LONG)(LPSTR) buf);
	UINT sel = (UINT)::SendMessage(hWnd,LB_GETCOUNT,0,0L);
	if(sel!=LB_ERR) {
		::SendMessage(hWnd,LB_SETCURSEL,sel-1,0L);
	}
}

void	CDebugWindow::Out(const string& str){
	Out((LPSTR)str.c_str());
}

void	CDebugWindow::Out(int i){
	CHAR buf[16];
	::wsprintf(buf,"%d",i);
	Out(buf);
}

void	CDebugWindow::Out(LONG*lpl){
	//	可変引数取るか～世話やけんなぁ（笑）
	CHAR	buf[512];
	::wvsprintf(buf,(LPSTR)*lpl,(LPSTR)(lpl+1) );
	Out(buf);
}


//////////////////////////////////////////////////////////////////////////////
//	チェックする

auto_ptrEx<CDebugWindow> CDbg::m_lpDebugWindow;

void	CDbg::CheckValid(void){
	if (m_lpDebugWindow==NULL) {
		m_lpDebugWindow.Add( new CDebugWindow );
		m_lpDebugWindow->Run();
	}
}

void __cdecl CDbg::Out( LPSTR fmt, ... ) {
	CheckValid();

	//	可変引数取るか～世話やけんなぁ（笑）
	CHAR	buf[512];
	::wvsprintf(buf,fmt,(LPSTR)(&fmt+1) );
	m_lpDebugWindow->Out(buf);
}
