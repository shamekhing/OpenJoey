// yaneGUIButton.cpp
// Created by derplayer
// Created on 2025-01-26 22:42:33

#include "yaneGUIButton.h"

///////////////////////////////////////////////////////////////////////////////
// CGUINormalButtonListener Implementation

CGUINormalButtonListener::CGUINormalButtonListener() {
    m_nType = 1;        // Normal On/Off button
    m_bReverse = false; // Not in reverse mode
    m_nBlink.Set(0, 6); // Set counter from 0 to 6
    m_nImageOffset = 0; // Default offset
    m_bLClick = false;
    m_bRClick = false;
}

void CGUINormalButtonListener::SetPlaneLoader(CPlaneLoader* loader, int nNo) {
    m_vPlaneLoader = loader;
    m_nPlaneStart = nNo;
}

void CGUINormalButtonListener::SetType(int nType) {
    m_nType = nType;
    if (m_nType & 16) m_nBlink.Reset();
}

bool CGUINormalButtonListener::IsButton(int px, int py) {
    if (m_nType == 0) return false; // Invalid button
    
    ISurface* lp = GetMyPlane(false);
    if (!lp) return false;

    // For alpha-enabled surfaces, check alpha value
    if (lp->IsAlpha()) {
        BYTE alpha;
        ISurfaceRGB rgba = lp->GetConstSurfaceInfo()->GetColorKey(); // Using GetColorKey instead of GetRGB
        ISurface::getRGB(rgba, alpha, alpha, alpha, alpha);
        return alpha != 0;
    }

    // For normal surfaces, check if within bounds
    int sx, sy;
    lp->GetSize(sx, sy);
    return (0 <= px && px < sx && 0 <= py && py < sy);
}

LRESULT CGUINormalButtonListener::OnDraw(ISurface* lp, int x, int y, bool bPush, bool bIn) {
    if (m_nType == 0) return 0;

    // Type 32/64: Use fixed image offset
    if (m_nType & (32+64)) {
        return lp->BltNatural(GetMyPlane(false), x, y);
    }

    bool bPressed;
    if (m_nType & 16) {      // Blink mode
        // Use IsEnd() to check if we reached the end of the counter
        bPressed = m_nBlink.IsEnd();
        m_nBlink++;
    } else if (m_nType & 8) {  // Hover mode
        bPressed = bIn;
    } else {                   // Normal push mode
        bPressed = bPush && bIn;
    }

    return lp->BltNatural(GetMyPlane(bPressed), x, y);
}

void CGUINormalButtonListener::OnLBClick() {
    if (m_nType == 0 || (m_nType & 16) || (m_nType & 8)) return;
    m_bLClick = true;
}

void CGUINormalButtonListener::OnRBClick() {
    if (m_nType == 0 || (m_nType & 16) || (m_nType & 8)) return;
    m_bRClick = true;
}

void CGUINormalButtonListener::OnLBDown() {
    if (!(m_nType & 8) || ((m_nType & 16))) return;
    m_bLClick = true;
}

ISurface* CGUINormalButtonListener::GetMyPlane(bool bPush) {
    int n = m_nPlaneStart;
    if (bPush) n++;
    if ((m_nType & 2) && m_bReverse) n += 2;
    if (m_nType & (32+64)) n = m_nPlaneStart + m_nImageOffset;
    return m_vPlaneLoader->GetPlane(n);
}

///////////////////////////////////////////////////////////////////////////////
// CGUIButton Implementation

CGUIButton::CGUIButton() {
    m_pvButtonEvent = NULL;  // Using NULL instead of nullptr for older C++
    m_pvMouse = NULL;
    m_bLeftClick = true;
    m_bRightClick = false;
    m_nXOffset = m_nYOffset = 0;
    Reset();
}

LRESULT CGUIButton::OnMove(ISurface* lp) {
    if (!m_pvButtonEvent) return -1;

    m_pvButtonEvent->OnInit();

    if (!m_pvMouse) return -1;

    int x, y, b;
    m_pvMouse->GetInfo(x, y, b);

    bool bRUp   = (m_nButton & 1) && !(b & 1);
    bool bRDown = !(m_nButton & 1) && (b & 1);
    bool bLUp   = (m_nButton & 2) && !(b & 2);
    bool bLDown = !(m_nButton & 2) && (b & 2);

    GetXY(m_nX, m_nY);
    bool bIn = m_pvButtonEvent->IsButton(x - m_nX, y - m_nY);

    // v3 doesn't have IsGuardTime, we'll use a simpler check
    bool bNGuard = true;  // Always allow input in v3

    if (bIn && bNGuard) {
        if (bRDown) m_pvButtonEvent->OnRBDown();
        if (bLDown) m_pvButtonEvent->OnLBDown();
        if (bRUp)   m_pvButtonEvent->OnRBUp();
        if (bLUp)   m_pvButtonEvent->OnLBUp();
    }

    // Left click handling
    if (m_bLeftClick && m_bIn && bIn && !(m_nButton & 2) && (b & 2)) {
        m_bPushed = true;
    }
    // Right click handling
    if (m_bRightClick && m_bIn && bIn && !(m_nButton & 1) && (b & 1)) {
        m_bPushed = true;
    }

    if (bIn && m_bPushed && bNGuard) {
        if (bLUp) m_pvButtonEvent->OnLBClick();
        if (bRUp) m_pvButtonEvent->OnRBClick();
    }

    if (m_bPushed && b == 0) {
        m_bPushed = false;
    }

    m_bFocusing = (!m_bIn) && (bIn);
    m_bIn = bIn;
    m_nButton = b;

    if (bIn || m_bPushed) {
        m_pvMouse->ResetButton();
    }

    return 0;
}

LRESULT CGUIButton::OnDraw(ISurface* lp) {
    return m_pvButtonEvent->OnDraw(lp, m_nX + m_nXOffset, m_nY + m_nYOffset, 
                                  m_bPushed, m_bIn);
}

void CGUIButton::Reset() {
    m_bPushed = false;
    m_nButton = 0;
    m_bIn = false;
    m_bFocusing = false;
}

void CGUIButton::GetXY(int& x, int& y) {
    x = m_nX;
    y = m_nY;
}