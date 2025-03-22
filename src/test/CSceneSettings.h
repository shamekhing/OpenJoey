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

    CRootCounter m_nRootFade;     // Fade effect counter
	CSaturationCounter m_nFade;
	CTimer m_timerMain;

	CGUIButton m_vButtons[11];		// Theoreticly 8 because of volume bars but TODO
	int m_nButton;					// Selected button (0=none, 1=x, 2=x, 3=x, 4=x, 5=x)

	ISurface* m_settingsBackdrop;

	// selected overlay effects
	ISurface* m_settingsWindowBtn;
	ISurface* m_settingsFullscreenBtn;
	ISurface* m_settingsBitBtn; // TODO: this needs veritical RECT slicing

	// hover overlay effect
	ISurface* m_settingsWindowBtnEffect;
	ISurface* m_settingsFullscreenBtnEffect;
	ISurface* m_settingsBitBtn16Effect;
	ISurface* m_settingsBitBtn24Effect;
	ISurface* m_settingsBitBtn32Effect;
	ISurface* m_settingsBackBtn;

	// TODO: we dont have slider UI code yet
	ISurface* m_settingsVolumeSlider1;
	ISurface* m_settingsVolumeSlider2;
	ISurface* m_settingsVolumeSlider3;
};

#endif // CSCENESETTINGS_H