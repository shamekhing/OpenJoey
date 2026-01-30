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

bool CWindow::g_bFullScreen		= false; // N®¼ãÍEBhD[hÅµåI
CCriticalSection CWindow::m_cs;

//////////////////////////////////////////////////////////////////////////////

CWindow::CWindow(){
	m_dwFillColor	=	0;			//	N®ªfBtHg
	m_hWnd			=	NULL;
	m_bShowCursor	=	true;		//	fBtHgÅ}EXJ[\ð\¦
	m_bUseMouseLayer=	false;		//	\tgEFA}EXJ[\
	m_bUseMenu		=	false;		//	j[Ì éâÈµâÍí©çñOOG
	m_bMinimized	=	false;		//	Å¬»³êÄ¢é©H
	m_bResized		=	false;		//	ChangeWindowStyleÌSetSizeÌtOB
	m_pWndProc		=	NULL;		//	hookµ½WndProc
}

CWindow::~CWindow(){
	GetHookList()->Del(this);		//	¼ÚtbNðð·é
/*
	if (m_hWnd!=NULL) {
		::DestroyWindow(m_hWnd);
	//	ª±ñÈ­øÈû@ÅDestroy·éÌÍA¨©ßµ©ËéOOG
	//	{ACAppBase©çg¤ÌÈçÎA±ÌfXgN^ÅÍA·ÅÉ
	//	WindowÍðÌ³ê½ ÆÌÍ¸¾ª...
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
	m_bFullScreen = g_bFullScreen;	//	»ÝÌæÊ[hðàIÉÛ
	m_opt	= opt;	//	Rs[µÄ¨­

	m_hWnd = NULL;

	LONG lChild = 0;

	//	Ü½Cªü¢½çT|[g
	if (m_opt.dialog!=NULL){
	//	dialogìéçµ¢ÅH
		m_hWnd = ::CreateDialogParam(hInst,m_opt.dialog, hParent, (DLGPROC)gDlgProc,
			reinterpret_cast<LPARAM>(this));
			//	ªthisªLPARAMÉüÁÄâÁÄ­éI
		goto window_setting;
	}

    // Ü¾o^³êÄ¢È¢EBhENX¼ÅA©ÂWindowsWNX
    // ÈOÌ¼OÌêÍRegisterClassEx()µÄâé
    WNDCLASSEX wndclass;
    if( !::GetClassInfoEx(hInst, opt.classname.c_str(), &wndclass) &&
        !IsWindowsClassName(opt.classname)) {

        // ¨«ÜèÌEChENX¶¬
		wndclass.cbSize		= sizeof(WNDCLASSEX);
		wndclass.style		= 0;	//	_uNbN´m·éÈç¨CS_DBLCLKS;
		wndclass.lpfnWndProc= gWndProc;
		wndclass.cbClsExtra	= 0;
		wndclass.cbWndExtra	= 0;
		wndclass.hInstance	= hInst;

		// Æè ¦¸A"MAIN"ÌACR\¦
		wndclass.hIcon		= LoadIcon(hInst,"MAIN");
		wndclass.hIconSm	= LoadIcon(hInst,"MAIN");

		wndclass.hCursor	= LoadCursor(NULL,IDC_ARROW);
		// Æè ¦¸A}EXJ[\àpÓisvÈçÎA ÆÅÁ·×µIj

		//	Æè ¦¸AN®¼ãÍA©ÉµÆ±ÁÆc
		if (m_dwFillColor==0) {
			wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		} else {
			wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		}

		//	MENUà¼ÅÇÝñÅÝé
		wndclass.lpszMenuName  = "IDR_MENU";
		//	±Ì\[Xª{É¶Ý·éÌ©ÍAÀsÜÅí©çÈ¢ÌÅ
		//	³mÈEBhDÌAhWXgªEBhD¶¬ãÅµ©oÈ¢

		{
			HRSRC hrsrc;
			hrsrc = ::FindResource(NULL,"IDR_MENU",RT_MENU);
			if (hrsrc!=NULL) { m_bUseMenu = true; }	//	j[ èâªñÅOOG
		}

		wndclass.lpszClassName = opt.classname.c_str();

		// ÈñÅ¸sµÄñ¾ëËH
		if (!::RegisterClassEx(&wndclass)) {
			Err.Out("CWindow::CreateÅRegisterClassEx¸s");
			return 1;
		}
	}

	if (hParent!=NULL) {
		lChild	= WS_CHILDWINDOW;
	}

	if (m_bFullScreen){
		//	EChEÌ¶¬itXN[j
		//	±ÌÆ«ÍAæÊÌX^CIvVÍ³·é

		int sx,sy;
		GetScreenSize(sx,sy);
		m_hWnd = ::CreateWindow(opt.classname.c_str(),opt.caption.c_str(),
						WS_POPUP|WS_VISIBLE	 /* |WS_SYSMENU|
						WS_MAXIMIZEBOX|WS_MINIMIZEBOX */ ,
						0, 0, sx, sy,
						hParent, NULL, hInst, NULL );
	} else {
		// EBhD«Eðl¶ÉüêÈ­ÄÍÈçÈ¢
		// \¦ÍÍª640*480ÈçÎA¶¬·×«WindowTCYÍA»êÈãÅ é
		RECT r;
		InnerAdjustWindow(r,opt);
		m_hWnd = ::CreateWindow(opt.classname.c_str(),opt.caption.c_str(),
				// WS_EX_TOPMOSTðwèµ½¢ñ¾¯ÇÈ[
						lChild | opt.style,
						CW_USEDEFAULT,
						CW_USEDEFAULT,
						r.right-r.left,r.bottom-r.top,
						hParent,NULL,hInst,NULL);
	}

window_setting:;
	if (m_hWnd==NULL){
		Err.Out("CWindow::CreateÅCreateWindowÉ¸sB");
		return 1;
	}

	//	R[obN³ê½Æ«ÉA»ê¼êÌWindowClassÉdispatchoéæ¤É
	//	GWL_USERDATAÉthisðBµÄ¨­B
	::SetWindowLong(m_hWnd,GWL_USERDATA,(LONG)this);

	//	æÊÌZ^[É¶¬·éæ¤ÉC³OO
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

	//	WÌWindowClassÈçÎAR[obNÖÆµÄA±êð
	//	«ñÅâéB
	if (IsWindowsClassName(opt.classname)){
		m_pWndProc = (WNDPROC)::SetWindowLong(m_hWnd,GWL_WNDPROC,(LONG)gWndProc);
		//	±¤¢¤hookÍAillegal©...
	} else {
		m_pWndProc = NULL;
	}

	//	©ªÌEBhDÈÌÅ¼ÚtbNÅ«é
	//	iÆ¢¤©CAppManagerÉ¢o^ÌiKÈÌÅ¼ÚtbNµ©È¢j
	GetHookList()->Add(this);			//	tbNðJn·é

	return 0;
}

void CWindow::GetScreenSize(int &x,int &y){
	x = ::GetSystemMetrics(SM_CXSCREEN);
	y = ::GetSystemMetrics(SM_CYSCREEN);
}

LRESULT CWindow::SetWindowPos(int x,int y){
	if (m_hWnd==NULL) return 0;
	//	EBhEª¶¬³êÄ¢½çÚ®³¹È­ÄÍÈçÈ¢i½¾µEBhD[hj
	if (!m_bFullScreen){
		//	±±ÅgbvI[_[ÉµÄ¨¢ÄàâèÈ¢Ì¾ë¤...
		return !::SetWindowPos(m_hWnd,HWND_TOP,x,y,NULL,NULL,SWP_NOSIZE|SWP_NOZORDER);
	}
	return 0;
}

void CWindow::SetSize(int sx,int sy){
	if (m_opt.size_x != sx || m_opt.size_y != sy){
		m_opt.size_x = sx;
		m_opt.size_y = sy;
		m_bResized	= true;	//	ÏX³ê½ÅtOOOG
	}
}

LRESULT CWindow::Resize(int sx,int sy){
	m_opt.size_x = sx;
	m_opt.size_y = sy;

	if (m_hWnd==NULL) return 0;

	if (!m_bFullScreen){
		//	[hÈÌÅ...
		if ((m_opt.size_x!=0) && (m_opt.size_y!=0)){
			::ShowWindow(m_hWnd, SW_SHOW);	//	OÌ½ßÉéOOG
		}
		RECT r;
		InnerAdjustWindow(r,m_opt);
		return !::SetWindowPos(m_hWnd,NULL,0,0,r.right-r.left,r.bottom-r.top,SWP_NOMOVE|SWP_NOZORDER);
	} else {
		//	tXN[ÈçÎATCYÍñT|[g
	}

	return 0;
}

void	CWindow::UseMouseLayer(bool bUse){
	if (bUse==m_bUseMouseLayer) return ;
	m_bUseMouseLayer = bUse;
	if (bUse) {
		//	g¤ÈçÎ³ðÉÁ·
		ShowCursor(false);
	} else {
		//	gíÈ¢ÌÅ êÎAEBhD[h(or DIBDraw)ÈçÎ}EXJ[\
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
	//	»ÝÌæÊ[hÆAEBhEX^Cªv·êÎÏXÌKvÈµ
	if (m_bFullScreen==g_bFullScreen && !m_bResized) return ;

	m_bFullScreen	= g_bFullScreen;
	m_bResized		= false;

	// ÙÈéêÍEBhDX^Cð»ÝÌ[hÉí¹ÄÏX
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

		//	±±ÌASetWindowLong() Å»ÝÌóÔ(Åå»AÅ¬»AÊí)
		//	ðÝè·é(êÞ '02/03/16)
		UINT nState = 0;// EChEÌóÔ
		if(::IsIconic(m_hWnd))nState = WS_MINIMIZE;
		else if(::IsZoomed(m_hWnd))nState = WS_MAXIMIZE;
		::SetWindowLong(m_hWnd,GWL_STYLE,m_opt.style | nState );
		//	WS_CAPTIONÁ·('01/04/24)@âÁÏA±êÍ[U[ªwèµÈ«á_

		::SetMenu(m_hWnd,GetMenu(m_hWnd)); // j[ÌæÌÄ`æ
		::ShowWindow(m_hWnd,SW_SHOW);
		CIMEWrapper::GetObj()->Show();
	} else {
		::SetWindowLong(m_hWnd, GWL_STYLE, WS_POPUP);
		::SetMenu(m_hWnd,GetMenu(m_hWnd)); // j[ÌæÌÄ`æ
		::ShowWindow(m_hWnd,SW_SHOW);
		CIMEWrapper::GetObj()->Hide();
	}
}

//	EBhDTCYÌadjust
//	Win32::AdjustWindowÌbug-fixp
//		(C) Tia-Deen & yaneurao '00/08/01,'00/11/05
void	CWindow::InnerAdjustWindow(RECT&rc,CWindowOption&opt){
	::SetRect(&rc,0,0,opt.size_x,opt.size_y);
	LONG	lStyle = opt.style;
/*	//	âÁÏA±ÌR[hÍuserªwè·×« '01/11/14
	if (lStyle & WS_POPUP) {
		lStyle |= WS_CAPTION;
	}
*/
	bool	bMenu = false;
	if (lStyle & WS_SYSMENU){
		//	m_hWndªNULLÌóÔÅGetMenuðÄÑo·ÌÍs³H
		if (m_bUseMenu || (m_hWnd!=NULL && ::GetMenu(m_hWnd)!=NULL)){
			bMenu = true;
		} else {
			bMenu = false;
			lStyle &= ~WS_SYSMENU;	//	SYSMENUtOðO·
		}
	//	lStyle |= WS_CAPTION;	//	µ©µALvVÌwèÍsÈ¤i³ª¶¤½ßj
	}
	::AdjustWindowRect(&rc,lStyle,bMenu);
}

///////////////////////////////////////////////////////////////////////////////

bool	CWindow::IsShowCursor(){
	//	n[hEFA}EXJ[\Ì\¦^ñ\¦ðæ¾
	return m_bShowCursor;
}

void	CWindow::ShowCursor(bool bShow){
	//	±êÍCreateWindowµ½XbhªµÈ­ÄÍÈçÈ¢

	//	\tgEFA}EXJ[\ÈçÎ­§It
	if (m_bUseMouseLayer) bShow = false;

	//	n[hEFA}EXJ[\Ì\¦^ñ\¦
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
//	bZ[WÌgbv
//

//	ÊíÌWindowÌê
LRESULT CALLBACK CWindow::gWndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam){

	CWindow* win = (CWindow*)::GetWindowLong(hWnd,GWL_USERDATA);	//	±±ÉCWindow*ðBµÄ¨¢½
	/**
		ÁÅµ½EBhDÉÎµÄçêÄ«½bZ[Wð^«­dispatch·é±Æª è¤é..
	*/
	if (win!=NULL) {
		return win->m_HookPtrList.Dispatch(hWnd,uMsg,wParam,lParam,win->m_pWndProc);
	} else {
		return ::DefWindowProc(hWnd,uMsg,wParam,lParam);
	}
}

//	_CAOÌê
LRESULT CALLBACK CWindow::gDlgProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam){
	CWindow* win = (CWindow*)::GetWindowLong(hWnd,GWL_USERDATA);	//	±±ÉCWindow*ðBµÄ¨¢½
	if (win==NULL) {
		win = reinterpret_cast<CWindow*>(lParam);	//	¶á ±±©H(Createµ½uÔH)
	}
	if (win!=NULL) {
		return win->m_HookPtrList.DispatchDlg(hWnd,uMsg,wParam,lParam,win->m_pWndProc);
	} else {
		//	_CAOÌêÍADefWindowProcðÄÑoµÄÍ¢¯È¢
		//	(àIÉ·éæ¤¾_CAOprocedureª)
		return 0;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////////
//	Å¬»³ê½©ð»èµAÛ·é½ßÌtbN
LRESULT	CWindow::WndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam){

	switch(uMsg){
	case WM_ACTIVATEAPP: {
		//	Windows2000ÈçÎK¸òñÅ­éªA
		//	Win95/98ÍWM_SIZEÌSIZE_RESTOREDµ©òñÅÈ¢±Æ èH
		UINT bActive = wParam;
		if (bActive) {
			m_bMinimized = false;	//	ÊíóÔÅ é
		}
		break;
						 }
	case WM_SIZE : {
		switch (wParam) {
		case SIZE_MINIMIZED : m_bMinimized = true; break;		//	Å¬»³êÄ¢é
		case SIZE_RESTORED	: m_bMinimized = false; break;		//	ÊíóÔÅ é
		//		ªÊíóÔÉßµ½©Ç¤©ÍA±±ÆWM_ACTIVATEAPPÆÅ©é×«
		}
		break;
						}


/*
	http://www.microsoft.com/JAPAN/support/kb/articles/JP214/6/55.HTM
	æèF

	[SDK32] AvP[VI¹ãA^XN o[ÉóÌ{bNXªcé 
	ÅIXVú: 2001/01/30
	¶Ô: JP214655	

»Û
AvP[VÌI¹ãA^XN o[ªXV³ê¸AóÌ{bNX (EBhEÌ^CgâACRª\¦³êÄ¢È¢àÌ) ªc³ê½ÜÜÉÈé±Æª èÜ·B»ÌóÌ{bNXðNbN·éÆAÁ¦ÄµÜ¢Ü·B 

´ö
±ÌâèÍAAvP[Vª^XN o[ÉÎ·é\¦ÉÖW·éEBhEÌX^CðÏXµ½½ßÉN±èÜ·B 

á¦ÎAEBhEª WS_EX_TOOLWINDOWS X^CðwèµÄA»Ìg£X^CðÏXµA»ÌEBhEðN[Y·éOÉg£X^CÌZbgÉ¸sµ½êA^XN o[ÉóÌ{^ª\¦³êÜ·B 

ðû@
±Ì®ìÉöµ½êA±Ìâèðð·éÉÍAEBhEÌX^CðZbg·é©AAvP[VÌC EBhEÌÅAWM_CLOSE Ü½Í WM_DESTROY ÌÅASW_HIDE ðp^ÉµÄ ShowWindow ðÄÑoµÜ·B 
*/

/*
	ÇLB
		³µ­ÍAEBhDÌóÔðWÉß³¸ÉI¹·éÆA±Ì
		»ÛªN«éæ¤¾B
		ÂÜèA
		EBhEÌóÔðÅå»©çWÉß·Æ«ÌAj[Vª
		©êµ¢½ßSW_HIDEÅAhideµÄA»Ì ÆAEBhDX^Cð
		m[}Éß¹ÎÇ¢B
*/
	case WM_DESTROY : {
		::ShowWindow(m_hWnd, SW_HIDE);

///		«ðâéÆAfbhbNÉ×é±Æª éæ¤¾DD
//		::SetWindowLong(m_hWnd, GWL_STYLE, WS_POPUP);
		//	ªÅ«ÌhCoÌoOÉ»È¦ÄA
		//	±êð½f³¹é½ßÉ(ShowWindow©SetWindowPos·éKvª é)
//		::SetWindowPos(hWnd, (HWND)HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
/*
		ªSetWindowPos(hWnd, (HWND)HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		µÄ¢½Æ«A±êðZbgµÈ¢ÆAâÍè¼³µEBhDª^XNo[ãÉcé±Æª éioO»ÌQj
*/

//		::ShowWindow(m_hWnd, SW_HIDE);
		}
		break;
	//	ãLÌâèÉÎ·éÎôR[h
	case WM_NCDESTROY : {
		//	EBhDªðÌ³ê½ÌÅA±ÌtFCYÅAEBhDnhð
		//	ZbgµÄâéKvª éBi»¤µÈ¢ÆA±ÌNXÌ
		//	fXgN^ÅAñdÉdestroy³êÄµÜ¤j
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
	int nLength = 16;	//	16¶ª©çJn
	while (true) {
		pBuf.AddArray(nLength);
		int nResultLength = ::GetDlgItemText(hWnd,nEditControlID,(LPTSTR)pBuf.get(),pBuf.size());
		if (nResultLength+1 == nLength){
			//	obt@·ª«èÈ¢Ævíêé
			nLength <<=1;
		} else {
			break;
		}
	}
	return string((LPCSTR)pBuf.get());
}

///	X^eBbNeLXgRg[etc..ÉÎµÄA»±ÉeLXg¶ñðÝè·é
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

///	{^óÔÌæ¾
LRESULT	CDialogHelper::GetCheck(int nEditControlID){
/*
	-1:»Ì{^Í¶ÝµÈ¢
	0:{^Ì`FbNÍItÉÈÁÄ¢Ü·B 
	1:{^Ì`FbNÍIÉÈÁÄ¢Ü·B 
	2:{^ÍOC\¦ismèjÌóÔÅ·B{^ªABS_3STATE X^CÜ½Í BS_AUTO3STATE X^CðÂÆ«É¾¯Kp³êÜ·B 
*/
	HWND hWnd = GetHWnd();
	if (hWnd==NULL) return -1;
//	HWND hDlgItem = ::GetDlgItem(hWnd,nEditControlID);
//	if (hDlgItem==NULL) return -1;
	UINT n = ::IsDlgButtonChecked(hWnd,nEditControlID);
	switch(n){
	case BST_UNCHECKED : return 0; // {^Ì`FbNÍItÉÈÁÄ¢Ü·B
	case BST_CHECKED : return 1; //{^Ì`FbNÍIÉÈÁÄ¢Ü·B 
	case BST_INDETERMINATE : return 2; // {^ÍOC\¦ismèjÌóÔÅ·B{^ªABS_3STATE X^CÜ½Í BS_AUTO3STATE X^CðÂÆ«É¾¯Kp³êÜ·B 
	}
	return -1;	//???
}

///////////////////////////////////////////////////////////////////////////

void	CMsgDlg::Out(const string& caption,const string& message){
/*
	// tXN[ÅtbvgÁÄé©àmêÈ¢ÌÅ
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
