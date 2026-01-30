// CSceneSettings.h
// Created by derplayer
// Created on 2025-03-22 21:35:28

#ifndef CSCENESETTINGS_H
#define CSCENESETTINGS_H

#include "../stdafx.h"
#include "../CApp.h"
#include "../system/CJoeySceneFactory.h"
#include "../system/backport/yaneGUIButton.h"
#include "../system/backport/yaneGUISlider.h"

class CSceneSettings : public CBaseScene {
public:
    virtual void OnInit();
    virtual void OnDraw(const smart_ptr<ISurface>& lp);
    virtual void OnMove(const smart_ptr<ISurface>& lp);
    virtual void Serialize(ISerialize&) {}
    virtual ~CSceneSettings();

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

	// option graphics (CPlane holds ownership so surfaces are not dangling from GetPlane() temporary)
	CPlane m_settingsBackdrop;
	CPlane m_settingsWindowBtn;
	CPlane m_settingsFullscreenBtn;
	CPlane m_settingsBitBtn;
	CPlane m_settingsBackBtn;

	// TODO: we dont have slider UI code yet (unused; kept as CPlane for consistency if re-enabled)
	CPlane m_settingsVolumeSlider1;
	CPlane m_settingsVolumeSlider2;
	CPlane m_settingsVolumeSlider3;

    CPlane m_sliderTop;
    CPlane m_sliderMiddle;
    CPlane m_sliderBottom;
    CFastPlane m_sliderNormal;    // Listener holds smart_ptr to this; must outlive slider
    CFastPlane m_sliderHover;
    CTimer m_timerSliderPress;
    smart_ptr<CGUISlider> m_volumeSlider;  // Destroy before m_sliderNormal so listener is destroyed while m_sliderNormal still exists
    bool m_sliderInitialized;
};

#endif // CSCENESETTINGS_H