// CSceneSplash.cpp
// Created by derplayer
// Created on 2025-03-21 01:25:41

#include "CSceneSplash.h"

void CSceneSplash::OnInit() {
    // Initialize input
    m_mouse.Flush();
    m_mouse.SetGuardTime(1);
	IsReadToLoadMenu = false;

	// Fade timer
	m_nFade = CRootCounter(0, 255, 8);
    m_nPhase = CRootCounter(0, 6, 1);
	m_timer = CTimer();
	m_timer.Reset();

	// Load splash screen define
	m_vPlaneLoader.SetLang(app->GetLang());
    m_vPlaneLoader.SetReadDir("data/y/title/");  // Base directory
    if (m_vPlaneLoader.Set("data/y/title/title.txt", false) != 0) {  // Relative to SetReadDir
        OutputDebugStringA("Error: Failed to load data/y/title/title.txt\n");
    }

    m_splash0 = m_vPlaneLoader.GetPlane(0);  // Plane 0 = first line splash asset

	if(app->GetLang() == "e")
		m_splash1 = m_vPlaneLoader.GetPlane(1);  // default splash
	else
		m_splash1 = m_vPlaneLoader.GetPlane(2);  // special demo trial splash (TODO: configurable?)

}

void CSceneSplash::OnMove(const smart_ptr<ISurface>& lp) {
    key.Input();
	m_mouse.Flush(); // or buttons will stuck
}

void CSceneSplash::OnDraw(const smart_ptr<ISurface>& lp) {

	lp->Clear();

	// TODO: this is wrong and results in invalid brightness fade step but i am to tired now its 3am, fix in future
	if (m_timer.Get() < 1000) m_nFade.Inc();
	if (m_timer.Get() > 4000) m_nFade.Dec();
	
	if (m_nFade.IsEnd())
	{
		if(IsReadToLoadMenu)
		{
			GetSceneControl()->ReturnScene();
			return;
		}

		// Reset timer and swap splash for the second screen
		m_timer.Reset();
		m_nFade.Reset();
		m_splash0 = m_splash1;
		IsReadToLoadMenu = true;
	}

	lp->BlendBltFast(m_splash0, 0, 0, m_nFade);
	
}