//
//      GUI Button Class
//
//      Originally programmed by yaneurao(M.Isozaki) '01/02/09-'01/02/26
//      Updated for YaneSdk3 compatibility
//

#ifndef __yaneGUIButton_h__
#define __yaneGUIButton_h__

#include "../../stdafx.h"
#include "yaneGUIParts.h"

namespace yaneuraoGameSDK3rd {
namespace Draw {

using namespace YTL;

class CGUIButtonEventListener {
public:
    virtual void OnInit(void) {}
    virtual void OnRBClick(void){}
    virtual void OnLBClick(void){}
    virtual void OnRBDown(void){}
    virtual void OnLBDown(void){}
    virtual void OnRBUp(void){}
    virtual void OnLBUp(void){}
	virtual ISurface* GetDrawSurface(bool bPush, bool bIn) = 0;

    virtual bool IsButton(int px, int py){ return true; }
	virtual bool IsButtonNoGFX(int px, int py, RECT& b){ return true; }
    virtual LRESULT OnDraw(ISurface* lp, int x, int y, bool bPush, bool bIn){ return 0; }

    virtual bool IsLClick(){ 
		return false; }
    virtual bool IsRClick(){ 
		return false; }

    virtual ~CGUIButtonEventListener(){}
};

class CGUINormalButtonListener : public CGUIButtonEventListener {
public:
    virtual void SetPlaneLoader(smart_ptr<CPlaneLoader> pv, int nNo);
    virtual void SetPlane(smart_ptr<ISurface> pv);
    virtual ISurface* GetPlane(void){
		if (m_bHasPlane)
			return m_vPlane.get();
		if (m_vPlaneLoader.get() == NULL) return NULL;
		m_cachedPlane = GetMyPlaneAsPlane(false);
		return m_cachedPlane.get();
	}

	virtual void SetPlaneNumber(int nNo){ 
		m_nPlaneStart = nNo;
	}

    virtual bool IsButton(int px, int py);
	virtual bool IsButtonNoGFX(int px, int py, RECT& b);
    virtual LRESULT OnDraw(ISurface* lp, int x, int y, bool bPush, bool bIn);

    virtual void SetType(int nType);
    virtual int  GetType() { return m_nType; }

    virtual void SetReverse(bool bReverse) { m_bReverse = bReverse; }
    virtual bool GetReverse() { return m_bReverse; }

	virtual ISurface* GetDrawSurface(bool bPush, bool bIn);

    virtual void SetBlinkSpeed(int n) { m_nBlink.SetEnd(n); }
    virtual void SetImageOffset(int n) { m_nImageOffset = n; }
    virtual int  GetImageOffset(void) { return m_nImageOffset; }
    virtual ISurface* GetMyPlane(bool bPush = false);

    /// Returns a CPlane that owns the surface; use this (or keep the CPlane in scope) instead of raw GetMyPlane() to avoid dangling pointers.
    CPlane GetMyPlaneAsPlane(bool bPush = false);

    virtual void OnLButtonClick(void) {}
    virtual void OnRButtonClick(void) {}
    virtual bool IsLClick(){ 
		return m_bLClick; }
    virtual bool IsRClick(){ 
		return m_bRClick; }

    CGUINormalButtonListener();

protected:
    virtual void OnInit(void);
    virtual void OnLBClick(void);
    virtual void OnRBClick(void);
    virtual void OnLBDown(void);

    smart_ptr<ISurface> m_vPlane;
    bool m_bHasPlane;
    smart_ptr<CPlaneLoader> m_vPlaneLoader;
    int  m_nPlaneStart;
    /// Cached plane so GetPlane()/GetDrawSurface() return valid pointers (no temporary CPlane destroyed).
    CPlane m_cachedPlane;
    CPlane m_cachedDrawPlane;

    int  m_nType;
    bool m_bReverse;
    bool m_bLClick;
    bool m_bRClick;
    Math::CRootCounter m_nBlink;
    int  m_nImageOffset;
};

class CGUIButton : public IGUIParts {
public:
    void SetEvent(smart_ptr<CGUIButtonEventListener> pv) { Reset(); m_pvButtonEvent = pv; }
    void SetLeftClick(bool b) { m_bLeftClick = b; }
    void SetRightClick(bool b) { m_bRightClick = b; }
	void SetBounds(RECT b) { m_bounds = b; m_boundsMode = true; }
	void ResetBounds(RECT b) { m_bounds = RECT(); m_boundsMode = false; }

	void SetScaleSize(int width, int height) { m_drawWidth = width; m_drawHeight = height; }
    void SetScaleSizeOnce(int width, int height) { m_oneShotDrawWidth = width; m_oneShotDrawHeight = height; }
	void ResetScaleSize() { m_drawWidth = 0; m_drawHeight = 0; } // Resets persistent size

	void SetID(int i) { m_id = i; }
	int  GetID() { return  m_id; }

    smart_ptr<CGUIButtonEventListener> GetEvent() { return m_pvButtonEvent; }
    bool IsPushed(void) { return m_bPushed; }
    bool IsIn(void) { return m_bIn; }

    bool IsLClick();
    bool IsRClick();

    ISurface* GetPlane();
    bool IsFocusing(){ return m_bFocusing; }

    virtual LRESULT OnSimpleMove(ISurface* lp);
    virtual LRESULT OnSimpleDraw(ISurface* lp);
	virtual LRESULT OnSimpleScaleDraw(ISurface* lp);

    virtual void Reset();
    virtual void GetXY(int &x, int &y);
	virtual void GetScaleXY(int &x, int &y);

    CGUIButton();
    virtual ~CGUIButton() {}

protected:
    smart_ptr<CGUIButtonEventListener> m_pvButtonEvent;

private:
    bool m_bPushed;
    int  m_nButton;
    bool m_bIn;
    bool m_bLeftClick;
    bool m_bRightClick;
    bool m_bFocusing;

	bool m_boundsMode;
	RECT m_bounds;
	int m_id;

    int m_drawWidth;
    int m_drawHeight;
    int m_oneShotDrawWidth;
    int m_oneShotDrawHeight;
};

} // namespace Draw
} // namespace yaneuraoGameSDK3rd

#endif