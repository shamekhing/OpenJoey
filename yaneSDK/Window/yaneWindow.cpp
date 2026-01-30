// yaneWindow.cpp
#include "stdafx.h"
#include "yaneWindow.h"
#include "yaneWinHook.h"
#include "../Auxiliary/yaneIMEWrapper.h"
#include "../AppFrame/yaneAppInitializer.h"
#include "../Auxiliary/yaneStream.h"
#include "../AppFrame/yaneAppManager.h"

namespace yaneuraoGameSDK3rd {
namespace Window {

bool CWindow::g_bFullScreen		= false; // ÂNÂÂ®ÂÂ¼ÂÃ£ÂÃÂEÂBÂÂÂhÂDÂÂÂ[ÂhÂÃÂÂµÂÃ¥ÂI
CCriticalSection CWindow::m_cs;

//////////////////////////////////////////////////////////////////////////////

CWindow::CWindow(){
	m_dwFillColor	=	0;			//	ÂNÂÂ®ÂÂÂÂÂÂªÂfÂBÂtÂHÂÂÂg
	m_hWnd			=	NULL;
	m_bShowCursor	=	true;		//	ÂfÂBÂtÂHÂÂÂgÂÃÂ}ÂEÂXÂJÂ[Â\ÂÂÂÃ°Â\ÂÂ¦
	m_bUseMouseLayer=	false;		//	Â\ÂtÂgÂEÂFÂAÂ}ÂEÂXÂJÂ[Â\ÂÂ
	m_bUseMenu		=	false;		//	ÂÂÂjÂÂÂ[ÂÃÂÂ ÂÃ©ÂÃ¢ÂÃÂÂµÂÃ¢ÂÃÂÃ­ÂÂ©ÂÃ§ÂÃ±ÂOÂOÂG
	m_bMinimized	=	false;		//	ÂÃÂÂ¬ÂÂ»ÂÂ³ÂÃªÂÃÂÂ¢ÂÃ©ÂÂ©ÂH
	m_bResized		=	false;		//	ChangeWindowStyleÂÃSetSizeÂÃÂtÂÂÂOÂB
	m_pWndProc		=	NULL;		//	hookÂÂµÂÂ½WndProc
}

CWindow::~CWindow(){
	GetHookList()->Del(this);		//	ÂÂ¼ÂÃÂtÂbÂNÂÃ°ÂÃ°ÂÂÂÂ·ÂÃ©
/*
	if (m_hWnd!=NULL) {
		::DestroyWindow(m_hWnd);
	//	ÂÂªÂÂ±ÂÃ±ÂÃÂÂ­ÂÃ¸ÂÃÂÃ»Â@ÂÃDestroyÂÂ·ÂÃ©ÂÃÂÃÂAÂÂ¨ÂÂ©ÂÃÂÂµÂÂ©ÂÃÂÃ©ÂOÂOÂG
	//	Â{ÂÂÂACAppBaseÂÂ©ÂÃ§ÂgÂÂ¤ÂÃÂÃÂÃ§ÂÃÂAÂÂ±ÂÃÂfÂXÂgÂÂÂNÂ^ÂÃÂÃÂAÂÂ·ÂÃÂÃ
	//	WindowÂÃÂÃ°ÂÃÂÂ³ÂÃªÂÂ½ÂÂ ÂÃÂÃÂÃÂÂ¸ÂÂ¾ÂÂª...
		m_hWnd	= NULL;
	}
*/
}

//////////////////////////////////////////////////////////////////////////////
/*
LRESULT CALLBACK CWindow::gDlgProc(HWND hDlgWnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
        case WM_INITDIALOG:
            return FALSE;
        case WM_COMMAND:
            switch (LOWORD(wp)) {
                case IDOK:
					::EndDialog(hDlgWnd, IDOK);
                    break;
                case IDCANCEL:
					::EndDialog(hDlgWnd, IDCANCEL);
                    break;
                default:
                    return FALSE;
            }
        default:
            return FALSE;
    }
    return TRUE;
}
*/

LRESULT CWindow::Create(CWindowOption& opt,HWND hParent){
	CCriticalLock cl(GetCriticalSection());

	HINSTANCE hInst = CAppInitializer::GetInstance();
	m_bFullScreen = g_bFullScreen;	//	ÂÂ»ÂÃÂÃÂÃ¦ÂÃÂÂÂ[ÂhÂÃ°ÂÃ ÂÂÂIÂÃÂÃÂÂ
	m_opt	= opt;	//	ÂRÂsÂ[ÂÂµÂÃÂÂ¨ÂÂ­

	m_hWnd = NULL;

	LONG lChild = 0;

	//	ÂÃÂÂ½ÂCÂÂªÂÃ¼ÂÂ¢ÂÂ½ÂÃ§ÂTÂ|Â[Âg
	if (m_opt.dialog!=NULL){
	//	dialogÂÃ¬ÂÃ©ÂÃ§ÂÂµÂÂ¢ÂÃÂH
		m_hWnd = ::CreateDialogParam(hInst,m_opt.dialog, hParent, (DLGPROC)gDlgProc,
			reinterpret_cast<LPARAM>(this));
			//	ÂÂªthisÂÂªLPARAMÂÃÂÃ¼ÂÃÂÃÂÃ¢ÂÃÂÃÂÂ­ÂÃ©ÂI
		goto window_setting;
	}

    // ÂÃÂÂ¾ÂoÂ^ÂÂ³ÂÃªÂÃÂÂ¢ÂÃÂÂ¢ÂEÂBÂÂÂhÂEÂNÂÂÂXÂÂ¼ÂÃÂAÂÂ©ÂÃWindowsÂWÂÂÂNÂÂÂX
    // ÂÃÂOÂÃÂÂ¼ÂOÂÃÂÃªÂÂÂÃRegisterClassEx()ÂÂµÂÃÂÃ¢ÂÃ©
    WNDCLASSEX wndclass;
    if( !::GetClassInfoEx(hInst, opt.classname.c_str(), &wndclass) &&
        !IsWindowsClassName(opt.classname)) {

        // ÂÂ¨ÂÂ«ÂÃÂÃ¨ÂÃÂEÂCÂÂÂhÂEÂNÂÂÂXÂÂ¶ÂÂ¬ÂÂÂÂ
		wndclass.cbSize		= sizeof(WNDCLASSEX);
		wndclass.style		= 0;	//	Â_ÂuÂÂÂNÂÂÂbÂNÂÂ´ÂmÂÂ·ÂÃ©ÂÃÂÃ§ÂÂ¨CS_DBLCLKS;
		wndclass.lpfnWndProc= gWndProc;
		wndclass.cbClsExtra	= 0;
		wndclass.cbWndExtra	= 0;
		wndclass.hInstance	= hInst;

		// ÂÃÂÃ¨ÂÂ ÂÂ¦ÂÂ¸ÂA"MAIN"ÂÃÂAÂCÂRÂÂÂ\ÂÂ¦
		wndclass.hIcon		= LoadIcon(hInst,"MAIN");
		wndclass.hIconSm	= LoadIcon(hInst,"MAIN");

		wndclass.hCursor	= LoadCursor(NULL,IDC_ARROW);
		// ÂÃÂÃ¨ÂÂ ÂÂ¦ÂÂ¸ÂAÂ}ÂEÂXÂJÂ[Â\ÂÂÂÃ ÂpÂÃÂiÂsÂvÂÃÂÃ§ÂÃÂAÂÂ ÂÃÂÃÂÃÂÂ·ÂÃÂÂµÂIÂj

		//	ÂÃÂÃ¨ÂÂ ÂÂ¦ÂÂ¸ÂAÂNÂÂ®ÂÂ¼ÂÃ£ÂÃÂAÂÂÂÂ©ÂÂÂÃÂÂµÂÃÂÂ±ÂÃÂÃÂc
		if (m_dwFillColor==0) {
			wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		} else {
			wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		}

		//	MENUÂÃ ÂÂ¼ÂÃÂÃÂÃÂÂÂÃ±ÂÃÂÃÂÃ©
		wndclass.lpszMenuName  = "IDR_MENU";
		//	ÂÂ±ÂÃÂÂÂ\Â[ÂXÂÂªÂ{ÂÂÂÃÂÂ¶ÂÃÂÂ·ÂÃ©ÂÃÂÂ©ÂÃÂAÂÃÂsÂÂÂÃÂÃÂÃ­ÂÂ©ÂÃ§ÂÃÂÂ¢ÂÃÂÃ
		//	ÂÂ³ÂmÂÃÂEÂBÂÂÂhÂDÂÃÂAÂhÂWÂÂÂXÂgÂÂªÂEÂBÂÂÂhÂDÂÂ¶ÂÂ¬ÂÃ£ÂÃÂÂµÂÂ©ÂoÂÂÂÃÂÂ¢

		{
			HRSRC hrsrc;
			hrsrc = ::FindResource(NULL,"IDR_MENU",RT_MENU);
			if (hrsrc!=NULL) { m_bUseMenu = true; }	//	ÂÂÂjÂÂÂ[ÂÂ ÂÃ¨ÂÃ¢ÂÂªÂÃ±ÂÃÂOÂOÂG
		}

		wndclass.lpszClassName = opt.classname.c_str();

		// ÂÃÂÃ±ÂÃÂÂ¸ÂsÂÂµÂÃÂÃ±ÂÂ¾ÂÃ«ÂÃÂH
		if (!::RegisterClassEx(&wndclass)) {
			Err.Out("CWindow::CreateÂÃRegisterClassExÂÂ¸Âs");
			return 1;
		}
	}

	if (hParent!=NULL) {
		lChild	= WS_CHILDWINDOW;
	}

	if (m_bFullScreen){
		//	ÂEÂCÂÂÂhÂEÂÃÂÂ¶ÂÂ¬ÂiÂtÂÂÂXÂNÂÂÂ[ÂÂÂÂÂj
		//	ÂÂ±ÂÃÂÃÂÂ«ÂÃÂAÂÃ¦ÂÃÂÃÂXÂ^ÂCÂÂÂIÂvÂVÂÂÂÂÂÃÂÂ³ÂÂÂÂ·ÂÃ©

		int sx,sy;
		GetScreenSize(sx,sy);
		m_hWnd = ::CreateWindow(opt.classname.c_str(),opt.caption.c_str(),
						WS_POPUP|WS_VISIBLE	 /* |WS_SYSMENU|
						WS_MAXIMIZEBOX|WS_MINIMIZEBOX */ ,
						0, 0, sx, sy,
						hParent, NULL, hInst, NULL );
	} else {
		// ÂEÂBÂÂÂhÂDÂÂ«ÂEÂÃ°ÂlÂÂ¶ÂÃÂÃ¼ÂÃªÂÃÂÂ­ÂÃÂÃÂÃÂÃ§ÂÃÂÂ¢
		// Â\ÂÂ¦ÂÃÂÃÂÂª640*480ÂÃÂÃ§ÂÃÂAÂÂ¶ÂÂ¬ÂÂ·ÂÃÂÂ«WindowÂTÂCÂYÂÃÂAÂÂ»ÂÃªÂÃÂÃ£ÂÃÂÂ ÂÃ©
		RECT r;
		InnerAdjustWindow(r,opt);
		m_hWnd = ::CreateWindow(opt.classname.c_str(),opt.caption.c_str(),
				// WS_EX_TOPMOSTÂÃ°ÂwÂÃ¨ÂÂµÂÂ½ÂÂ¢ÂÃ±ÂÂ¾ÂÂ¯ÂÃÂÃÂ[
						lChild | opt.style,
						CW_USEDEFAULT,
						CW_USEDEFAULT,
						r.right-r.left,r.bottom-r.top,
						hParent,NULL,hInst,NULL);
	}

window_setting:;
	if (m_hWnd==NULL){
		Err.Out("CWindow::CreateÂÃCreateWindowÂÃÂÂ¸ÂsÂB");
		return 1;
	}

	//	ÂRÂ[ÂÂÂoÂbÂNÂÂ³ÂÃªÂÂ½ÂÃÂÂ«ÂÃÂAÂÂ»ÂÃªÂÂ¼ÂÃªÂÃWindowClassÂÃdispatchÂoÂÂÂÃ©ÂÃ¦ÂÂ¤ÂÃ
	//	GWL_USERDATAÂÃthisÂÃ°ÂBÂÂµÂÃÂÂ¨ÂÂ­ÂB
	::SetWindowLong(m_hWnd,GWL_USERDATA,(LONG)this);

	//	ÂÃ¦ÂÃÂÃÂZÂÂÂ^Â[ÂÃÂÂ¶ÂÂ¬ÂÂ·ÂÃ©ÂÃ¦ÂÂ¤ÂÃÂCÂÂ³ÂOÂO
	if ((!m_bFullScreen) && (m_opt.size_x!=0) && (m_opt.size_y!=0)
			&& m_opt.bCentering) {
		int	sx,sy;
		GetScreenSize(sx,sy);
		sx	 = (sx-m_opt.size_x) >> 1;
		sy	 = (sy-m_opt.size_y) >> 1;
		SetWindowPos(sx,sy);
	}

	if ((m_opt.size_x!=0) && (m_opt.size_y!=0)){
		::ShowWindow(m_hWnd, SW_SHOW);
	}

	::SetFocus(m_hWnd);

	//	ÂWÂÂÂÃWindowClassÂÃÂÃ§ÂÃÂAÂRÂ[ÂÂÂoÂbÂNÂÃÂÂÂÃÂÂµÂÃÂAÂÂ±ÂÃªÂÃ°
	//	ÂÂÂÂ«ÂÂÂÃ±ÂÃÂÃ¢ÂÃ©ÂB
	if (IsWindowsClassName(opt.classname)){
		m_pWndProc = (WNDPROC)::SetWindowLong(m_hWnd,GWL_WNDPROC,(LONG)gWndProc);
		//	ÂÂ±ÂÂ¤ÂÂ¢ÂÂ¤hookÂÃÂAillegalÂÂ©...
	} else {
		m_pWndProc = NULL;
	}

	//	ÂÂ©ÂÂªÂÃÂEÂBÂÂÂhÂDÂÃÂÃÂÃÂÂ¼ÂÃÂtÂbÂNÂÃÂÂ«ÂÃ©
	//	ÂiÂÃÂÂ¢ÂÂ¤ÂÂ©CAppManagerÂÃÂÂ¢ÂoÂ^ÂÃÂiÂKÂÃÂÃÂÃÂÂ¼ÂÃÂtÂbÂNÂÂµÂÂ©ÂÃÂÂ¢Âj
	GetHookList()->Add(this);			//	ÂtÂbÂNÂÃ°ÂJÂnÂÂ·ÂÃ©

	return 0;
}

void CWindow::GetScreenSize(int &x,int &y){
	x = ::GetSystemMetrics(SM_CXSCREEN);
	y = ::GetSystemMetrics(SM_CYSCREEN);
}

LRESULT CWindow::SetWindowPos(int x,int y){
	if (m_hWnd==NULL) return 0;
	//	ÂEÂBÂÂÂhÂEÂÂªÂÂ¶ÂÂ¬ÂÂ³ÂÃªÂÃÂÂ¢ÂÂ½ÂÃ§ÂÃÂÂ®ÂÂ³ÂÂ¹ÂÃÂÂ­ÂÃÂÃÂÃÂÃ§ÂÃÂÂ¢ÂiÂÂ½ÂÂ¾ÂÂµÂEÂBÂÂÂhÂDÂÂÂ[ÂhÂÂÂj
	if (!m_bFullScreen){
		//	ÂÂ±ÂÂ±ÂÃÂgÂbÂvÂIÂ[Â_Â[ÂÃÂÂµÂÃÂÂ¨ÂÂ¢ÂÃÂÃ ÂÃ¢ÂÃ¨ÂÃÂÂ¢ÂÃÂÂ¾ÂÃ«ÂÂ¤...
		return !::SetWindowPos(m_hWnd,HWND_TOP,x,y,NULL,NULL,SWP_NOSIZE|SWP_NOZORDER);
	}
	return 0;
}

void CWindow::SetSize(int sx,int sy){
	if (m_opt.size_x != sx || m_opt.size_y != sy){
		m_opt.size_x = sx;
		m_opt.size_y = sy;
		m_bResized	= true;	//	ÂÃÂXÂÂ³ÂÃªÂÂ½ÂÃÂtÂÂÂOÂOÂOÂG
	}
}

LRESULT CWindow::Resize(int sx,int sy){
	m_opt.size_x = sx;
	m_opt.size_y = sy;

	if (m_hWnd==NULL) return 0;

	if (!m_bFullScreen){
		//	ÂÂÂÂÂ[ÂhÂÃÂÃÂÃ...
		if ((m_opt.size_x!=0) && (m_opt.size_y!=0)){
			::ShowWindow(m_hWnd, SW_SHOW);	//	ÂOÂÃÂÂ½ÂÃÂÃÂÂÂÃ©ÂOÂOÂG
		}
		RECT r;
		InnerAdjustWindow(r,m_opt);
		return !::SetWindowPos(m_hWnd,NULL,0,0,r.right-r.left,r.bottom-r.top,SWP_NOMOVE|SWP_NOZORDER);
	} else {
		//	ÂtÂÂÂXÂNÂÂÂ[ÂÂÂÃÂÃ§ÂÃÂAÂÂÂTÂCÂYÂÃÂÃ±ÂTÂ|Â[Âg
	}

	return 0;
}

void	CWindow::UseMouseLayer(bool bUse){
	if (bUse==m_bUseMouseLayer) return ;
	m_bUseMouseLayer = bUse;
	if (bUse) {
		//	ÂgÂÂ¤ÂÃÂÃ§ÂÃÂÂ³ÂÃ°ÂÂÂÃÂÃÂÂ·
		ShowCursor(false);
	} else {
		//	ÂgÂÃ­ÂÃÂÂ¢ÂÃÂÃÂÂ ÂÃªÂÃÂAÂEÂBÂÂÂhÂDÂÂÂ[Âh(or DIBDraw)ÂÃÂÃ§ÂÃÂ}ÂEÂXÂJÂ[Â\ÂÂÂÂÂÂ
		if (!IsFullScreen()
/*
#ifdef USE_DIB32
			|| CAppManager::GetMyDIBDraw()!=NULL
			|| CAppManager::GetMyFastDraw()!=NULL
#endif
*/
		){
			ShowCursor(true);
		}
	}
}

void	CWindow::ChangeWindowStyle() {
	//	ÂÂ»ÂÃÂÃÂÃ¦ÂÃÂÂÂ[ÂhÂÃÂAÂEÂBÂÂÂhÂEÂXÂ^ÂCÂÂÂÂªÂÂÂvÂÂ·ÂÃªÂÃÂÃÂXÂÃÂKÂvÂÃÂÂµ
	if (m_bFullScreen==g_bFullScreen && !m_bResized) return ;

	m_bFullScreen	= g_bFullScreen;
	m_bResized		= false;

	// ÂÃÂÃÂÃ©ÂÃªÂÂÂÃÂEÂBÂÂÂhÂDÂXÂ^ÂCÂÂÂÃ°ÂÂ»ÂÃÂÃÂÂÂ[ÂhÂÃÂÂÂÃ­ÂÂ¹ÂÃÂÃÂX
	if (!m_bFullScreen) {
		RECT r;
		int	sx,sy;
		GetScreenSize(sx,sy);
		r.left	 = (sx-m_opt.size_x) >> 1;
		r.top	 = (sy-m_opt.size_y) >> 1;
		r.right	 = r.left + m_opt.size_x;
		r.bottom = r.top  + m_opt.size_y;
		sx = r.left;	sy = r.top;
//		SetWindowPos(r.left,r.top);
		InnerAdjustWindow(r,m_opt);
		::SetWindowPos(m_hWnd,HWND_TOP,sx,sy,r.right-r.left,r.bottom-r.top,SWP_NOZORDER);

		//	ÂÂ±ÂÂ±ÂÃÂASetWindowLong() ÂÃÂÂ»ÂÃÂÃÂÃ³ÂÃ(ÂÃÂÃ¥ÂÂ»ÂAÂÃÂÂ¬ÂÂ»ÂAÂÃÂÃ­)
		//	ÂÃ°ÂÃÂÃ¨ÂÂ·ÂÃ©(ÂÃªÂÃ '02/03/16)
		UINT nState = 0;// ÂEÂCÂÂÂhÂEÂÃÂÃ³ÂÃ
		if(::IsIconic(m_hWnd))nState = WS_MINIMIZE;
		else if(::IsZoomed(m_hWnd))nState = WS_MAXIMIZE;
		::SetWindowLong(m_hWnd,GWL_STYLE,m_opt.style | nState );
		//	WS_CAPTIONÂÃÂÂ·('01/04/24)Â@ÂÃ¢ÂÃÂÃÂAÂÂ±ÂÃªÂÃÂÂÂ[ÂUÂ[ÂÂªÂwÂÃ¨ÂÂµÂÃÂÂ«ÂÃ¡Â_ÂÂ

		::SetMenu(m_hWnd,GetMenu(m_hWnd)); // ÂÂÂjÂÂÂ[ÂÃÂÃ¦ÂÃÂÃÂ`ÂÃ¦
		::ShowWindow(m_hWnd,SW_SHOW);
		CIMEWrapper::GetObj()->Show();
	} else {
		::SetWindowLong(m_hWnd, GWL_STYLE, WS_POPUP);
		::SetMenu(m_hWnd,GetMenu(m_hWnd)); // ÂÂÂjÂÂÂ[ÂÃÂÃ¦ÂÃÂÃÂ`ÂÃ¦
		::ShowWindow(m_hWnd,SW_SHOW);
		CIMEWrapper::GetObj()->Hide();
	}
}

//	ÂEÂBÂÂÂhÂDÂTÂCÂYÂÃadjustÂÂÂÂ
//	Win32::AdjustWindowÂÃbug-fixÂp
//		(C) Tia-Deen & yaneurao '00/08/01,'00/11/05
void	CWindow::InnerAdjustWindow(RECT&rc,CWindowOption&opt){
	::SetRect(&rc,0,0,opt.size_x,opt.size_y);
	LONG	lStyle = opt.style;
/*	//	ÂÃ¢ÂÃÂÃÂAÂÂ±ÂÃÂRÂ[ÂhÂÃuserÂÂªÂwÂÃ¨ÂÂ·ÂÃÂÂ« '01/11/14
	if (lStyle & WS_POPUP) {
		lStyle |= WS_CAPTION;
	}
*/
	bool	bMenu = false;
	if (lStyle & WS_SYSMENU){
		//	m_hWndÂÂªNULLÂÃÂÃ³ÂÃÂÃGetMenuÂÃ°ÂÃÂÃÂoÂÂ·ÂÃÂÃÂsÂÂ³ÂH
		if (m_bUseMenu || (m_hWnd!=NULL && ::GetMenu(m_hWnd)!=NULL)){
			bMenu = true;
		} else {
			bMenu = false;
			lStyle &= ~WS_SYSMENU;	//	SYSMENUÂtÂÂÂOÂÃ°ÂOÂÂ·
		}
	//	lStyle |= WS_CAPTION;	//	ÂÂµÂÂ©ÂÂµÂAÂLÂÂÂvÂVÂÂÂÂÂÃÂwÂÃ¨ÂÃÂsÂÃÂÂ¤ÂiÂÂÂÂ³ÂÂªÂÂ¶ÂÂ¤ÂÂ½ÂÃÂj
	}
	::AdjustWindowRect(&rc,lStyle,bMenu);
}

///////////////////////////////////////////////////////////////////////////////

bool	CWindow::IsShowCursor(){
	//	ÂnÂ[ÂhÂEÂFÂAÂ}ÂEÂXÂJÂ[Â\ÂÂÂÃÂ\ÂÂ¦Â^ÂÃ±Â\ÂÂ¦ÂÃ°ÂÃ¦ÂÂ¾
	return m_bShowCursor;
}

void	CWindow::ShowCursor(bool bShow){
	//	ÂÂ±ÂÃªÂÃCreateWindowÂÂµÂÂ½ÂXÂÂÂbÂhÂÂªÂÂÂÂÂÂµÂÃÂÂ­ÂÃÂÃÂÃÂÃ§ÂÃÂÂ¢

	//	Â\ÂtÂgÂEÂFÂAÂ}ÂEÂXÂJÂ[Â\ÂÂÂÃÂÃ§ÂÃÂÂ­ÂÂ§ÂIÂt
	if (m_bUseMouseLayer) bShow = false;

	//	ÂnÂ[ÂhÂEÂFÂAÂ}ÂEÂXÂJÂ[Â\ÂÂÂÃÂ\ÂÂ¦Â^ÂÃ±Â\ÂÂ¦
	if (m_bShowCursor==bShow) return ;

	m_bShowCursor = bShow;
	::ShowCursor(bShow);
}

bool CWindow::IsWindowsClassName(const string& strClassName){
	return (strClassName=="BUTTON") || (strClassName=="COMBOBOX") ||
		(strClassName=="EDIT") || (strClassName=="LISTBOX") ||
		(strClassName=="MDICLIENT") || (strClassName=="SCROLLBAR") ||
		(strClassName=="STATIC");
}

///////////////////////////////////////////////////////////////////////////////
//
//	ÂÂÂbÂZÂ[ÂWÂÃÂgÂÂÂbÂv
//

//	ÂÃÂÃ­ÂÃWindowÂÃÂÃªÂÂ
LRESULT CALLBACK CWindow::gWndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam){

	CWindow* win = (CWindow*)::GetWindowLong(hWnd,GWL_USERDATA);	//	ÂÂ±ÂÂ±ÂÃCWindow*ÂÃ°ÂBÂÂµÂÃÂÂ¨ÂÂ¢ÂÂ½
	/**
		ÂÃÂÃÂÂµÂÂ½ÂEÂBÂÂÂhÂDÂÃÂÃÂÂµÂÃÂÂÂÃ§ÂÃªÂÃÂÂ«ÂÂ½ÂÂÂbÂZÂ[ÂWÂÃ°Â^ÂÂ«ÂÂ­dispatchÂÂ·ÂÃ©ÂÂ±ÂÃÂÂªÂÂ ÂÃ¨ÂÂ¤ÂÃ©..
	*/
	if (win!=NULL) {
		return win->m_HookPtrList.Dispatch(hWnd,uMsg,wParam,lParam,win->m_pWndProc);
	} else {
		return ::DefWindowProc(hWnd,uMsg,wParam,lParam);
	}
}

//	Â_ÂCÂAÂÂÂOÂÃÂÃªÂÂ
LRESULT CALLBACK CWindow::gDlgProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam){
	CWindow* win = (CWindow*)::GetWindowLong(hWnd,GWL_USERDATA);	//	ÂÂ±ÂÂ±ÂÃCWindow*ÂÃ°ÂBÂÂµÂÃÂÂ¨ÂÂ¢ÂÂ½
	if (win==NULL) {
		win = reinterpret_cast<CWindow*>(lParam);	//	ÂÂ¶ÂÃ¡ÂÂ ÂÂ±ÂÂ±ÂÂ©ÂH(CreateÂÂµÂÂ½ÂuÂÃÂH)
	}
	if (win!=NULL) {
		return win->m_HookPtrList.DispatchDlg(hWnd,uMsg,wParam,lParam,win->m_pWndProc);
	} else {
		//	Â_ÂCÂAÂÂÂOÂÃÂÃªÂÂÂÃÂADefWindowProcÂÃ°ÂÃÂÃÂoÂÂµÂÃÂÃÂÂ¢ÂÂ¯ÂÃÂÂ¢
		//	(ÂÃ ÂÂÂIÂÃÂÂÂÂÂÂ·ÂÃ©ÂÃ¦ÂÂ¤ÂÂ¾ÂÂÂ_ÂCÂAÂÂÂOprocedureÂÂª)
		return 0;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////////
//	ÂÃÂÂ¬ÂÂ»ÂÂ³ÂÃªÂÂ½ÂÂ©ÂÃ°ÂÂ»ÂÃ¨ÂÂµÂAÂÃÂÂÂÂ·ÂÃ©ÂÂ½ÂÃÂÃÂtÂbÂN
LRESULT	CWindow::WndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam){

	switch(uMsg){
	case WM_ACTIVATEAPP: {
		//	Windows2000ÂÃÂÃ§ÂÃÂKÂÂ¸ÂÃ²ÂÃ±ÂÃÂÂ­ÂÃ©ÂÂªÂA
		//	Win95/98ÂÃWM_SIZEÂÃSIZE_RESTOREDÂÂµÂÂ©ÂÃ²ÂÃ±ÂÃÂÂÂÃÂÂ¢ÂÂ±ÂÃÂÂ ÂÃ¨ÂH
		UINT bActive = wParam;
		if (bActive) {
			m_bMinimized = false;	//	ÂÃÂÃ­ÂÃ³ÂÃÂÃÂÂ ÂÃ©
		}
		break;
						 }
	case WM_SIZE : {
		switch (wParam) {
		case SIZE_MINIMIZED : m_bMinimized = true; break;		//	ÂÃÂÂ¬ÂÂ»ÂÂ³ÂÃªÂÃÂÂ¢ÂÃ©
		case SIZE_RESTORED	: m_bMinimized = false; break;		//	ÂÃÂÃ­ÂÃ³ÂÃÂÃÂÂ ÂÃ©
		//		ÂÂªÂÃÂÃ­ÂÃ³ÂÃÂÃÂÃÂÂµÂÂ½ÂÂ©ÂÃÂÂ¤ÂÂ©ÂÃÂAÂÂ±ÂÂ±ÂÃWM_ACTIVATEAPPÂÃÂÃÂÂ©ÂÃ©ÂÃÂÂ«
		}
		break;
						}


/*
	http://www.microsoft.com/JAPAN/support/kb/articles/JP214/6/55.HTM
	ÂÃ¦ÂÃ¨ÂF

	[SDK32] ÂAÂvÂÂÂPÂ[ÂVÂÂÂÂÂIÂÂ¹ÂÃ£ÂAÂ^ÂXÂN ÂoÂ[ÂÃÂÃ³ÂÂÂÃÂ{ÂbÂNÂXÂÂªÂcÂÃ© 
	ÂÃÂIÂXÂVÂÃº: 2001/01/30
	ÂÂ¶ÂÂÂÃÂÂ: JP214655	

ÂÂ»ÂÃ
ÂAÂvÂÂÂPÂ[ÂVÂÂÂÂÂÃÂIÂÂ¹ÂÃ£ÂAÂ^ÂXÂN ÂoÂ[ÂÂªÂXÂVÂÂ³ÂÃªÂÂ¸ÂAÂÃ³ÂÂÂÃÂ{ÂbÂNÂX (ÂEÂBÂÂÂhÂEÂÃÂ^ÂCÂgÂÂÂÃ¢ÂAÂCÂRÂÂÂÂªÂ\ÂÂ¦ÂÂ³ÂÃªÂÃÂÂ¢ÂÃÂÂ¢ÂÃ ÂÃ) ÂÂªÂcÂÂ³ÂÃªÂÂ½ÂÃÂÃÂÃÂÃÂÃ©ÂÂ±ÂÃÂÂªÂÂ ÂÃ¨ÂÃÂÂ·ÂBÂÂ»ÂÃÂÃ³ÂÂÂÃÂ{ÂbÂNÂXÂÃ°ÂNÂÂÂbÂNÂÂ·ÂÃ©ÂÃÂAÂÃÂÂ¦ÂÃÂÂµÂÃÂÂ¢ÂÃÂÂ·ÂB 

ÂÂ´ÂÃ¶
ÂÂ±ÂÃÂÃ¢ÂÃ¨ÂÃÂAÂAÂvÂÂÂPÂ[ÂVÂÂÂÂÂÂªÂ^ÂXÂN ÂoÂ[ÂÃÂÃÂÂ·ÂÃ©Â\ÂÂ¦ÂÃÂÃÂWÂÂ·ÂÃ©ÂEÂBÂÂÂhÂEÂÃÂXÂ^ÂCÂÂÂÃ°ÂÃÂXÂÂµÂÂ½ÂÂ½ÂÃÂÃÂNÂÂ±ÂÃ¨ÂÃÂÂ·ÂB 

ÂÃ¡ÂÂ¦ÂÃÂAÂEÂBÂÂÂhÂEÂÂª WS_EX_TOOLWINDOWS ÂXÂ^ÂCÂÂÂÃ°ÂwÂÃ¨ÂÂµÂÃÂAÂÂ»ÂÃÂgÂÂ£ÂXÂ^ÂCÂÂÂÃ°ÂÃÂXÂÂµÂAÂÂ»ÂÃÂEÂBÂÂÂhÂEÂÃ°ÂNÂÂÂ[ÂYÂÂ·ÂÃ©ÂOÂÃÂgÂÂ£ÂXÂ^ÂCÂÂÂÃÂÂÂZÂbÂgÂÃÂÂ¸ÂsÂÂµÂÂ½ÂÃªÂÂÂAÂ^ÂXÂN ÂoÂ[ÂÃÂÃ³ÂÂÂÃÂ{Â^ÂÂÂÂªÂ\ÂÂ¦ÂÂ³ÂÃªÂÃÂÂ·ÂB 

ÂÃ°ÂÂÂÃ»Â@
ÂÂ±ÂÃÂÂ®ÂÃ¬ÂÃÂÂÂÃ¶ÂÂµÂÂ½ÂÃªÂÂÂAÂÂ±ÂÃÂÃ¢ÂÃ¨ÂÃ°ÂÃ°ÂÂÂÂ·ÂÃ©ÂÃÂÃÂAÂEÂBÂÂÂhÂEÂÃÂXÂ^ÂCÂÂÂÃ°ÂÂÂZÂbÂgÂÂ·ÂÃ©ÂÂ©ÂAÂAÂvÂÂÂPÂ[ÂVÂÂÂÂÂÃÂÂÂCÂÂ ÂEÂBÂÂÂhÂEÂÃÂÂÂÂÂÃÂAWM_CLOSE ÂÃÂÂ½ÂÃ WM_DESTROY ÂÃÂÂÂÃÂASW_HIDE ÂÃ°ÂpÂÂÂÂÂ^ÂÃÂÂµÂÃ ShowWindow ÂÃ°ÂÃÂÃÂoÂÂµÂÃÂÂ·ÂB 
*/

/*
	ÂÃÂLÂB
		ÂÂ³ÂÂµÂÂ­ÂÃÂAÂEÂBÂÂÂhÂDÂÃÂÃ³ÂÃÂÃ°ÂWÂÂÂÃÂÃÂÂ³ÂÂ¸ÂÃÂIÂÂ¹ÂÂ·ÂÃ©ÂÃÂAÂÂ±ÂÃ
		ÂÂ»ÂÃÂÂªÂNÂÂ«ÂÃ©ÂÃ¦ÂÂ¤ÂÂ¾ÂB
		ÂÃÂÃÂÃ¨ÂA
		ÂEÂBÂÂÂhÂEÂÃÂÃ³ÂÃÂÃ°ÂÃÂÃ¥ÂÂ»ÂÂ©ÂÃ§ÂWÂÂÂÃÂÃÂÂ·ÂÃÂÂ«ÂÃÂAÂjÂÂÂ[ÂVÂÂÂÂÂÂª
		ÂÂ©ÂÃªÂÂµÂÂ¢ÂÂ½ÂÃSW_HIDEÂÃÂAhideÂÂµÂÃÂAÂÂ»ÂÃÂÂ ÂÃÂAÂEÂBÂÂÂhÂDÂXÂ^ÂCÂÂÂÃ°
		ÂmÂ[Â}ÂÂÂÃÂÃÂÂ¹ÂÃÂÃÂÂ¢ÂB
*/
	case WM_DESTROY : {
		::ShowWindow(m_hWnd, SW_HIDE);

///		ÂÂ«ÂÃ°ÂÃ¢ÂÃ©ÂÃÂAÂfÂbÂhÂÂÂbÂNÂÃÂÃÂÃ©ÂÂ±ÂÃÂÂªÂÂ ÂÃ©ÂÃ¦ÂÂ¤ÂÂ¾ÂDÂD
//		::SetWindowLong(m_hWnd, GWL_STYLE, WS_POPUP);
		//	ÂÂªÂÃÂÂ«ÂÃÂhÂÂÂCÂoÂÃÂoÂOÂÃÂÂ»ÂÃÂÂ¦ÂÃÂA
		//	ÂÂ±ÂÃªÂÃ°ÂÂ½ÂfÂÂ³ÂÂ¹ÂÃ©ÂÂ½ÂÃÂÃ(ShowWindowÂÂ©SetWindowPosÂÂ·ÂÃ©ÂKÂvÂÂªÂÂ ÂÃ©)
//		::SetWindowPos(hWnd, (HWND)HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
/*
		ÂÂªSetWindowPos(hWnd, (HWND)HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		ÂÂµÂÃÂÂ¢ÂÂ½ÂÃÂÂ«ÂAÂÂ±ÂÃªÂÃ°ÂÂÂZÂbÂgÂÂµÂÃÂÂ¢ÂÃÂAÂÃ¢ÂÃÂÃ¨ÂÂ¼ÂÂ³ÂÂµÂEÂBÂÂÂhÂDÂÂªÂ^ÂXÂNÂoÂ[ÂÃ£ÂÃÂcÂÃ©ÂÂ±ÂÃÂÂªÂÂ ÂÃ©ÂiÂoÂOÂÂ»ÂÃÂQÂj
*/

//		::ShowWindow(m_hWnd, SW_HIDE);
		}
		break;
	//	ÂÃ£ÂLÂÃÂÃ¢ÂÃ¨ÂÃÂÃÂÂ·ÂÃ©ÂÃÂÃ´ÂRÂ[Âh
	case WM_NCDESTROY : {
		//	ÂEÂBÂÂÂhÂDÂÂªÂÃ°ÂÃÂÂ³ÂÃªÂÂ½ÂÃÂÃÂAÂÂ±ÂÃÂtÂFÂCÂYÂÃÂAÂEÂBÂÂÂhÂDÂnÂÂÂhÂÂÂÃ°
		//	ÂÂÂZÂbÂgÂÂµÂÃÂÃ¢ÂÃ©ÂKÂvÂÂªÂÂ ÂÃ©ÂBÂiÂÂ»ÂÂ¤ÂÂµÂÃÂÂ¢ÂÃÂAÂÂ±ÂÃÂNÂÂÂXÂÃ
		//	ÂfÂXÂgÂÂÂNÂ^ÂÃÂAÂÃ±ÂdÂÃdestroyÂÂ³ÂÃªÂÃÂÂµÂÃÂÂ¤Âj
		m_hWnd = NULL;
		break;
		}
	} // end switch

	return 0;
}

//////////////////////////////////////////////////////////////////////////////

void	CDialogHelper::hook(IWindow*pWin){
	if (pWin!=NULL){
		pWin->GetHookList()->Add(GetPooler());
	}
}

void	CDialogHelper::unhook(IWindow*pWin){
	if (pWin!=NULL){
		pWin->GetHookList()->Del(GetPooler());
	}
}

string	CDialogHelper::GetText(int nEditControlID){
	HWND hWnd = GetHWnd();
	if (hWnd==NULL) return "";
	smart_ptr<BYTE> pBuf;
	int nLength = 16;	//	16ÂÂ¶ÂÂÂÂªÂÂ©ÂÃ§ÂJÂn
	while (true) {
		pBuf.AddArray(nLength);
		int nResultLength = ::GetDlgItemText(hWnd,nEditControlID,(LPTSTR)pBuf.get(),pBuf.size());
		if (nResultLength+1 == nLength){
			//	ÂoÂbÂtÂ@ÂÂ·ÂÂªÂÂ«ÂÃ¨ÂÃÂÂ¢ÂÃÂvÂÃ­ÂÃªÂÃ©
			nLength <<=1;
		} else {
			break;
		}
	}
	return string((LPCSTR)pBuf.get());
}

///	ÂXÂ^ÂeÂBÂbÂNÂeÂLÂXÂgÂRÂÂÂgÂÂÂ[ÂÂetc..ÂÃÂÃÂÂµÂÃÂAÂÂ»ÂÂ±ÂÃÂeÂLÂXÂgÂÂ¶ÂÂÂÃ±ÂÃ°ÂÃÂÃ¨ÂÂ·ÂÃ©
LRESULT	CDialogHelper::SetText(int nEditControlID,const string& str){
	HWND hWnd = GetHWnd();
	if (hWnd==NULL) return 1;
	return ::SetDlgItemText(hWnd,nEditControlID,str.c_str())?0:1;
}

LRESULT CDialogHelper::SetCheck(int nEditControlID,int nCheck){
	HWND hWnd = GetHWnd();
	if (hWnd==NULL) return 1;
	HWND hDlgItem = ::GetDlgItem(hWnd,nEditControlID);
	if (hDlgItem==NULL) return 2;
	switch (nCheck){
	case 0 : nCheck = BST_UNCHECKED; break;
	case 1 : nCheck = BST_CHECKED; break;
	case 2 : nCheck = BST_INDETERMINATE; break;
	}
	return ::SendMessage(hDlgItem , BM_SETCHECK, nCheck , 0L)?0:3;
}

///	Â{Â^ÂÂÂÃ³ÂÃÂÃÂÃ¦ÂÂ¾
LRESULT	CDialogHelper::GetCheck(int nEditControlID){
/*
	-1:ÂÂ»ÂÃÂ{Â^ÂÂÂÃÂÂ¶ÂÃÂÂµÂÃÂÂ¢
	0:Â{Â^ÂÂÂÃÂ`ÂFÂbÂNÂÃÂIÂtÂÃÂÃÂÃÂÃÂÂ¢ÂÃÂÂ·ÂB 
	1:Â{Â^ÂÂÂÃÂ`ÂFÂbÂNÂÃÂIÂÂÂÃÂÃÂÃÂÃÂÂ¢ÂÃÂÂ·ÂB 
	2:Â{Â^ÂÂÂÃÂOÂÂÂCÂ\ÂÂ¦ÂiÂsÂmÂÃ¨ÂjÂÃÂÃ³ÂÃÂÃÂÂ·ÂBÂ{Â^ÂÂÂÂªÂABS_3STATE ÂXÂ^ÂCÂÂÂÃÂÂ½ÂÃ BS_AUTO3STATE ÂXÂ^ÂCÂÂÂÃ°ÂÂÂÃÂÃÂÂ«ÂÃÂÂ¾ÂÂ¯ÂKÂpÂÂ³ÂÃªÂÃÂÂ·ÂB 
*/
	HWND hWnd = GetHWnd();
	if (hWnd==NULL) return -1;
//	HWND hDlgItem = ::GetDlgItem(hWnd,nEditControlID);
//	if (hDlgItem==NULL) return -1;
	UINT n = ::IsDlgButtonChecked(hWnd,nEditControlID);
	switch(n){
	case BST_UNCHECKED : return 0; // Â{Â^ÂÂÂÃÂ`ÂFÂbÂNÂÃÂIÂtÂÃÂÃÂÃÂÃÂÂ¢ÂÃÂÂ·ÂB
	case BST_CHECKED : return 1; //Â{Â^ÂÂÂÃÂ`ÂFÂbÂNÂÃÂIÂÂÂÃÂÃÂÃÂÃÂÂ¢ÂÃÂÂ·ÂB 
	case BST_INDETERMINATE : return 2; // Â{Â^ÂÂÂÃÂOÂÂÂCÂ\ÂÂ¦ÂiÂsÂmÂÃ¨ÂjÂÃÂÃ³ÂÃÂÃÂÂ·ÂBÂ{Â^ÂÂÂÂªÂABS_3STATE ÂXÂ^ÂCÂÂÂÃÂÂ½ÂÃ BS_AUTO3STATE ÂXÂ^ÂCÂÂÂÃ°ÂÂÂÃÂÃÂÂ«ÂÃÂÂ¾ÂÂ¯ÂKÂpÂÂ³ÂÃªÂÃÂÂ·ÂB 
	}
	return -1;	//???
}

///////////////////////////////////////////////////////////////////////////

void	CMsgDlg::Out(const string& caption,const string& message){
/*
	// ÂtÂÂÂXÂNÂÂÂ[ÂÂÂÃÂtÂÂÂbÂvÂgÂÃÂÃÂÃ©ÂÂ©ÂÃ ÂmÂÃªÂÃÂÂ¢ÂÃÂÃ
#ifdef USE_DirectDraw
	CDirectDraw* lpDraw = CAppManager::GetMyDirectDraw();
	if (lpDraw!=NULL){
		lpDraw->FlipToGDISurface();
	}
#endif
	CWindow* lpWindow = CAppManager::GetMyApp()->GetMyWindow();
	bool bShowCursor;
	if (lpWindow!=NULL) {
		bShowCursor = lpWindow->IsShowCursor();
		lpWindow->ShowCursor(true);
	}
*/
	::MessageBox(CAppManager::GetHWnd(),message.c_str(),caption.c_str()
		,MB_OK|MB_SYSTEMMODAL|MB_SETFOREGROUND);
/*
	if (lpWindow!=NULL) {
		lpWindow->ShowCursor(bShowCursor);
	}
*/
}

} // end of namespace Window
} // end of namespace yaneuraoGameSDK3rd
