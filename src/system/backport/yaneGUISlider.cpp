#include "yaneGUISlider.h"

namespace yaneuraoGameSDK3rd {
namespace Draw {

///////////////////////////////////////////////////////////////////////////////

CGUISliderEventListener::CGUISliderEventListener() {
    m_nMinX = 5;
    m_nMinY = 5;
}

void CGUINormalSliderListener::GetSliderSize(int nX, int nY, int& sx, int& sy) {
    // Get the movable area
    LPRECT lprc = GetSlider()->GetRect();
    
    // Get size from our surface
    if (m_bHasPlane && m_vPlane.get()) {
        m_vPlane->GetSize(sx, sy);
    }
    else {
        sx = m_nMinX;
        sy = m_nMinY;
    }

    // Ensure minimum size
    if (sx < m_nMinX) sx = m_nMinX;
    if (sy < m_nMinY) sy = m_nMinY;
}

LRESULT CGUINormalSliderListener::OnDraw(ISurface* lp, int x, int y, int nX, int nY) {
    int nType = GetSlider()->GetType();
	LPRECT lprc = GetSlider()->GetRect();

    // Get total slider size
    int nSx, nSy;
    GetSliderSize(nX, nY, nSx, nSy);

    // For horizontal slider (Type 1), fix the Y position
    if (nType == 1) {
        y = lprc->top;  // Lock Y position to top of movable area
    }

    if (m_bHasPlane) {
        return lp->BltNatural(m_vPlane.get(), x, y);
    }

    // Display the plane from plane loader stretched
    ISurface* plane0 = m_vPlaneLoader->GetPlane(0).get();
    ISurface* plane1 = m_vPlaneLoader->GetPlane(1).get();
    ISurface* plane2 = m_vPlaneLoader->GetPlane(2).get();

    if (!plane0 || !plane1 || !plane2) {
        WARNING(true, "CGUINormalSliderListener::Plane is NULL");
        return -1;
    }

    int nSx1, nSy1, nSx2, nSy2, nSx3, nSy3;
    plane0->GetSize(nSx1, nSy1);
    plane1->GetSize(nSx2, nSy2);
    plane2->GetSize(nSx3, nSy3);

    // Slider type
    switch (nType) {
    case 0: { // Vertical movement
        lp->BltNatural(plane0, x, y);
        lp->BltNatural(plane2, x, y + nSy - nSy3);
        
        // Calculate slider center part
        nSy -= (nSy1 + nSy3);
        y += nSy1;
        
        for (; nSy >= nSy2; nSy -= nSy2, y += nSy2) {
            lp->BltNatural(plane1, x, y);
        }
        
        // Draw remaining slider center part
        if (nSy != 0) {
            SIZE sz = { nSx2, nSy };
            lp->BltNatural(plane1, x, y, &sz);
        }
        } break;

    case 1: { // Horizontal movement
        lp->BltNatural(plane0, x, y);
        lp->BltNatural(plane2, x + nSx - nSx3, y);
        
        // Calculate slider center part
        nSx -= (nSx1 + nSx3);
        x += nSx1;
        
        for (; nSx >= nSx2; nSx -= nSx2, x += nSx2) {
            lp->BltNatural(plane1, x, y);
        }
        
        // Draw remaining slider center part
        if (nSx != 0) {
            SIZE sz = { nSx, nSy2 };
            lp->BltNatural(plane1, x, y, &sz);
        }
        } break;

    case 2: { // Both vertical and horizontal movement
        lp->BltNatural(plane0, x, y);
        } break;
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

CGUISlider::CGUISlider() {
    ::SetRect(&m_rcRect, 0, 0, 640, 480);
    m_nItemNumX = 1;
    m_nItemNumY = 1;
    m_nType = 0;
    Reset();
}

void CGUISlider::Reset() {
    m_bDraged = false;
    m_nButton = 0;
    m_bIn = false;    // Prevent immediate press when first displaying
    m_bFocusing = false;
    m_bUpdate = false;
    m_nSelectedItemX = 0;
    m_nSelectedItemY = 0;
}

void CGUISlider::GetXY(int& x, int& y) {
    x = m_nX;
    y = m_nY;
}

void CGUISlider::SetEvent(smart_ptr<CGUISliderEventListener> pv) {
    Reset();
    m_pvSliderEvent = pv;
    m_pvSliderEvent->SetSlider(smart_ptr<CGUISlider>(this));
}

void CGUISlider::CalcSliderPos(int& x, int& y, int& nSLX, int& nSLY, int& w, int& h) {
    // Calculate slider position
    // Determine slider position and button size
    int nXX = m_nX + m_nXOffset + m_rcRect.left;
    int nYY = m_nY + m_nYOffset + m_rcRect.top;
    m_pvSliderEvent->GetSliderSize(m_nItemNumX, m_nItemNumY, nSLX, nSLY);
    
    w = m_rcRect.right - m_rcRect.left - nSLX;
    h = m_rcRect.bottom - m_rcRect.top - nSLY;
    
    if (m_nItemNumX >= 2) {
        nXX += (w * m_nSelectedItemX) / (m_nItemNumX - 1);
    }
    if (m_nItemNumY >= 2) {
        nYY += (h * m_nSelectedItemY) / (m_nItemNumY - 1);
    }
    x = nXX;
    y = nYY;
}

LRESULT CGUISlider::OnSimpleMove(ISurface* lp) {
    if (m_pvMouse.get() == NULL) return -1;

    int nXX, nYY, nSLX, nSLY, w, h;
    CalcSliderPos(nXX, nYY, nSLX, nSLY, w, h);

    int mx, my, mb;
    m_pvMouse->GetInfo(mx, my, mb);

    GetXY(m_nX, m_nY);
    
    // Slider virtual coordinates with (0,0) at top-left
    int mmx = mx - m_nX - m_nXOffset;
    int mmy = my - m_nY - m_nYOffset;

    // Guard frame input rejection mechanism
    bool bNGuard = !m_pvMouse->IsGuardTime();

    // Is mouse over slider button?
    bool bIn;
    // Is mouse over slider area?
    LPRECT prc = GetRect();
    bool bSliderIn = (prc->left <= mmx) && (mmx < prc->right) &&
                    (prc->top <= mmy) && (mmy < prc->bottom);

    if (m_bDraged) {
        bIn = true; // Always true while dragging
    }
    else if (!bSliderIn) {
        bIn = false;
    }
    else {
        // Check if over slider button and valid button position
        if ((nXX <= mx) && (mx < nXX + nSLX) &&
            (nYY <= my) && (my < nYY + nSLY) &&
            m_pvSliderEvent->IsButton(mmx - nXX, mmy - nYY)) {
            bIn = true;
        }
        else {
            bIn = false;
        }
    }

    // Handle slider button clicks
    if (!bIn && bSliderIn && bNGuard) {
        switch (m_nType) {
        case 0: // Vertical slider
            if (mmy < nYY) {
                m_pvSliderEvent->OnPageUp();
            }
            else {
                m_pvSliderEvent->OnPageDown();
            }
            break;
        case 1: // Horizontal slider 
            if (mmx < nXX) {
                m_pvSliderEvent->OnPageLeft();
            }
            else {
                m_pvSliderEvent->OnPageRight();
            }
            break;
        case 2: // Both
            if (mmy < nYY) {
                m_pvSliderEvent->OnPageUp();
            }
            else {
                m_pvSliderEvent->OnPageDown();
            }
            if (mmx < nXX) {
                m_pvSliderEvent->OnPageLeft();
            }
            else {
                m_pvSliderEvent->OnPageRight();
            }
            break;
        }
    }

    // Start dragging on left click
    if (m_bIn && bIn && !(m_nButton & 2) && (mb & 2)) {
        if (!m_bDraged) {
            m_bDraged = true;
            m_nDragPosX = mx - nXX;
			m_nDragPosY = my - nYY;
		}
	}

	if (m_bDraged) {
		// Update selected item while dragging
		//int x = mmx - m_nDragPosX;
		//int y = mmy - m_nDragPosY;
		int x = mmx - m_rcRect.left - m_nDragPosX;
		int y = mmy - m_rcRect.top - m_nDragPosY;

		if (m_nItemNumX >= 2 && w != 0) {
			m_nSelectedItemX = (x * (m_nItemNumX - 1) + w / 2) / w;
		}
		else {
            m_nSelectedItemX = 0;
        }

        if (m_nSelectedItemX < 0) m_nSelectedItemX = 0;
        if (m_nSelectedItemX >= m_nItemNumX) m_nSelectedItemX = m_nItemNumX - 1;

        if (m_nItemNumY >= 2 && h != 0) {
            m_nSelectedItemY = (y * (m_nItemNumY - 1) + h / 2) / h;
        }
        else {
            m_nSelectedItemY = 0;
        }

        if (m_nSelectedItemY < 0) m_nSelectedItemY = 0;
        if (m_nSelectedItemY >= m_nItemNumY) m_nSelectedItemY = m_nItemNumY - 1;

        // Released from drag?
        if (mb == 0) {
            m_bDraged = false;
            CalcSliderPos(nXX, nYY, nSLX, nSLY, w, h);
        }
    }

    m_bFocusing = (!m_bIn) && (bIn);
    m_bIn = bIn;
    m_nButton = mb;

    if (bIn || m_bDraged) {
        m_pvMouse->ResetButton();
    }

    if (m_bDraged) {
        // During drag (display at arbitrary position)
        m_nPosX = mx - m_nDragPosX;
        m_nPosY = my - m_nDragPosY;

        // Push position back into movable area
        if (m_nPosX < prc->left + m_nX + m_nXOffset) {
            m_nPosX = prc->left + m_nX + m_nXOffset;
        }
        if (m_nPosY < prc->top + m_nY + m_nYOffset) {
            m_nPosY = prc->top + m_nY + m_nYOffset;
        }
        if (m_nPosX + nSLX > prc->right + m_nX + m_nXOffset) {
            m_nPosX = prc->right + m_nX + m_nXOffset - nSLX;
        }
        if (m_nPosY + nSLY > prc->bottom + m_nY + m_nYOffset) {
            m_nPosY = prc->bottom + m_nY + m_nYOffset - nSLY;
        }
    }
    else {
        // Not dragging (display at fixed position)
        m_nPosX = nXX;
        m_nPosY = nYY;
    }

    return 0;
}

LRESULT CGUISlider::OnSimpleDraw(ISurface* lp) {
    if (m_pvSliderEvent.get() == NULL) return -1;
    return m_pvSliderEvent->OnDraw(lp, m_nPosX, m_nPosY, m_nItemNumX, m_nItemNumY);
}

} // namespace Draw
} // namespace yaneuraoGameSDK3rd