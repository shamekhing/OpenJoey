#include "yaneGUIButton.h"

namespace yaneuraoGameSDK3rd {
namespace Draw {

///////////////////////////////////////////////////////////////////////////////
CGUINormalButtonListener::CGUINormalButtonListener(void) {
    m_nType = 1;
    m_bReverse = false;
    m_nBlink.Set(0,6);
    m_nImageOffset = 0;
    m_bHasPlane = false;
    m_bLClick = false;
    m_bRClick = false;
}

void CGUINormalButtonListener::SetPlaneLoader(smart_ptr<CPlaneLoader> pv, int nNo) {
    if (pv.get() != NULL) {
        m_vPlaneLoader = pv;
    }
    if (m_vPlaneLoader.get() == NULL) {
        WARNING(true, "CGUINormalButtonListener::m_vPlaneLoader == NULL");
    }
    m_nPlaneStart = nNo;
}

void CGUINormalButtonListener::SetPlane(smart_ptr<ISurface> pv) {
    m_vPlane = pv;
    m_bHasPlane = (pv.get() != NULL);
}

ISurface* CGUINormalButtonListener::GetDrawSurface(bool bPush, bool bIn) {
    if (m_nType == 0) return NULL;

    bool bUsePushState;
    if (m_nType & 16) { // Blink type
        bUsePushState = (m_nBlink.Get() % 2) == 0;
        m_nBlink++; // Advance blink counter
    } else if (m_nType & 8) { // Hover type
        bUsePushState = bIn;
    } else { // Normal push type
        bUsePushState = bPush && bIn;
    }

    // Returns the surface based on the button's state for drawing
    return GetMyPlane(bUsePushState);
}

void CGUINormalButtonListener::SetType(int nType) {
    m_nType = nType;
    if (m_nType & 16) m_nBlink.Reset();
}

bool CGUINormalButtonListener::IsButton(int px, int py) {
    if (m_nType == 0) return false;
    
    ISurface* lp = GetMyPlane();
    if (lp == NULL) return false;

    if (lp->IsAlpha() && (m_nType & 4)) {
        ISurfaceRGB rgba;
        if (lp->GetConstSurfaceInfo()->GetPixel(px, py, rgba) == 0) {
            return ((rgba >> 24) & 0xff) != 0;
        }
        return false;
    } else {
        ISurfaceRGB rgba;
        if (lp->GetConstSurfaceInfo()->GetPixel(px, py, rgba) == 0) {
            return rgba != lp->GetColorKey();
        }
        return false;
    }
}

bool CGUINormalButtonListener::IsButtonNoGFX(int px, int py, RECT& bounds) {
    // Check if the button type is valid
    if (m_nType == 0) {
        return false; // Button type is invalid
    }

    // Perform manual boundary check with the provided rectangle
    if (px >= bounds.left && px <= bounds.right &&
        py >= bounds.top && py <= bounds.bottom) {
        return true; // Point is within the rectangle
    }

    return false; // Point is outside the rectangle
}

LRESULT CGUINormalButtonListener::OnDraw(ISurface* lp, int x, int y, bool bPush, bool bIn) {
    if (m_nType == 0) return 0;

    if (m_nType & (32+64)) {
        if (m_bHasPlane && m_vPlane.get() != NULL) {
            ISurface* surface = m_vPlaneLoader->GetPlane(m_nPlaneStart + m_nImageOffset).get();
            return lp->BltNatural(surface, x, y);
        }
        return 0;
    }

    bool b;
    if (m_nType & 16) {
        // Use Get() method instead of GetCount()
        b = (m_nBlink.Get() % 2) == 0;  // or simply use: b = ((int)m_nBlink % 2) == 0;
        m_nBlink++;
    } else if (m_nType & 8) {
        b = bIn;
    } else {
        b = bPush && bIn;
    }

    return lp->BltNatural(GetMyPlane(b), x, y);
}

void CGUINormalButtonListener::OnLBClick() {
    if (m_nType == 0 || (m_nType & 16) || (m_nType & 8)) return;
    m_bLClick = true;
    OnLButtonClick();
}

void CGUINormalButtonListener::OnRBClick() {
    if (m_nType == 0 || (m_nType & 16) || (m_nType & 8)) return;
    m_bRClick = true;
    OnRButtonClick();
}

void CGUINormalButtonListener::OnLBDown() {
    if (!(m_nType & 8) || ((m_nType & 16))) return;
    m_bLClick = true;
    OnLButtonClick();
}

ISurface* CGUINormalButtonListener::GetMyPlane(bool bPush) {
	if(m_bHasPlane) return m_vPlane.get();
    if (m_vPlaneLoader.get() == NULL) return NULL;
    
    int n = m_nPlaneStart;
    if (bPush) n++;
    if ((m_nType & 2) && m_bReverse) n += 2;
    if (m_nType & (32+64)) n = m_nPlaneStart + m_nImageOffset;
    return m_vPlaneLoader->GetPlane(n).get();
}

void CGUINormalButtonListener::OnInit(void) {
    m_bLClick = false;
    m_bRClick = false;
}

///////////////////////////////////////////////////////////////////////////////

CGUIButton::CGUIButton(void) {
    m_pvButtonEvent = smart_ptr<CGUIButtonEventListener>(new CGUINormalButtonListener());
    m_bLeftClick = true;
    m_bRightClick = false;
	m_id = -1;
	m_drawWidth = 0;
    m_drawHeight = 0;
    m_oneShotDrawWidth = 0;
    m_oneShotDrawHeight = 0;
    Reset();
}

LRESULT CGUIButton::OnSimpleMove(ISurface* lp) {
    if (m_pvButtonEvent.get() == NULL) return -1;

    m_pvButtonEvent->OnInit();

    if (m_pvMouse.get() == NULL) return -1;

    int x, y, b;
    m_pvMouse->GetInfo(x, y, b);

    bool bRUp = (m_nButton & 1) && (!(b & 1));
    bool bRDown = (!(m_nButton & 1)) && (b & 1);
    bool bLUp = (m_nButton & 2) && (!(b & 2));
    bool bLDown = (!(m_nButton & 2)) && (b & 2);

    //GetXY(m_nX, m_nY); // TODO: ??? was it always here? - this causes movement wtf
    bool bIn = m_pvButtonEvent->IsButton(x - m_nX, y - m_nY);

	// Special edgecase mode - for collision on sliced menu gfx buttons
	if(m_boundsMode){
		bIn = m_pvButtonEvent->IsButtonNoGFX(x - m_nX, y - m_nY, m_bounds);
	}
    bool bNGuard = !m_pvMouse->IsGuardTime();
	
    if (bIn && bNGuard) {
        if (bRDown) m_pvButtonEvent->OnRBDown();
        if (bLDown) m_pvButtonEvent->OnLBDown();
        if (bRUp) m_pvButtonEvent->OnRBUp();
        if (bLUp) m_pvButtonEvent->OnLBUp();
    }

    if (m_bLeftClick && m_bIn && bIn && !(m_nButton & 2) && (b & 2)) {
        m_bPushed = true;
    }
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

LRESULT CGUIButton::OnSimpleDraw(ISurface* lp) {
    if (m_pvButtonEvent.get() == NULL) return -1;
    return m_pvButtonEvent->OnDraw(lp, m_nX, m_nY, m_bPushed, m_bIn);
}

LRESULT CGUIButton::OnSimpleScaleDraw(ISurface* lp) {
    if (m_pvButtonEvent.get() == NULL) return -1;

    int actualDrawWidth = 0;
    int actualDrawHeight = 0;
    bool useCustomScale = false;

    // 1. Prioritize one-shot draw size (SetScaleSizeOnce)
    if (m_oneShotDrawWidth > 0 && m_oneShotDrawHeight > 0) {
        actualDrawWidth = m_oneShotDrawWidth;
        actualDrawHeight = m_oneShotDrawHeight;
        useCustomScale = true;
        // IMPORTANT: Reset one-shot dimensions immediately after use
        m_oneShotDrawWidth = 0;
        m_oneShotDrawHeight = 0;
    }
    // 2. Fallback to persistent draw size (SetScaleSize)
    else if (m_drawWidth > 0 && m_drawHeight > 0) {
        actualDrawWidth = m_drawWidth;
        actualDrawHeight = m_drawHeight;
        useCustomScale = true;
    }

    // If a custom scale is active, use BltFast with the source surface
    if (useCustomScale) {
        ISurface* sourcePlane = m_pvButtonEvent->GetDrawSurface(m_bPushed, m_bIn);
        if (sourcePlane) {
            SIZE dstSize = { actualDrawWidth, actualDrawHeight };
            RECT srcRect = { 0, 0, sourcePlane->GetConstSurfaceInfo()->GetSize().cx, sourcePlane->GetConstSurfaceInfo()->GetSize().cy };
            return lp->BltFast(sourcePlane, m_nX, m_nY, &dstSize, &srcRect, NULL, 0);
        }
        return 0; // If sourcePlane is null, nothing to draw
    } else {
        // Otherwise, fallback to the listener's default OnDraw (which uses natural size)
        return m_pvButtonEvent->OnDraw(lp, m_nX, m_nY, m_bPushed, m_bIn);
    }
}

ISurface* CGUIButton::GetPlane() {
	if (m_pvButtonEvent.get() == NULL) return NULL;
	//CGUINormalButtonListener* pListener = static_cast<CGUINormalButtonListener*>(m_pvButtonEvent.get());
	//return pListener->GetPlane();
	return ((CGUINormalButtonListener*)(CGUIButtonEventListener*)m_pvButtonEvent.get())->GetPlane();

	//CGUIButtonEventListener* buttonEventListener = (CGUIButtonEventListener*)m_pvButtonEvent.get();
	//CGUINormalButtonListener* normalButtonListener = (CGUINormalButtonListener*)buttonEventListener;
	//ISurface* plane;

	//if (normalButtonListener) {
	//	plane = normalButtonListener->GetPlane();
	//}
	//return plane;
}

bool CGUIButton::IsLClick() {
    return m_pvButtonEvent.get() ? m_pvButtonEvent->IsLClick() : false;
}

bool CGUIButton::IsRClick() {
    return m_pvButtonEvent.get() ? m_pvButtonEvent->IsRClick() : false;
}

void CGUIButton::Reset() {
    m_bPushed = false;
    m_nButton = 0;
    m_bIn = false;
    m_bFocusing = false;

	m_boundsMode = false;
	m_bounds = RECT();
	m_drawWidth = 0;          // Reset persistent custom draw size
    m_drawHeight = 0;         // Reset persistent custom draw size
    m_oneShotDrawWidth = 0;   // Reset one-shot custom draw size
    m_oneShotDrawHeight = 0;  // Reset one-shot custom draw size
}

void CGUIButton::GetXY(int &x, int &y) {
    ISurface* plane = GetPlane();
    if (plane == NULL) {
        x = m_nX;
        y = m_nY;
    } else {
        x = m_nX;
        y = m_nY;
        int sx, sy;
        if (plane->GetSize(sx, sy) == 0) {
            x += sx;
            y += sy;
        }
    }
}

/// Special GetXY that accounts for the plane scale
void CGUIButton::GetScaleXY(int &x, int &y) {
    // Determine the size to use for coordinate calculation
    int currentWidth = m_drawWidth;
    int currentHeight = m_drawHeight;

    // If a one-shot scale is active, prioritize it for current size calculation
    if (m_oneShotDrawWidth > 0 && m_oneShotDrawHeight > 0) {
        currentWidth = m_oneShotDrawWidth;
        currentHeight = m_oneShotDrawHeight;
    }
    // If no explicit scaled size (persistent or one-shot) is set, get natural size
    else if (currentWidth <= 0 || currentHeight <= 0) {
        // Note: GetPlane() here might not return the state-specific plane (pushed/in)
        // This is a limitation if precise bounds are needed for dynamic images,
        // but it's consistent with how GetPlane() was likely used before.
        ISurface* plane = GetPlane();
        if (plane && plane->GetSize(currentWidth, currentHeight) != 0) {
             currentWidth = 0; // Failed to get size
             currentHeight = 0;
        }
    }

    x = m_nX; // Base X position
    y = m_nY; // Base Y position
    
    // Add dimensions to get bottom-right corner (or for other uses of this GetXY)
    x += currentWidth;
    y += currentHeight;
}

} // namespace Draw
} // namespace yaneuraoGameSDK3rd