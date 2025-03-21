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
	m_nFade = CSaturationCounter(0, 255, 8);
    m_nPhase = CRootCounter(0, 6, 1);
	m_timer = CTimer();
	m_timer.Reset();
	m_timer.Pause();

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
	//lp->Clear();
	
	if (m_nFade.IsEnd() == false && m_timer.IsPause()){
		m_nFade.Inc();
	}
	else {
		if(m_timer.IsPause() == true) {
			m_timer.Restart();
		}
	}

	if(m_timer.Get() > 2000)
	{
		// Start deincrement
		m_nFade.Dec();

		if(m_nFade.IsBegin()){

			// The swap already happen so we are at stage 2 of splash
			if(m_splash0 == m_splash1){
				IsReadToLoadMenu = true;
			}

			// Reinit timer for second splash after de-fade is done
			m_timer.Reset();
			m_timer.Pause();
			m_splash0 = m_splash1; // swap splash after finish
		}
	}

	if(IsReadToLoadMenu)
	{
		GetSceneControl()->ReturnScene();
		return;
	}

	lp->BlendBltFast(m_splash0, 0, 0, m_nFade);
}