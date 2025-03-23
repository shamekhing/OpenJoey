// CSceneSettings.h
// Created by derplayer
// Created on 2025-03-22 21:35:28

#ifndef CSCENESETTINGS_H
#define CSCENESETTINGS_H

#include "../stdafx.h"
#include "../CApp.h"
#include "../system/CJoeySceneFactory.h"
#include "../system/backport/yaneGUIButton.h"

class CSceneSettings : public CBaseScene {
public:
    virtual void OnInit();
    virtual void OnDraw(const smart_ptr<ISurface>& lp);
    virtual void OnMove(const smart_ptr<ISurface>& lp);
    virtual void Serialize(ISerialize&) {}

private:
    CKey1 key;
    CFixMouse m_mouse;
    smart_ptr<ISurface> m_background;
    CPlaneLoader m_vPlaneLoader;

    CRootCounter m_nRootFade;	// Fade effect counter
	CSaturationCounter m_nFade;
	CTimer m_timerMain;

	smart_vector_ptr<CGUIButton> m_vButtons; // we vector now, fuck arrays :^)
	int m_nButton;	// Selected button

	// option graphics
	ISurface* m_settingsBackdrop;
	ISurface* m_settingsWindowBtn;
	ISurface* m_settingsFullscreenBtn;
	ISurface* m_settingsBitBtn;
	ISurface* m_settingsBackBtn;

	// TODO: we dont have slider UI code yet
	ISurface* m_settingsVolumeSlider1;
	ISurface* m_settingsVolumeSlider2;
	ISurface* m_settingsVolumeSlider3;
};

#endif // CSCENESETTINGS_H