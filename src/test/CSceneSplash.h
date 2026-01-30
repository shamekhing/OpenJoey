// CSceneSplash.h
// Created by derplayer
// Created on 2025-03-21 01:25:41

#ifndef CSCENESPLASH_H
#define CSCENESPLASH_H

#include "../stdafx.h"
#include "../CApp.h"
#include "../system/CJoeySceneFactory.h"
#include "../system/backport/yaneGUIButton.h"

class CSceneSplash : public CBaseScene {
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
	CPlane m_splash0;  // hold ownership so surface is not freed when temporary from GetPlane() goes out of scope
	CPlane m_splash1;

    CSaturationCounter m_nFade;     // Fade effect counter
	CRootCounter m_nPhase;     // Fade effect phase
	CTimer m_timer;
	bool SplashTimerWait;
	bool IsReadToLoadMenu;
	bool m_splashLoadFailed;   // true when title.txt failed to load -> skip to menu after short delay
	int m_framesNoSplash;      // frames drawn with no splash; when high enough, jump to main menu
};

#endif // CSCENESPLASH_H