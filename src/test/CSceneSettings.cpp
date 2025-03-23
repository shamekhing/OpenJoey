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

	//m_settingsWindowBtn = m_vPlaneLoader.GetPlane(1);
	//m_settingsWindowBtn->SetPos(m_vPlaneLoader.GetXY(1));
	//m_settingsFullscreenBtn = m_vPlaneLoader.GetPlane(2);
	//m_settingsFullscreenBtn->SetPos(m_vPlaneLoader.GetXY(2));
	//m_settingsBitBtn = m_vPlaneLoader.GetPlane(3);
	//m_settingsBitBtn->SetPos(m_vPlaneLoader.GetXY(3));

	// INFO: alpha channel. do not use fast draw calls for those.
	m_settingsWindowBtnEffect = m_vPlaneLoader.GetPlane(4);
	m_settingsWindowBtnEffect->SetPos(m_vPlaneLoader.GetXY(4));
	m_settingsFullscreenBtnEffect = m_vPlaneLoader.GetPlane(5);
	m_settingsFullscreenBtnEffect->SetPos(m_vPlaneLoader.GetXY(5));
	m_settingsBitBtn16Effect = m_vPlaneLoader.GetPlane(6);
	m_settingsBitBtn16Effect->SetPos(m_vPlaneLoader.GetXY(6));
	m_settingsBitBtn24Effect = m_vPlaneLoader.GetPlane(7);
	m_settingsBitBtn24Effect->SetPos(m_vPlaneLoader.GetXY(7));
	m_settingsBitBtn32Effect = m_vPlaneLoader.GetPlane(8);
	m_settingsBitBtn32Effect->SetPos(m_vPlaneLoader.GetXY(8));

	//m_settingsBackBtn = m_vPlaneLoader.GetPlane(9);
	//m_settingsBackBtn->SetPos(m_vPlaneLoader.GetXY(9));

	//m_settingsVolumeSlider1 = m_vPlaneLoader.GetPlane(10);
	//m_settingsVolumeSlider1->SetPos(m_vPlaneLoader.GetXY(10));
	//m_settingsVolumeSlider2 = m_vPlaneLoader.GetPlane(11);
	//m_settingsVolumeSlider2->SetPos(m_vPlaneLoader.GetXY(11));
	//m_settingsVolumeSlider3 = m_vPlaneLoader.GetPlane(12);
	//m_settingsVolumeSlider3->SetPos(m_vPlaneLoader.GetXY(12));

	// Create an array of button IDs
    int buttonIds[] = {1, 2, 9, 10, 11};
    const int buttonCount = sizeof(buttonIds) / sizeof(buttonIds[0]); // Calculate array size

	// Setup buttons
	for (int j = 0; j < buttonCount; ++j) {
        int i = buttonIds[j]; // Get the current ID
		CGUIButton* btn = new CGUIButton();
		btn->SetID(i);
		btn->SetMouse(smart_ptr<CFixMouse>(&m_mouse, false));

		// Create the button listener as CGUIButtonEventListener type directly
		smart_ptr<CGUIButtonEventListener> buttonListener(new CGUINormalButtonListener());

		// Cast to derived type to access CGUINormalButtonListener methods
		CGUINormalButtonListener* p = static_cast<CGUINormalButtonListener*>(buttonListener.get());

		// Setup button plane
		CPlane pln = m_vPlaneLoader.GetPlane(i); // alpha channel - so no FastPlanes
		pln->SetPos(m_vPlaneLoader.GetXY(i)); // rendering pos
		smart_ptr<ISurface> plnPtr(pln.get(), false); // no ownership
		p->SetPlane(plnPtr);
		btn->SetEvent(buttonListener);
		btn->SetXY(pln->GetPosX(), pln->GetPosY()); // collision pos

		// Veritical button to be sliced via BUTTON_SPACING!
		if(i == 3)
		{
			// TODO: this is so unfinished
			const int BUTTON_SPACING = 80; // BUTTON_BITS / 3
			RECT boundsRect = { 0, i * BUTTON_SPACING, pln->GetPosY(), (i + 1) * BUTTON_SPACING };
			btn->SetBounds(boundsRect);
			//m_vButtonsBit
			continue;
		}

		// Insert btn into smart pointer vector list
		smart_ptr<CGUIButton> btnSmartPtr(btn);
		m_vButtons.insert(btnSmartPtr);
	}
}

void CSceneSettings::OnMove(const smart_ptr<ISurface>& lp) {
	key.Input();
	m_mouse.Flush(); // or buttons will stuck

	// Handle Space debug key
    if (key.IsKeyPush(5)) {  GetSceneControl()->ReturnScene(); }

	// Update buttons
	int index = 0;
	for (smart_vector_ptr<CGUIButton>::iterator it = m_vButtons.begin(); it != m_vButtons.end(); ++it, ++index) {
		CGUIButton* button = it->get();
		if (button) {
			button->OnSimpleMove(lp.get());
			CGUIButtonEventListener* e	= button->GetEvent().get();
			CGUINormalButtonListener* p	= (CGUINormalButtonListener*)e;

			if (m_nButton == 0 && button->IsLClick())
			{
				m_nButton = button->GetID(); // Selected button (0=none, .......................
			}

			if (button->IsIn())
			{
				//p->SetImageOffset(1);
			}
		}
	}

	switch(m_nButton) {
		case 1: //WINDOW
			break;
		case 2: //FULLSCREEN
			break;
		case 3: //BITCOUNT
			break;
		case 9: //BACK
			GetSceneControl()->ReturnScene();
			break;
		case 10: //VOL_MINUS
			break;
		case 11: //VOL_PLUS
			break;
		default:
			break;
	}
	m_nButton = 0;
}

void CSceneSettings::OnDraw(const smart_ptr<ISurface>& lp) {
	//lp->Clear();
	//lp->BltFast(m_background.get(), 0, 0); // render cached framebuffer (this can restore FB when something overrides it)

	if(m_timerMain.Get() > 0) {
		lp->BlendBltFast(m_settingsBackdrop, m_settingsBackdrop->GetPosX(), m_settingsBackdrop->GetPosY(), m_nFade);
	}

	if(m_timerMain.Get() > 200) {
		//lp->BlendBltFast(m_settingsWindowBtn, m_settingsWindowBtn->GetPosX(), m_settingsWindowBtn->GetPosY(), m_nFade);
		//lp->BlendBltFast(m_settingsFullscreenBtn, m_settingsFullscreenBtn->GetPosX(), m_settingsFullscreenBtn->GetPosY(), m_nFade);
		//lp->BlendBltFast(m_settingsBitBtn, m_settingsBitBtn->GetPosX(), m_settingsBitBtn->GetPosY(), m_nFade);

		//lp->BlendBltFast(m_settingsBackBtn, m_settingsBackBtn->GetPosX(), m_settingsBackBtn->GetPosY(), m_nFade);

		//lp->BlendBlt(m_settingsBitBtn32Effect, m_settingsBitBtn32Effect->GetPosX(), m_settingsBitBtn32Effect->GetPosY(), m_nFade);
	}

	// Draw settings overlays (TODO: in real game its INVERTED...)
	if(app->GetSettings()->WindowMode == true)
		lp->Blt(m_settingsWindowBtnEffect, m_settingsWindowBtnEffect->GetPosX(), m_settingsWindowBtnEffect->GetPosY());
	else
		lp->Blt(m_settingsFullscreenBtnEffect, m_settingsFullscreenBtnEffect->GetPosX(), m_settingsFullscreenBtnEffect->GetPosY());

	//Draw buttons
	const int buttonCount = 6; // Total number of buttons

	// Get the dimensions of the source surface
	int width = 0, height = 0;

	int index = 0;
	for (smart_vector_ptr<CGUIButton>::iterator it = m_vButtons.begin(); it != m_vButtons.end(); ++it, ++index) {
		CGUIButton* button = it->get();
		if (button) {
			// Button event cast
			CGUIButtonEventListener* e	= button->GetEvent().get();
			CGUINormalButtonListener* p	= (CGUINormalButtonListener*)e;

			ISurface* originalSurface = button->GetPlane();
			originalSurface->GetSize(width, height); // Ensure variables match expected types

			// Define the source rectangle for the current slice
			//RECT sourceRect = { 0, i * sliceHeight, width, (i + 1) * sliceHeight };

			//// Calculate the destination position on the target surface (lp)
			//int destX = BUTTON_X; // X position remains constant
			//int destY = BUTTON_Y + (i * sliceHeight); // Increment Y for each button

			// Blit the button surface onto the primary surface
			if (button->IsIn())
			{
				lp->Blt(originalSurface, originalSurface->GetPosX(), originalSurface->GetPosY());
			}
		}
	}

	m_nFade.Inc();
}