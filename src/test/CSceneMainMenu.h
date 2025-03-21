// CSceneMainMenu.cpp
// Created by derplayer
// Created on 2025-03-21 13:42:24

#ifndef CSCENEMAINMENU_H
#define CSCENEMAINMENU_H

#include "../stdafx.h"
#include "../CApp.h"
#include "../system/CJoeySceneFactory.h"
#include "../system/backport/yaneGUIButton.h"

class CSceneMainMenu : public CBaseScene {
public:
    virtual void OnInit();
    virtual void OnDraw(const smart_ptr<ISurface>& lp);
    virtual void OnMove(const smart_ptr<ISurface>& lp);
    virtual void Serialize(ISerialize&) {}

private:
    CKey1 key;
    CFixMouse m_mouse;
    smart_ptr<ISurface> m_background;
    
    CRootCounter m_nFade;     // Fade effect counter
};

#endif // CSCENEMAINMENU_H