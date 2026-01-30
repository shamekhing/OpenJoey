#include "stdafx.h"
#include "yaneMouse.h"
#include "../AppFrame/yaneAppManager.h"
// Needed for GET_WHEEL_DELTA_WPARAM macro
#include <Windows.h> 

namespace yaneuraoGameSDK3rd {
namespace Input {

CMouse::CMouse(){
    m_bOutScreenInput = false;
    m_bRB = false;
    m_bLB = false;
    m_bHistRB = false;
    m_bHistLB = false;
#ifdef OPENJOEY_ENGINE_FIXES
    m_nWheelDelta = 0; // Initialize mouse wheel delta
#endif
    CAppManager::Hook(this);        //  Hook Window Messages
}

CMouse::~CMouse(){
    CAppManager::Unhook(this);        //  Unhook Window Messages
}

//////////////////////////////////////////////////////////////////////////////

LRESULT    CMouse::GetXY(int &x,int &y) const {
    POINT point,point2 = { 0,0 };
    ::GetCursorPos(&point);

    if (CAppManager::IsFullScreen()) {
    //  In full screen mode, due to the influence of invisible window captions, etc.,
    //  the position might become strange, so it's fine to return it as is.
        x = point.x;
        y = point.y;
    } else {
        HWND hWnd = CAppManager::GetHWnd();
        ::ClientToScreen(hWnd,&point2);
        x = point.x - point2.x;
        y = point.y - point2.y;

        if (!m_bOutScreenInput) {
            //  If outside the window bounds, reset the button state.
            ::ScreenToClient(hWnd,&point);// Convert to client coordinates
            RECT rt;
            ::GetClientRect(hWnd,&rt);// Get client area
            // If outside the bounds, initialize mouse state.
            if(point.x<0 || rt.right<=point.x || point.y<0 || rt.bottom<=point.y) {
                //  const_cast fake
                *const_cast<bool*>(&m_bLB) = false;
                *const_cast<bool*>(&m_bRB) = false;
            }
        }
        
    }
    return 0;
}

LRESULT CMouse::SetXY(int x,int y) {
    POINT point,point2 = { 0,0 };

    if (CAppManager::IsFullScreen()) {
        point.x=x;
        point.y=y;
        ::SetCursorPos(point.x,point.y);
    } else {
        ::ClientToScreen(CAppManager::GetHWnd(),&point2);
        point.x= x + point2.x;
        point.y= y + point2.y;
        ::SetCursorPos(point.x,point.y);
    }
    return 0;
}

LRESULT CMouse::GetInfo(int &x,int &y,int &b) const {    // Returns mouse position and button state
    CMouse::GetXY(x,y);    //    A virtual function call here would be useless

    b = 0;
    if (m_bRB) b++;        //    Button info. If right-clicked, 1
    if (m_bLB) b+=2;    //    Button info. If left-clicked, 2

    return 0;
}

bool CMouse::RButton() const {
    return m_bRB;
}

bool CMouse::LButton() const {
    return m_bLB;
}

void CMouse::SetOutScreenInput(bool bEnable) { m_bOutScreenInput=bEnable;}

#ifdef OPENJOEY_ENGINE_FIXES
int CMouse::GetWheelDelta() const {
    return m_nWheelDelta;
}

void CMouse::ResetWheelDelta() {
    m_nWheelDelta = 0;
}
#endif

//////////////////////////////////////////////////////////////////////////////

void    CMouse::GetButton(bool&bL,bool&bR){
    bL = !m_bHistLB && m_bLB;
    bR = !m_bHistRB && m_bRB;
    m_bHistLB = m_bLB;
    m_bHistRB = m_bRB;
}

void    CMouse::ResetButton(){
    m_bHistRB = false;
    m_bHistLB = false;
}

//////////////////////////////////////////////////////////////////////////////
// If messages are not hooked, button states cannot be known...
LRESULT CMouse::WndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam){ // Message callback
    switch(uMsg){
    /*
        What to do when the button is held down and the mouse moves out of the screen?
        (WM_MOUSEMOVE messages don't arrive when outside the screen..)
        ? Do WM_NCBUTTONUP messages arrive? ? Seems they don't (Tears)
    */

/*
    //    ? It seems this message does not necessarily always fly!
    case WM_NCHITTEST:
    {
        POINT pos;
        RECT rt;
        pos.x = LOWORD(lParam);
        pos.y = HIWORD(lParam);
        ::ScreenToClient(hWnd,&pos);// Convert to client coordinates
        GetClientRect(hWnd,&rt);// Get client area
        // If outside the bounds, initialize mouse state
        if(pos.x<0 || rt.right<=pos.x || pos.y<0 || rt.bottom<=pos.y) {
            m_bLB = false;
            m_bRB = false;
        }
        break;
    }
*/

    case WM_LBUTTONDOWN:
//    case WM_NCLBUTTONDOWN:
        m_bLB = true; break;
    case WM_LBUTTONUP:
//    case WM_NCLBUTTONUP:
        m_bLB = /* m_bLDC = */ false; break;
    case WM_RBUTTONDOWN:
//    case WM_NCRBUTTONDOWN:
        m_bRB = true; break;
    case WM_RBUTTONUP:
//    case WM_NCRBUTTONUP:
        m_bRB = /* m_bRDC = */ false; break;
/* //    This message doesn't fly
    case WM_LBUTTONDBLCLK:
//    case WM_NCLBUTTONDBLCLK:
        m_bLDC= true; break;
    case WM_RBUTTONDBLCLK:
//    case WM_NCRBUTTONDBLCLK:
        m_bRDC= true; break;
*/
    case WM_MOUSEMOVE:
//    case WM_NCMOUSEMOVE:    //    For when the mouse moves out of the screen
        m_bLB = (wParam & MK_LBUTTON)!=0;
        m_bRB = (wParam & MK_RBUTTON)!=0;
        break;

#ifdef OPENJOEY_ENGINE_FIXES
    case WM_MOUSEWHEEL:
        // Accumulate the wheel delta. WHEEL_DELTA is usually 120 (per notch).
        m_nWheelDelta += GET_WHEEL_DELTA_WPARAM(wParam);
        break;
#endif
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////////

CFixMouse::CFixMouse(){
    m_bRBN = false;
    m_bLBN = false;
    m_bHistRB = false;
    m_bHistLB = false;
    m_nRLBN = 0;
    m_nX = m_nY = 0;
    m_nGuardTime = 2;
#ifdef OPENJOEY_ENGINE_FIXES
    m_nFixedWheelDelta = 0; // Initialize fixed wheel delta
#endif
}

CFixMouse::~CFixMouse(){
}

LRESULT    CFixMouse::Flush(){
    if (m_nGuardTime!=0) m_nGuardTime--;

    m_bHistLB = m_bLBN;
    m_bHistRB = m_bRBN;

    if (GetMouse()->GetInfo(m_nX,m_nY,m_nRLBN)!=0) {
        m_bRBN = m_bLBN = false;
#ifdef OPENJOEY_ENGINE_FIXES
        m_nFixedWheelDelta = 0; // Reset wheel delta on error/issue
#endif
        return 1;
    }
    m_bRBN = (m_nRLBN&1)!=0;
    m_bLBN = (m_nRLBN&2)!=0;

#ifdef OPENJOEY_ENGINE_FIXES
    // Get accumulated delta from CMouse and reset CMouse's delta
    m_nFixedWheelDelta = GetMouse()->GetWheelDelta();
    GetMouse()->ResetWheelDelta();

    // Scale mouse from window client coords to logical backbuffer coords (640x480)
    HWND hWnd = CAppManager::GetHWnd();
    if (hWnd) {
        RECT rt;
        ::GetClientRect(hWnd, &rt);
        int clientW = rt.right - rt.left;
        int clientH = rt.bottom - rt.top;
        const int logW = 800, logH = 600;
        if (clientW > 0 && clientH > 0 && (clientW != logW || clientH != logH)) {
            m_nX = (int)((long)m_nX * logW / clientW);
            m_nY = (int)((long)m_nY * logH / clientH);
            if (m_nX < 0) m_nX = 0; else if (m_nX >= logW) m_nX = logW - 1;
            if (m_nY < 0) m_nY = 0; else if (m_nY >= logH) m_nY = logH - 1;
        }
    }
#endif

    return 0;
}

LRESULT CFixMouse::GetXY(int &x,int &y)const{
    x = m_nX;
    y = m_nY;
    return 0;
}

LRESULT CFixMouse::SetXY(int x,int y){
                    //    Move mouse to specified position (in client coordinates)
    m_nX = x; m_nY = y;    //    Update info immediately
    return GetMouse()->SetXY(x,y);
}

bool    CFixMouse::RButton()const{
    return m_bRBN;
}

bool    CFixMouse::LButton()const{
    return m_bLBN;
}

LRESULT CFixMouse::GetInfo(int &x,int &y,int &b)const{
    x = m_nX;
    y = m_nY;
    b = m_nRLBN;
    return 0;
}

#ifdef OPENJOEY_ENGINE_FIXES
int CFixMouse::GetWheelDelta() const {
    return m_nFixedWheelDelta;
}

void CFixMouse::ResetWheelDelta() {
    m_nFixedWheelDelta = 0; // Reset fixed delta if needed manually (though Flush usually handles it)
}

bool CFixMouse::IsWheelUp() const {
    // Check if there's a positive delta (scroll up) and not in guard time
    return m_nFixedWheelDelta > 0 && (m_nGuardTime == 0);
}

bool CFixMouse::IsWheelDown() const {
    // Check if there's a negative delta (scroll down) and not in guard time
    return m_nFixedWheelDelta < 0 && (m_nGuardTime == 0);
}
#endif

/////////////////////////////////////////////////////////////////////////////
//    Was pressed since last Flush?

void    CFixMouse::GetButton(bool&bL,bool&bR){
    bR = !m_bHistRB && m_bRBN;
    bL = !m_bHistLB && m_bLBN;
}
bool    CFixMouse::IsPushRButton() const{
    return !m_bHistRB && m_bRBN && (m_nGuardTime==0);
}
bool    CFixMouse::IsPushLButton() const{
    return !m_bHistLB && m_bLBN && (m_nGuardTime==0);
}

/////////////////////////////////////////////////////////////////////////////
//    Was released since last Flush?

void    CFixMouse::GetUpButton(bool&bL,bool&bR){
    bR = m_bHistRB && !m_bRBN;
    bL = m_bHistLB && !m_bLBN;

}
bool    CFixMouse::IsPushUpRButton()const{
    return m_bHistRB && !m_bRBN && (m_nGuardTime==0);
}
bool    CFixMouse::IsPushUpLButton()const{
    return m_bHistLB && !m_bLBN && (m_nGuardTime==0);
}

/////////////////////////////////////////////////////////////////////////////

void    CFixMouse::ResetButton(){
    m_bHistRB = m_bRBN = false;
    m_bHistLB = m_bLBN = false;
#ifdef OPENJOEY_ENGINE_FIXES
    m_nFixedWheelDelta = 0; // Also reset wheel delta
#endif
}

void    CFixMouse::SetGuardTime(int nTime){
    m_nGuardTime = nTime;
}

bool    CFixMouse::IsGuardTime() const {
    return m_nGuardTime!=0;
}

} // end of namespace Input
} // end of namespace yaneuraoGameSDK3rd