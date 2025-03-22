// CSceneSplash.cpp
// Created by derplayer
// Created on 2025-03-21 01:25:41

#include "CSceneSplash.h"

void CSceneSplash::OnInit() {
    // Initialize input
    m_mouse.Flush();
    m_mouse.SetGuardTime(1);
}

void CSceneSplash::OnMove(const smart_ptr<ISurface>& lp) {
    key.Input();
	m_mouse.Flush(); // or buttons will stuck
}

void CSceneSplash::OnDraw(const smart_ptr<ISurface>& lp) {
	lp->Clear();
}