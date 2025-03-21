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

	// Main Menu buttons
	CGUIButton m_vButtons[6];
	int m_nButton;					// Selected button (0=none, 1=duel, 2=edit, 3=list, 4=setup, 5=quit)

	ISurface* m_title1;
	ISurface* m_title2;

	ISurface* m_menu1;
	ISurface* m_menu2;
	ISurface* m_menu3;
	ISurface* m_menu4;
	ISurface* m_menu5;
	ISurface* m_menu6;
	ISurface* m_menu7;
};

#endif // CSCENEMAINMENU_H