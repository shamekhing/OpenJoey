// CSceneMainMenu.cpp
// Created by derplayer
// Created on 2025-03-21 13:42:24

#include "CSceneMainMenu.h"

void CSceneMainMenu::OnInit() {
    // Initialize input
    m_mouse.Flush();
    m_mouse.SetGuardTime(1);
}

void CSceneMainMenu::OnMove(const smart_ptr<ISurface>& lp) {
    key.Input();
	m_mouse.Flush(); // or buttons will stuck
}

void CSceneMainMenu::OnDraw(const smart_ptr<ISurface>& lp) {

}