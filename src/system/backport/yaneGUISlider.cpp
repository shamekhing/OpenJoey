#include "stdafx.h"
#include "yaneGUISlider.h"

namespace yaneuraoGameSDK3rd {
namespace Draw {

///////////////////////////////////////////////////////////////////////////////

CGUISliderEventListener::CGUISliderEventListener() {
    m_nMinX = 5;
    m_nMinY = 5;
    ResetEventFlag();
}

void CGUINormalSliderListener::GetSliderSize(int nX, int nY, int& sx, int& sy) {
    LPRECT lprc = GetSlider()->GetRect();
    int w = lprc->right - lprc->left;
    int h = lprc->bottom - lprc->top;

    if (nX == 0) sx = w;
    else {
        sx = w / nX;
        if (sx < m_nMinX) sx = m_nMinX;
    }
    if (nY == 0) sy = h;
    else {
        sy = h / nY;
        if (sy < m_nMinY) sy = m_nMinY;
    }
}

LRESULT CGUINormalSliderListener::OnDraw(ISurface* lp, int x, int y, int nX, int nY) {
    int nType = GetSlider()->GetType();
    int nSx, nSy;
    GetSliderSize(nX, nY, nSx, nSy);

    if (m_bHasPlane) {
        return lp->BltNatural(m_vPlane.get(), x, y);
    }

    CPlane planeHolder = m_vPlaneLoader->GetPlane(0);
    ISurface* plane0 = planeHolder.get();
    ISurface* plane1 = planeHolder.get();
    ISurface* plane2 = planeHolder.get();

    if (!plane0 || !plane1 || !plane2) {
        WARNING(true, "CGUINormalSliderListener::Plane is NULL");
        return -1;
    }

    int nSx1, nSy1, nSx2, nSy2, nSx3, nSy3;
    plane0->GetSize(nSx1, nSy1);
    plane1->GetSize(nSx2, nSy2);
    plane2->GetSize(nSx3, nSy3);

    switch (nType) {
    case 0: { // Vertical
        lp->BltNatural(plane0, x, y);
        lp->BltNatural(plane2, x, y + nSy - nSy3);
        nSy -= (nSy1 + nSy3);
        y += nSy1;
        for (; nSy >= nSy2; nSy -= nSy2, y += nSy2) {
            lp->BltNatural(plane1, x, y);
        }
        if (nSy != 0) {
            SIZE sz = { nSx2, nSy };
            lp->BltNatural(plane1, x, y, &sz);
        }
        } break;
    case 1: { // Horizontal
        lp->BltNatural(plane0, x, y);
        lp->BltNatural(plane2, x + nSx - nSx3, y);
        nSx -= (nSx1 + nSx3);
        x += nSx1;
        for (; nSx >= nSx2; nSx -= nSx2, x += nSx2) {
            lp->BltNatural(plane1, x, y);
        }
        if (nSx != 0) {
            SIZE sz = { nSx, nSy2 };
            lp->BltNatural(plane1, x, y, &sz);
        }
        } break;
    case 2: { // Both
        lp->BltNatural(plane0, x, y);
        } break;
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

CGUISlider::CGUISlider() {
    ::SetRect(&m_rcRect, 0, 0, 800, 600);
    m_nItemNumX = 1;
    m_nItemNumY = 1;
    m_nType = 0;
    Reset();
}

void CGUISlider::Reset() {
    m_bDraged = false;
    m_nButton = 0;
    m_bIn = false;    
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
    int trackStartX = m_nX + m_nXOffset + m_rcRect.left;
    int trackStartY = m_nY + m_nYOffset + m_rcRect.top;

    m_pvSliderEvent->GetSliderSize(m_nItemNumX, m_nItemNumY, nSLX, nSLY);
    
    w = m_rcRect.right - m_rcRect.left - nSLX;
    h = m_rcRect.bottom - m_rcRect.top - nSLY;

    int thumbOffsetX = 0;
    int thumbOffsetY = 0;

    if (m_nItemNumX >= 2 && w > 0) {
        long long num = (long long)m_nSelectedItemX * w;
        long long den = (long long)m_nItemNumX - 1;
        thumbOffsetX = (int)(num / den);
    }
    if (m_nItemNumY >= 2 && h > 0) {
        long long num = (long long)m_nSelectedItemY * h;
        long long den = (long long)m_nItemNumY - 1;
        thumbOffsetY = (int)(num / den);
    }

    x = trackStartX + thumbOffsetX;
    y = trackStartY + thumbOffsetY;
}

LRESULT CGUISlider::OnSimpleMove(ISurface* lp) {
    if (m_pvMouse.get() == NULL) return -1;

    int nXX, nYY, nSLX, nSLY, w, h;
    CalcSliderPos(nXX, nYY, nSLX, nSLY, w, h); // nXX/nYY are Absolute Screen Coordinates of Thumb

    int mx, my, mb;
    m_pvMouse->GetInfo(mx, my, mb);

    GetXY(m_nX, m_nY);
    int mmx = mx - m_nX - m_nXOffset;
    int mmy = my - m_nY - m_nYOffset;

    bool bNGuard = !m_pvMouse->IsGuardTime();

    // Hit Testing
    bool bIn;
    LPRECT prc = GetRect();
    m_nInSlider = (prc->left <= mmx) && (mmx < prc->right) &&
                    (prc->top <= mmy) && (mmy < prc->bottom);

    if (m_bDraged) {
        bIn = true; 
    }
    else if (!m_nInSlider) {
        bIn = false;
    }
    else {
        if ((nXX <= mx) && (mx < nXX + nSLX) &&
            (nYY <= my) && (my < nYY + nSLY) &&
            m_pvSliderEvent->IsButton(mmx - nXX, mmy - nYY)) {
            bIn = true;
        }
        else {
            bIn = false;
        }
    }

    // --- REFACTORED SCROLL LOGIC FOR CENTER ALIGNMENT ---
    
    // Determine if we are clicking/holding
    bool bIsDown = (mb & 1) || (mb & 2);
    bool bWasDown = (m_nButton & 1) || (m_nButton & 2);
    bool bJustClicked = bIsDown && !bWasDown;

    // Determine if we are in "Track Scroll" mode
    // We are scrolling if mouse is down, we are in the slider rect, AND we aren't dragging the handle.
    // Crucially: If we just clicked, we must NOT have clicked the thumb (bIn must be false).
    // If we are holding, we continue scrolling even if the thumb moves under the mouse (bIn becomes true).
    bool bExecuteTrackScroll = false;

    if (bIsDown && m_nInSlider && !m_bDraged) {
        if (bJustClicked) {
             if (!bIn) bExecuteTrackScroll = true;
        } else {
             bExecuteTrackScroll = true;
        }
    }

    if (bExecuteTrackScroll && bNGuard) {
        // Calculate Absolute Center of the Thumb
        int thumbCenterX = nXX + (nSLX / 2);
        int thumbCenterY = nYY + (nSLY / 2);

        // Logic pivot: Use Center instead of Top-Left
        switch (m_nType) {
        case 0: // Vertical
            if (my < thumbCenterY) m_pvSliderEvent->OnPageUp();
            else if (my > thumbCenterY) m_pvSliderEvent->OnPageDown();
            break;
        case 1: // Horizontal  
            if (mx < thumbCenterX) m_pvSliderEvent->OnPageLeft();
            else if (mx > thumbCenterX) m_pvSliderEvent->OnPageRight();
            break;
        case 2: // Both
            if (my < thumbCenterY) m_pvSliderEvent->OnPageUp();
            else if (my > thumbCenterY) m_pvSliderEvent->OnPageDown();
            
            if (mx < thumbCenterX) m_pvSliderEvent->OnPageLeft();
            else if (mx > thumbCenterX) m_pvSliderEvent->OnPageRight();
            break;
        }
    }

    // Start dragging on left click (Only if we clicked ON the thumb)
    if (m_bIn && bIn && !(m_nButton & 2) && (mb & 2)) {
        if (!m_bDraged) {
            m_bDraged = true;
            m_nDragPosX = mx - nXX;
            m_nDragPosY = my - nYY;
        }
    }

    if (m_bDraged) {
        // Dragging Logic
        int x = mmx - m_rcRect.left - m_nDragPosX;
        int y = mmy - m_rcRect.top - m_nDragPosY;

        if (m_nItemNumX >= 2 && w != 0) {
            m_nSelectedItemX = (x * (m_nItemNumX - 1) + w / 2) / w;
        } else {
            m_nSelectedItemX = 0;
        }

        if (m_nSelectedItemX < 0) m_nSelectedItemX = 0;
        if (m_nSelectedItemX >= m_nItemNumX) m_nSelectedItemX = m_nItemNumX - 1;

        if (m_nItemNumY >= 2 && h != 0) {
            m_nSelectedItemY = (y * (m_nItemNumY - 1) + h / 2) / h;
        } else {
            m_nSelectedItemY = 0;
        }

        if (m_nSelectedItemY < 0) m_nSelectedItemY = 0;
        if (m_nSelectedItemY >= m_nItemNumY) m_nSelectedItemY = m_nItemNumY - 1;

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

    // Display Logic
    if (m_bDraged) {
        m_nPosX = mx - m_nDragPosX;
        m_nPosY = my - m_nDragPosY;
        if (m_nPosX < prc->left + m_nX + m_nXOffset) m_nPosX = prc->left + m_nX + m_nXOffset;
        if (m_nPosY < prc->top + m_nY + m_nYOffset) m_nPosY = prc->top + m_nY + m_nYOffset;
        if (m_nPosX + nSLX > prc->right + m_nX + m_nXOffset) m_nPosX = prc->right + m_nX + m_nXOffset - nSLX;
        if (m_nPosY + nSLY > prc->bottom + m_nY + m_nYOffset) m_nPosY = prc->bottom + m_nY + m_nYOffset - nSLY;
    }
    else {
        m_nPosX = nXX;
        m_nPosY = nYY;
    }

    return 0;
}

LRESULT CGUISlider::OnSimpleDraw(ISurface* lp) {
    if (m_pvSliderEvent.get() == NULL) return -1;
    
    // Pass the calculated positions (m_nPosX, m_nPosY) to the listener for drawing
    // We also pass the item counts (m_nItemNumX, m_nItemNumY) so the listener knows how to size the thumb
    return m_pvSliderEvent->OnDraw(lp, m_nPosX, m_nPosY, m_nItemNumX, m_nItemNumY);
}

} // namespace Draw
} // namespace yaneuraoGameSDK3rd