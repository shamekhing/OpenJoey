// CSceneYesNo.cpp
// Created by derplayer
// Created on 2025-01-26 23:01:28

#include "CSceneYesNo.h"

void CSceneYesNo::OnInit() {
    // Initialize input
    m_mouse.Flush();
    m_mouse.SetGuardTime(1);

    // Load resources
    m_vPlaneLoader.SetReadDir("test/yesno/");
    m_vPlaneLoader.Set("list.txt", true);

    // Load background and message
    m_vBackground = CPlane(app->GetDraw()->GetSecondary());  // Capture current screen
    //m_vBackground->SubColorBltFastFade(m_vBackground.get(), 0, 0, 172);  // Darken
    m_pMessageSurface = m_vPlaneLoader.GetPlane(0);  // Load message

    // Setup buttons
    static const int BUTTON_Y = 240;
    static const int BUTTON_SPACING = 120;
    
    for(int i = 0; i < 2; i++) {
        m_vButtons[i].SetMouse((CMouse*)&m_mouse);

		CGUINormalButtonListener *p = new CGUINormalButtonListener;
        p->SetPlaneLoader(&m_vPlaneLoader, 1 + i*2); // Image offsets: Yes=1,2 No=3,4
        p->SetType(1);  // Normal button type

		m_vButtons[i].SetEvent(p);
        m_vButtons[i].SetXY(216 + i*BUTTON_SPACING, BUTTON_Y);  // Yes at 216, No at 336
    }

    m_nButton = 0;
    m_nFade.Set(0, 16, 1);  // 16 frames fade
}

void CSceneYesNo::OnMove(const smart_ptr<ISurface>& lp) {
    key.Input();

    // Handle ESC key
    if (key.IsKeyPush(VK_ESCAPE)) {
        m_nButton = 2;  // No
        return;
    }

    // Update buttons
    for(int i = 0; i < 2; i++) {
        m_vButtons[i].OnMove(lp.get());
        
        // Check for button clicks
        if (m_nButton == 0 && m_vButtons[i].IsLClick()) {
            m_nButton = i + 1;
			/*
			CGUIButtonEventListener* e	= m_vButtons[i].GetEvent();
			CGUINormalButtonListener* p	= (CGUINormalButtonListener*)e;
			p->SetType(32);
			p->SetImageOffset(2);
			*/
        }
    }
}

void CSceneYesNo::OnDraw(const smart_ptr<ISurface>& lp) {
	return;
    // Draw darkened background
    lp->BltFast(m_vBackground.get(), 0, 0);

    // Draw message centered
    int sx, sy;
    m_pMessageSurface->GetSize(sx, sy);
    lp->BltNatural(m_pMessageSurface.get(), 320 - sx/2, 200 - sy/2);

    // Draw buttons
    for(int i = 0; i < 2; i++) {
        m_vButtons[i].OnDraw(lp.get());
    }

    // Handle fade out effect when button is selected
    if (m_nButton != 0) {
        m_nFade++;
        if (m_nFade.IsEnd()) {
            if (m_nButton == 1) {
                GetSceneControl()->ReturnScene();
            }
            return;
        }

        // Apply fade effect
        BYTE fadeAlpha = (BYTE)(255 - ((int)m_nFade * 16));
        lp->BlendBltFast(m_vBackground.get(), 0, 0, fadeAlpha);
    }
}