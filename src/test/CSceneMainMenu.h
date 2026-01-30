// CSceneMainMenu.cpp
// Created by derplayer
// Created on 2025-03-21 19:02:59

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
	CPlaneLoader m_vPlaneLoader;
    smart_ptr<ISurface> m_background;
    
	CSaturationCounter m_nFade;     // Fade effect counter
	CSaturationCounter m_nFade2;     // Fade effect counter
	CSaturationCounter m_nFade3;     // Fade effect counter
	CSaturationCounter m_nFadeButton;     // button Fade effect counter
	CSaturationCounter m_nButtonClickTracker;
	CTimer m_timer;
	CTimer m_timer2;

	// Main Menu buttons
	CGUIButton m_vButtons[6];
	int m_nButton;					// Selected button (0=none, 1=duel, 2=edit, 3=list, 4=setup, 5=quit)

	CPlane m_title1;
	CPlane m_title2;
	CPlane m_menu1;
	CPlane m_menu2;
	CPlane m_menu3;
	CPlane m_menu4;
	CPlane m_menu5;
	CPlane m_menu6;
	CPlane m_menu7;
};

#endif // CSCENEMAINMENU_H