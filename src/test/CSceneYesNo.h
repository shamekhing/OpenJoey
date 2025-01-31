// CSceneYesNo.h
// Created by derplayer
// Created on 2025-01-26 23:01:28

#ifndef CSCENEYESNO_H
#define CSCENEYESNO_H

#include "../stdafx.h"
#include "../CApp.h"
#include "../system/CJoeySceneFactory.h"
#include "../system/backport/yaneGUIButton.h"

class CSceneYesNo : public CBaseScene {
public:
    virtual void OnInit();
    virtual void OnDraw(const smart_ptr<ISurface>& lp);
    virtual void OnMove(const smart_ptr<ISurface>& lp);
    virtual void Serialize(ISerialize&) {}

private:
    CKey1 key;
    CFixMouse m_mouse;
    CPlane m_vBackground;      // Background capture
    CPlaneLoader m_vPlaneLoader;
    CPlane m_pMessageSurface;  // "Exit?" message surface
    
    CGUIButton m_vButtons[2];  // Yes/No buttons
    //CGUINormalButtonListener m_vButtonEvents[2];  // Button event handlers
    
    int m_nButton;            // Selected button (0=none, 1=yes, 2=no)
    CRootCounter m_nFade;     // Fade effect counter
};

#endif // CSCENEYESNO_H