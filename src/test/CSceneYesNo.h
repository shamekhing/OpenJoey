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
    smart_ptr<ISurface> m_vBackground;      // Background capture
	ISurface* m_vBackground1;      // Background capture
	CFastPlane m_vFastBackground;      // Background capture1
	CFastPlane* m_vFastBackground1;
    CPlaneLoader m_vPlaneLoader;
    CPlane m_pMessageSurface;  // "Exit?" message surface
    
	CFastPlane* m_lastFrameFB;

    CGUIButton m_vButtons[2];  // Yes/No buttons
    //CGUINormalButtonListener m_vButtonEvents[2];  // Button event handlers
    
    int m_nButton;            // Selected button (0=none, 1=yes, 2=no)
    CRootCounter m_nFade;     // Fade effect counter
	bool IsSetLeva;
};

#endif // CSCENEYESNO_H