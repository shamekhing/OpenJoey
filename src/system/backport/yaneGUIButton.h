// yaneGUIButton.h
// Created by derplayer
// Created on 2025-01-26 22:40:52

#ifndef __yaneGUIButton_h__
#define __yaneGUIButton_h__

#include "../../stdafx.h"

class CGUIButtonEventListener {
public:
    virtual void OnInit() {}
    virtual bool IsButton(int px, int py) = 0;
    virtual LRESULT OnDraw(ISurface* lp, int x, int y, bool bPush, bool bIn) = 0;
    virtual void OnLBClick() {}
    virtual void OnRBClick() {}
    virtual void OnLBDown() {}
    virtual void OnLBUp() {}
    virtual void OnRBDown() {}
    virtual void OnRBUp() {}
    virtual ~CGUIButtonEventListener() {}
};

class CGUINormalButtonListener : public CGUIButtonEventListener {
public:
    CGUINormalButtonListener();
    
    virtual bool IsButton(int px, int py);
    virtual LRESULT OnDraw(ISurface* lp, int x, int y, bool bPush, bool bIn);
    virtual void OnLBClick();
    virtual void OnRBClick();
    virtual void OnLBDown();
    
    void SetPlaneLoader(CPlaneLoader* loader, int nNo);
    void SetType(int nType);
    void SetImageOffset(int offset) { m_nImageOffset = offset; }
    bool IsLClick() const { return m_bLClick; }
    void ResetClick() { m_bLClick = m_bRClick = false; }

protected:
    ISurface* GetMyPlane(bool bPush);
    
    CPlaneLoader* m_vPlaneLoader;
    int m_nPlaneStart;
    int m_nType;
    int m_nImageOffset;
    bool m_bLClick;
    bool m_bRClick;
    CRootCounter m_nBlink;
    bool m_bReverse;
};

class CGUIButton {
public:
    CGUIButton();
    
    void SetMouse(CMouse* pMouse) { m_pvMouse = pMouse; }
    void SetEvent(CGUIButtonEventListener* pEvent) { m_pvButtonEvent = pEvent; }
    void SetXY(int x, int y) { m_nX = x; m_nY = y; }
    
    LRESULT OnMove(ISurface* lp);
    LRESULT OnDraw(ISurface* lp);
    
    bool IsLClick() const { 
        return m_pvButtonEvent ? 
            ((CGUINormalButtonListener*)m_pvButtonEvent)->IsLClick() : false; 
    }
    void Reset();

protected:
    void GetXY(int& x, int& y);
    
    CMouse* m_pvMouse;
    CGUIButtonEventListener* m_pvButtonEvent;
    int m_nX, m_nY;
    int m_nXOffset, m_nYOffset;
    bool m_bLeftClick;
    bool m_bRightClick;
    bool m_bPushed;
    int m_nButton;
    bool m_bIn;
    bool m_bFocusing;
};

#endif