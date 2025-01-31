// MouseInput.h:
// For Mouse Input
//    programmed by yaneurao(M.Isozaki) '99/7/31
//
// This is not a sophisticated class :p
//
// Note: Double-click detection is not implemented.
//

#ifndef __yaneMouseInput_h__
#define __yaneMouseInput_h__

#include "../Window/yaneWinHook.h"

namespace yaneuraoGameSDK3rd {
namespace Input {

class IMouse {
public:
    virtual LRESULT GetXY(int &x,int &y)const=0;
    virtual bool RButton()const=0;
    virtual bool LButton()const=0;
    virtual LRESULT GetInfo(int &x,int &y,int &b)const=0;
    virtual void GetButton(bool&bL,bool&bR)=0;
    virtual void ResetButton()=0;
    virtual LRESULT SetXY(int x,int y)=0;
    virtual void SetOutScreenInput(bool bEnable)=0;

    virtual ~IMouse(){}
};

class CMouse : public IWinHook,public IMouse {
/**
    This class is for getting real-time mouse state.

    Since it needs a completed window, 
    please use it within a class derived from CAppFrame.
*/
public:
    virtual LRESULT GetXY(int &x,int &y)const;
    /// Get mouse position (in client coordinates)

    virtual bool RButton()const;
    /// Get right button state (current real-time info)

    virtual bool LButton()const;
    /// Get left button state (current real-time info)

    virtual LRESULT GetInfo(int &x,int &y,int &b)const;
    /// Returns mouse position and button state
    /// (b: +1 if right button pressed, +2 if left button pressed, +3 if both pressed)

    virtual void GetButton(bool&bL,bool&bR);
    /// Was button pressed since last GetButton?
    virtual void ResetButton();
    /// Reset button state that can be retrieved with GetButton

    virtual LRESULT SetXY(int x,int y);
    /// Move mouse to specified position (in client coordinates)

    virtual void SetOutScreenInput(bool bEnable);
    /**
        What happens when mouse moves outside the window while button is pressed?
            true  == Button is considered still pressed
            false == Button is considered released
            default is false
    */

    CMouse();
    virtual ~CMouse();

protected:
    LRESULT WndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam); // Message callback

    bool m_bRB;              // Mouse button state
    bool m_bLB;
    bool m_bHistRB;          // History
    bool m_bHistLB;
    bool m_bOutScreenInput;  // Input outside screen
};

class CFixMouse : public IMouse {
/**
    This class is for getting real-time mouse information

    When using CMouse in a game, it's better to have
    fixed values during a frame.

    This relationship is similar to CTimer and CFixTimer.
*/
public:
    /// Flush (update) state
    virtual LRESULT Flush();
    /**
        Updates mouse coordinates and button states.
        After this is called, member functions will return values
        based on the state at this moment.
        After that, mostly same as CMouse, however,
    */

    virtual LRESULT GetXY(int &x,int &y)const;
    virtual bool RButton()const;
    virtual bool LButton()const;
    virtual LRESULT GetInfo(int &x,int &y,int &b)const;

    /// Was button pressed since last Flush?
    virtual void GetButton(bool&bL,bool&bR);
    virtual bool IsPushRButton()const;
    virtual bool IsPushLButton()const;

    /// Was button released since last Flush?
    virtual void GetUpButton(bool&bL,bool&bR);
    virtual bool IsPushUpRButton()const;
    virtual bool IsPushUpLButton()const;

    virtual void ResetButton();
    /// Reset button state

    virtual LRESULT SetXY(int x,int y);
    /// Move mouse to specified position (in client coordinates)
    /// If coordinates are moved this way, GetXY will return these coordinates even without Flush

    /**
        Guard time is a feature to prevent unwanted button press detection
        when switching scenes. When a button is pressed and the scene changes,
        the next scene might immediately detect that button press.
        To prevent this, for the duration set by SetGuardTime,
        GetButton/IsPushRButton/IsPushLButton will return that
        no buttons are pressed until that many Flush calls have been made.
    */
    /// Returns whether in guard time
    virtual bool IsGuardTime()const;
    /// Set guard time
    void SetGuardTime(int nTime);

    virtual void SetOutScreenInput(bool bEnable)
        { GetMouse()->SetOutScreenInput(bEnable);}

    CFixMouse();
    virtual ~CFixMouse();

protected:
    bool m_bRBN;           // Mouse button state at time of Flush
    bool m_bLBN;
    int m_nRLBN;          // Mouse button state at time of Flush
    int m_nX,m_nY;        // Mouse position at time of Flush
    int m_nGuardTime;     // Guard time
    bool m_bHistRB;       // History
    bool m_bHistLB;

    CMouse m_vMouse;      // Delegate to this
    CMouse* GetMouse() { return &m_vMouse; }
};

} // end of namespace Input
} // end of namespace yaneuraoGameSDK3rd

#endif