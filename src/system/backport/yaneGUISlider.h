//
// GUI Slider Class
//
// Originally programmed by yaneurao(M.Isozaki) '01/06/10
// Updated for YaneSdk3 compatibility
//

#ifndef __yaneGUISlider_h__
#define __yaneGUISlider_h__

#include "../../stdafx.h"
#include "yaneGUIParts.h"

namespace yaneuraoGameSDK3rd {
namespace Draw {

using namespace YTL;

class CGUISlider;

// Button event notification handler
class CGUISliderEventListener {
public:
    virtual void OnInit(void) {}

    // These MUST be overridden
    virtual bool IsButton(int px, int py) { return true; }
    virtual bool IsButtonNoGFX(int px, int py, RECT& b) { return true; }
    virtual LRESULT OnDraw(ISurface* lp, int x, int y, int nX, int nY) { return 0; }
    virtual void GetSliderSize(int nX, int nY, int& sx, int& sy) = 0;

    // Events
	enum SliderEvent { None, PageUp, PageDown, PageLeft, PageRight };
	virtual void OnPageUp() { m_lastEvent = PageUp; }		// Vertical slider: area above button clicked
    virtual void OnPageDown() { m_lastEvent = PageDown; }	// Vertical slider: area below button clicked
    virtual void OnPageLeft() { m_lastEvent = PageLeft; }	// Horizontal slider: area left of button clicked
    virtual void OnPageRight() { m_lastEvent = PageRight; }	// Horizontal slider: area right of button clicked

    SliderEvent GetLastEvent() const { return m_lastEvent; }
    void ResetEventFlag() { m_lastEvent = None; }

    void SetMinSize(int sx, int sy) { m_nMinX = sx; m_nMinY = sy; }
    void GetMinSize(int& sx, int& sy) { sx = m_nMinX; sy = m_nMinY; }

    void SetSlider(smart_ptr<CGUISlider> v) { m_vGUISlider = v; }
    smart_ptr<CGUISlider> GetSlider() { return m_vGUISlider; }

    CGUISliderEventListener();
    virtual ~CGUISliderEventListener() {}

protected:
    smart_ptr<CGUISlider> m_vGUISlider;
    int m_nMinX, m_nMinY;    // Minimum slider size
    SliderEvent m_lastEvent; // Using the "m_" prefix for clarity
};

class CGUINormalSliderListener : public CGUISliderEventListener {
public:
    CGUINormalSliderListener();
    virtual void SetPlaneLoader(smart_ptr<CPlaneLoader> pv) { m_vPlaneLoader = pv; }
    virtual void GetSliderSize(int nX, int nY, int& sx, int& sy);
    virtual LRESULT OnDraw(ISurface* lp, int x, int y, int nX, int nY);
    
    virtual void SetPlane(smart_ptr<ISurface> pv) { m_vPlane = pv; m_bHasPlane = (pv.getPointer() != NULL); }
    virtual ISurface* GetPlane() { return m_vPlane.get(); }

protected:
    smart_ptr<CPlaneLoader> m_vPlaneLoader;
    smart_ptr<ISurface> m_vPlane;
    bool m_bHasPlane;
};

class CGUISlider : public IGUIParts {
public:
    void SetEvent(smart_ptr<CGUISliderEventListener> pv);
    /// Pass same smart_ptr so listener shares ref (avoids double-delete when scene destroys slider).
    void SetEvent(smart_ptr<CGUISliderEventListener> pv, smart_ptr<CGUISlider> selfRef);
    smart_ptr<CGUISliderEventListener> GetEvent() { return m_pvSliderEvent; }
    
    bool IsDraged() { return m_bDraged; }
    bool IsIn() { return m_bIn; }
    bool IsUpdate() { return m_bUpdate; }
	int GetButton() { if(!m_nInSlider) { return 0; } return m_nButton; }
    
    void SetRect(LPRECT lprc) { m_rcRect = *lprc; }
    LPRECT GetRect() { return &m_rcRect; }
    
    void SetSelectedItem(int nX, int nY) { m_nSelectedItemX = nX; m_nSelectedItemY = nY; }
    void GetSelectedItem(int& nX, int& nY) { nX = m_nSelectedItemX; nY = m_nSelectedItemY; }
    
    void SetItemNum(int nX, int nY) { m_nItemNumX = nX; m_nItemNumY = nY; }
    void GetItemNum(int& nX, int& nY) { nX = m_nItemNumX; nY = m_nItemNumY; }
    
    void SetType(int nType) { m_nType = nType; }
    int GetType() { return m_nType; }

    virtual LRESULT OnSimpleDraw(ISurface* lp);
    virtual LRESULT OnSimpleMove(ISurface* lp);
    
    virtual void Reset();
    virtual void GetXY(int& x, int& y);
    void CalcSliderPos(int& x, int& y, int& nSx, int& nSy, int& w, int& h);

    CGUISlider();
    virtual ~CGUISlider() {}

protected:
    smart_ptr<CGUISliderEventListener> m_pvSliderEvent;
    int m_nItemNumX;         // Number of items in X direction
    int m_nItemNumY;         // Number of items in Y direction
    int m_nSelectedItemX;    // Selected item in X direction
    int m_nSelectedItemY;    // Selected item in Y direction
    int m_nPosX, m_nPosY;   // Current coordinates (relative)

private:
    int m_nType;            // Slider type (0:vertical, 1:horizontal, 2:both)
    bool m_bDraged;         // Is button being dragged?
    int m_nDragPosX;       // X coordinate where drag started (relative)
    int m_nDragPosY;       // Y coordinate where drag started (relative)
    bool m_bFocusing;      // Focus gained this frame?
    int m_nButton;         // Previous mouse button state
	bool m_nInSlider;		// Set when the cursor is inside the slider area
    bool m_bIn;           // Was mouse inside button last frame?
    bool m_bUpdate;       // Slider position changed since last frame
    RECT m_rcRect;       // Slider movement area
};

} // namespace Draw
} // namespace yaneuraoGameSDK3rd

#endif