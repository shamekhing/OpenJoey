// CSceneSettings.cpp
// Created by derplayer
// Created on 2025-03-22 21:35:28
#include "stdafx.h"
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
	if (m_settingsBackdrop.get()) m_settingsBackdrop->SetPos(m_vPlaneLoader.GetXY(0));

	// Not buttons, so are rendered manualy in Draw tick
	m_settingsWindowBtn = m_vPlaneLoader.GetPlane(1);
	if (m_settingsWindowBtn.get()) m_settingsWindowBtn->SetPos(m_vPlaneLoader.GetXY(1));
	m_settingsFullscreenBtn = m_vPlaneLoader.GetPlane(2);
	if (m_settingsFullscreenBtn.get()) m_settingsFullscreenBtn->SetPos(m_vPlaneLoader.GetXY(2));
	m_settingsBitBtn = m_vPlaneLoader.GetPlane(3);
	if (m_settingsBitBtn.get()) m_settingsBitBtn->SetPos(m_vPlaneLoader.GetXY(3));

	//m_settingsVolumeSlider1 = m_vPlaneLoader.GetPlane(10);
	//m_settingsVolumeSlider1->SetPos(m_vPlaneLoader.GetXY(10));
	//m_settingsVolumeSlider2 = m_vPlaneLoader.GetPlane(11);
	//m_settingsVolumeSlider2->SetPos(m_vPlaneLoader.GetXY(11));
	//m_settingsVolumeSlider3 = m_vPlaneLoader.GetPlane(12);
	//m_settingsVolumeSlider3->SetPos(m_vPlaneLoader.GetXY(12));

	// Create an array of button IDs
    int buttonIds[] = {4, 5, 6, 7, 8, 9, 10, 11};
    const int buttonCount = sizeof(buttonIds) / sizeof(buttonIds[0]); // Calculate array size
	m_vPlaneLoader.SetColorKey(ISurface::makeRGB(0, 255, 0, 128)); // alpha value because transparency logic will mismatch, code is expecting RGBA and not RGB

	// Setup buttons
	for (int j = 0; j < buttonCount; ++j) {
        int i = buttonIds[j]; // Get the current ID
		CGUIButton* btn = new CGUIButton();
		btn->SetID(i);
		btn->SetMouse(smart_ptr<CFixMouse>(&m_mouse, false)); // Non-owning: m_mouse outlives scene

		// Create the button listener as CGUIButtonEventListener type directly
		smart_ptr<CGUIButtonEventListener> buttonListener(new CGUINormalButtonListener());

		// Cast to derived type to access CGUINormalButtonListener methods
		CGUINormalButtonListener* p = static_cast<CGUINormalButtonListener*>(buttonListener.get());

		// Setup button plane
		CPlane pln = m_vPlaneLoader.GetPlane(i);
		POINT pos = m_vPlaneLoader.GetXY(i);
		pln->SetPos(pos); // rendering pos
		smart_ptr<ISurface> plnPtr(pln.get(), false); // no ownership
		p->SetPlane(plnPtr);
		btn->SetEvent(buttonListener);
		btn->SetXY(pln->GetPosX(), pln->GetPosY()); // collision pos

		// Manual calculate the bounding rectangle - only use when really needed like for sliced gfx.
		//SIZE surfSize = pln->GetSurfaceInfo()->GetSize();
		//RECT boundsRect = { 0, 0, surfSize.cx, surfSize.cy };
		//btn->SetBounds(boundsRect);

		// Vol+/- are sliced and need custom bounds
		if(i == 10 || i == 11){
			POINT posVol = m_vPlaneLoader.GetXY(12); //HACKO (only use x value!)
			RECT boundsRect = { 0, 0, posVol.x, posVol.x };
			btn->SetBounds(boundsRect);
		}

		// Insert btn into smart pointer vector list
		smart_ptr<CGUIButton> btnSmartPtr(btn);
		m_vButtons.insert(btnSmartPtr);
	}

    // Volume Slider test
    m_sliderTop = m_vPlaneLoader.GetPlane(10);
    m_sliderMiddle = m_vPlaneLoader.GetPlane(11);
    m_sliderBottom = m_vPlaneLoader.GetPlane(12);

    if (m_sliderTop.get()) m_sliderTop->SetPos(m_vPlaneLoader.GetXY(10));
    if (m_sliderMiddle.get()) m_sliderMiddle->SetPos(m_vPlaneLoader.GetXY(11));
    if (m_sliderBottom.get()) m_sliderBottom->SetPos(m_vPlaneLoader.GetXY(12));

    // Create slider
    m_volumeSlider = new CGUISlider();

    // Create the slider listener
    smart_ptr<CGUISliderEventListener> sliderListener(new CGUINormalSliderListener());
    CGUINormalSliderListener* p = static_cast<CGUINormalSliderListener*>(sliderListener.get());
	POINT sizeInfo = m_vPlaneLoader.GetXY(12);  // This will give slider x/y size (inverted, yes its this hacky specified in TXT)
	sliderListener->SetMinSize(sizeInfo.y, sizeInfo.x);

    if (m_sliderTop.get() && m_sliderMiddle.get() && m_sliderBottom.get()) {
		// Get positions from the plane loader coordinates
		POINT leftPos = m_vPlaneLoader.GetXY(10);   // Position of left cap
		POINT rightPos = m_vPlaneLoader.GetXY(11);  // Position of right cap

		// For slicing
	    int buttonSize = sizeInfo.x;    // Width AND height of the arrow buttons
		int sliderWidth = sizeInfo.y;   // Width of the slider graphic
		int buttonHeight = sizeInfo.x*2;  // Height is same as buttonSize for ALL elements
		//##---
		int sliderWidthOffset = sizeInfo.y;
		// Define normal and clicked slider dynamically
		// Adjusted slider buttons (1x2 setup)
		RECT normalSlider  = { sizeInfo.x * 2, 0,			sizeInfo.x * 2 + sliderWidthOffset, sizeInfo.x	   };  // Last entry in row 0
		RECT clickedSlider = { sizeInfo.x * 2, sizeInfo.x,  sizeInfo.x * 2 + sliderWidthOffset, sizeInfo.x * 2 };  // Last entry in row 1

        // Setup the planes directly in the listener
        CPlane pln = m_vPlaneLoader.GetPlane(10); // Use slider entry for pre-slicing
		m_sliderNormal.CreateSurface(sliderWidth, buttonHeight, false);  
		//normalSlider.top *= 2;
		m_sliderNormal.BltFast(pln.get(), 0, 0, NULL, &normalSlider);

		smart_ptr<ISurface> plnPtr(&m_sliderNormal, false); // no ownership
        p->SetPlane(plnPtr);
        
        // Configure the slider
        m_volumeSlider->SetEvent(sliderListener);
        

		RECT rc;
		SetRect(&rc,
			leftPos.x + sizeInfo.x,	  // Left position X
			leftPos.y,                // Left position Y
			rightPos.x,               // Right position X
			leftPos.y + sizeInfo.x    // Use height from size specification
		);

		m_volumeSlider->SetMouse(smart_ptr<CFixMouse>(&m_mouse, false)); // Non-owning: m_mouse outlives scene
		m_volumeSlider->SetRect(&rc);                                     // Then set position
		m_volumeSlider->SetType(1);                                       // Make it horizontal
		m_volumeSlider->SetItemNum(101, 0);                               // Set range
		m_volumeSlider->SetSelectedItem(app->GetSettings()->Volume, 0);   // Set initial value last
    }
}

void CSceneSettings::OnMove(const smart_ptr<ISurface>& lp) {
	key.Input();
	m_mouse.Flush(); // or buttons will stuck

	if(m_timerMain.Get() < 500) return; // Wait until backdrop is blitted
    if (key.IsKeyPush(5)) {  GetSceneControl()->ReturnScene(); } // Handle Space debug key

	// Update buttons
	int index = 0;
	for (smart_vector_ptr<CGUIButton>::iterator it = m_vButtons.begin(); it != m_vButtons.end(); ++it, ++index) {
		CGUIButton* button = it->get();
		if (button) {
			button->OnSimpleMove(lp.get());
			CGUIButtonEventListener* e	= button->GetEvent().get();
			CGUINormalButtonListener* p	= (CGUINormalButtonListener*)e;

			bool btnPressFlag = button->IsLClick(); //TODO: button->IsPushed() + some Counter , to match game
			if (m_nButton == 0 && btnPressFlag)
			{
				m_nButton = button->GetID(); // Selected button id
			}

			if (button->IsIn())
			{
				p->SetImageOffset(1);
			}
		}
	}

	int vol = app->GetSettings()->Volume;
	int volInc = 8;

	// Update slider
    if(m_volumeSlider) {
		m_volumeSlider->OnSimpleMove(lp.get());
		
		// Apply volume mouse click
		if (m_volumeSlider->GetButton() != 0 && m_mouse.IsPushLButton())
		{
			CGUISliderEventListener* s = m_volumeSlider->GetEvent().get();
			if(s->GetLastEvent() == CGUISliderEventListener::SliderEvent::PageLeft)
				vol -= volInc;
			if(s->GetLastEvent() == CGUISliderEventListener::SliderEvent::PageRight)
				vol += volInc;
		}

		if(m_volumeSlider->IsDraged()) {
			int x, y;
			m_volumeSlider->GetSelectedItem(x, y);
			vol = x;
			OutputDebugStringA("Volume during drag: ");
			char debug[32];
			sprintf_s(debug, sizeof(debug), "%d\n", x);
			OutputDebugStringA(debug);
		}
	}

	switch(m_nButton) {
		case 4: //WINDOW
			app->GetSettings()->WindowMode = true;
			break;
		case 5: //FULLSCREEN
			app->GetSettings()->WindowMode = false;
			break;
		case 6: //BITCOUNT16
			app->GetSettings()->BitCount = 0;
			break;
		case 7: //BITCOUNT24
			app->GetSettings()->BitCount = 1;
			break;
		case 8: //BITCOUNT32
			app->GetSettings()->BitCount = 2;
			break;
		case 9: //BACK
			GetSceneControl()->ReturnScene();
			break;
		case 10: //VOL_MINUS
		case 11: //VOL_PLUS
			if(m_nButton == 10) vol -= volInc;
			if(m_nButton == 11) vol += volInc;
			break;
		default:
			break;
	}
	m_nButton = 0;

	// Apply volume value
	if(vol >= 100) vol = 100;
	if(vol <= 0) vol = 0;

	if(vol >= 0 && vol <= 100){
		app->GetSettings()->Volume = vol;
		m_volumeSlider->SetSelectedItem(app->GetSettings()->Volume, 0);
	}
}

void CSceneSettings::OnDraw(const smart_ptr<ISurface>& lp) {
	//lp->Clear();
	//lp->BltFast(m_background.get(), 0, 0); // render cached framebuffer (this can restore FB when something overrides it)

	if(m_timerMain.Get() > 0) {
		if (m_settingsBackdrop.get())
			lp->BlendBltFast(m_settingsBackdrop.get(), m_settingsBackdrop->GetPosX(), m_settingsBackdrop->GetPosY(), m_nFade);
	}

	if(m_timerMain.Get() > 500) {

		// Draw slider
		if(m_volumeSlider && m_timerMain.Get() > 500) {
			m_volumeSlider->OnSimpleDraw(lp.get());
		}
		
		// Draw settings overlays (TODO: in real game its INVERTED...)
		if(app->GetSettings()->WindowMode == true){
			if (m_settingsWindowBtn.get())
				lp->BltFast(m_settingsWindowBtn.get(), m_settingsWindowBtn->GetPosX(), m_settingsWindowBtn->GetPosY());
		} else {
			if (m_settingsFullscreenBtn.get())
				lp->BltFast(m_settingsFullscreenBtn.get(), m_settingsFullscreenBtn->GetPosX(), m_settingsFullscreenBtn->GetPosY());

			// Prepare horizontal blit rectangle split
			int buttonCount = 3;
			int btnId = app->GetSettings()->BitCount; // Retrieve the button ID
			const CSurfaceInfo* bitInfo = m_settingsBitBtn.get() ? m_settingsBitBtn->GetConstSurfaceInfo() : NULL;
			if (bitInfo) {
				SIZE surfSize = bitInfo->GetSize();
				int sliceWidth = surfSize.cx / buttonCount;
				int sliceWidthOffset = sliceWidth * btnId;
				RECT sourceRect = {
					btnId * sliceWidth,
					0,
					(btnId + 1) * sliceWidth,
					surfSize.cy
				};
				lp->BltFast(m_settingsBitBtn.get(), m_settingsBitBtn->GetPosX()+sliceWidthOffset, m_settingsBitBtn->GetPosY(), NULL, &sourceRect, NULL, 0);
			}
		}

		int index = 0;
		for (smart_vector_ptr<CGUIButton>::iterator it = m_vButtons.begin(); it != m_vButtons.end(); ++it, ++index) {
			CGUIButton* button = it->get();
			if (button) {
				// Button event cast
				CGUIButtonEventListener* e	= button->GetEvent().get();
				CGUINormalButtonListener* p	= (CGUINormalButtonListener*)e;
				ISurface* originalSurface = button->GetPlane();
				if (!originalSurface) continue;

				// Special edge case for sliced volume buttons
				if(index == 6 || index == 7){
					POINT sizeInfo = m_vPlaneLoader.GetXY(12);
					RECT hoverButtonLeft = { 0, 0, sizeInfo.x, sizeInfo.x };
					RECT hoverButtonRight = { sizeInfo.x, 0, sizeInfo.x * 2, sizeInfo.x };
					RECT clickedButtonLeft = { 0, sizeInfo.x, sizeInfo.x, sizeInfo.x * 2 };
					RECT clickedButtonRight = { sizeInfo.x, sizeInfo.x, sizeInfo.x * 2, sizeInfo.x * 2 };
					if (button->IsIn())
					{
						if(index == 6){
							if (button->IsPushed())  lp->BltFast(originalSurface, originalSurface->GetPosX(), originalSurface->GetPosY(), NULL, &clickedButtonLeft, NULL, 0);
							else lp->BltFast(originalSurface, originalSurface->GetPosX(), originalSurface->GetPosY(), NULL, &hoverButtonLeft, NULL, 0);
						}
						if(index == 7){
							if (button->IsPushed())  lp->BltFast(originalSurface, originalSurface->GetPosX(), originalSurface->GetPosY(), NULL, &clickedButtonRight, NULL, 0);
							else lp->BltFast(originalSurface, originalSurface->GetPosX(), originalSurface->GetPosY(), NULL, &hoverButtonRight, NULL, 0);
						}
					}
				}
				else
				{
					if (button->IsIn())
					{
						lp->BltNatural(originalSurface, originalSurface->GetPosX(), originalSurface->GetPosY());
						break;
					}
				}
			}
		}
	}
	m_nFade.Inc();
}