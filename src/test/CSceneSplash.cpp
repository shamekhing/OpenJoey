// CSceneSplash.cpp
// Created by derplayer
// Created on 2025-03-21 01:25:41
#include "stdafx.h"
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

	// Load splash screen define from data/y/title/
	m_vPlaneLoader.SetLang(app->GetLang());
	m_vPlaneLoader.SetReadDir("data/y/title/");
    LRESULT setResult = m_vPlaneLoader.Set("data/y/title/title.txt", false);
    m_splashLoadFailed = (setResult != 0);
    m_framesNoSplash = 0;
    if (setResult != 0) {
        OutputDebugStringA("Error: Failed to load data/y/title/title.txt (check data is next to exe)\n");
    }

    m_splash0 = m_vPlaneLoader.GetPlane(0);  // Plane 0 = first line splash asset

	if(app->GetLang() == "e")
		m_splash1 = m_vPlaneLoader.GetPlane(1);  // default splash
	else
		m_splash1 = m_vPlaneLoader.GetPlane(2);  // special demo trial splash (TODO: configurable?)

	if (m_splash0.get() == nullptr) m_splashLoadFailed = true;
}

void CSceneSplash::OnMove(const smart_ptr<ISurface>& lp) {
    key.Input();
	m_mouse.Flush(); // or buttons will stuck
}

void CSceneSplash::OnDraw(const smart_ptr<ISurface>& lp) {
	lp->Clear();
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
		GetSceneControl()->JumpScene(SCENE_MAINMENU);
		return;
	}

	// When load failed, GetPlane(0) is still non-null (CSurfaceNullDevice), so we must check m_splashLoadFailed first to run the skip-to-menu path
	if (m_splashLoadFailed) {
		// Title assets missing or wrong path: skip to main menu after ~1.5s
		m_framesNoSplash++;
		if (m_framesNoSplash > 45) {
			GetSceneControl()->JumpScene(SCENE_MAINMENU);
		}
	} else if (m_splash0.get() != nullptr) {
		lp->BlendBltFast(m_splash0.get(), 0, 0, m_nFade);
	}
}