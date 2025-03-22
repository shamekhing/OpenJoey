// CSceneSettings.cpp
// Created by derplayer
// Created on 2025-03-22 21:35:28

#include "CSceneSettings.h"

void CSceneSettings::OnInit() {
    // Initialize input
    m_mouse.Flush();
    m_mouse.SetGuardTime(1);

	// Init props
	m_nFade = CSaturationCounter(0, 255, 8);
	m_timerMain = CTimer();
	m_timerMain.Restart();
	m_timerMain.Reset(); //always running

	// Clone fb from menu
	smart_ptr<ISurface> screenPtr = app->GetDraw()->GetSecondary()->cloneFull();
	if (screenPtr.get()) {
		m_background = screenPtr;
		OutputDebugStringA("Framebuffer data cloned and loaded with successfuly!\n");
	}

	// Load options screen define
	m_vPlaneLoader.SetLang(app->GetLang());
	m_vPlaneLoader.SetReadDir("data/y/title/");  // Base directory
	if (m_vPlaneLoader.Set("data/y/title/option.txt", false) != 0) {  // Relative to SetReadDir
		OutputDebugStringA("Error: Failed to load data/y/title/option.txt\n");
	}

	m_settingsBackdrop = m_vPlaneLoader.GetPlane(0);
	m_settingsBackdrop->SetPos(m_vPlaneLoader.GetXY(0));
}

void CSceneSettings::OnMove(const smart_ptr<ISurface>& lp) {
    key.Input();
	m_mouse.Flush(); // or buttons will stuck
}

void CSceneSettings::OnDraw(const smart_ptr<ISurface>& lp) {
	//lp->Clear();
	//lp->BltFast(m_background.get(), 0, 0); // always render cached framebuffer

	if(m_timerMain.Get() > 0)
		lp->BlendBltFast(m_settingsBackdrop, m_settingsBackdrop->GetPosX(), m_settingsBackdrop->GetPosY(), m_nFade);

	m_nFade.Inc();
}