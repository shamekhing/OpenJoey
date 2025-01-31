//
// GUI Parts Base Class
//
// Created by derplayer
// Created on 2025-01-31 10:28:45

#ifndef __yaneGUIParts_h__
#define __yaneGUIParts_h__

// GUI Component Base Class
class IGUIParts {
public:
    IGUIParts() { 
        m_nX = m_nY = m_nXOffset = m_nYOffset = 0; 
    }
    virtual ~IGUIParts() {}

    // Coordinate settings
    virtual void SetXY(int x, int y) { m_nX = x; m_nY = y; }
    virtual void GetXY(int& x, int& y) { x = m_nX; y = m_nY; }
    virtual void SetXYOffset(int x, int y) { m_nXOffset = x; m_nYOffset = y; }
    virtual void GetXYOffset(int& x, int& y) { x = m_nXOffset; y = m_nYOffset; }

    // Called every frame for actual usage
    // IMouse is assumed to be flushed externally
    virtual LRESULT OnDraw(ISurface* lp) {
        LRESULT l = 0;
        l |= OnMove(lp);
        l |= Draw(lp);
        return l;
    }

    // Split into movement and drawing phases like the scene system
    virtual LRESULT OnMove(ISurface* lp) { return 0; }  // Movement/logic phase
    virtual LRESULT Draw(ISurface* lp) { return 0; }    // Drawing phase only

    // Mouse settings
    virtual void SetMouse(smart_ptr<IMouse> pv) { Reset(); m_pvMouse = pv; }
    virtual smart_ptr<IMouse> GetMouse() { return m_pvMouse; }

protected:
    virtual void Reset() {}  // State reset
    smart_ptr<IMouse> m_pvMouse;

    // Drawing coordinates
    int m_nX;        // X position
    int m_nY;        // Y position
    int m_nXOffset;  // X offset for drawing
    int m_nYOffset;  // Y offset for drawing
};

//////////////////////////////////////////////////////////////////////////////

#endif