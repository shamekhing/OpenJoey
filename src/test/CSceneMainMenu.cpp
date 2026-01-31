// CSceneMainMenu.cpp
// Created by derplayer
// Created on 2025-03-21 19:02:59
#include "stdafx.h"
#include "CSceneMainMenu.h"
static bool ButtonAnimFowardDirection = true;
static bool ButtonAnimPause = false;
static bool ButtonClicked = false;

void CSceneMainMenu::OnInit() {
	// Initialize input
	m_mouse.Flush();
	m_mouse.SetGuardTime(1);

	// Fade timer
	m_nFade = CSaturationCounter(0, 255, 8);
	m_nFade2 = CSaturationCounter(0, 255, 8);
	m_nFade3 = CSaturationCounter(0, 255, 8);
	m_nFadeButton = CSaturationCounter(0, 5, 1);
	m_nButtonClickTracker = CSaturationCounter(0, 5, 1);
	m_timer = CTimer();
	m_timer2 = CTimer();
	m_timer.Reset();
	m_timer.Pause();
	m_timer2.Restart();
	m_timer2.Reset(); //always running

	// Load title/menu define from data/y/title/
	m_vPlaneLoader.SetLang(app->GetLang());
	m_vPlaneLoader.SetReadDir("data/y/title/");
	LRESULT setResult = m_vPlaneLoader.Set("data/y/title/title.txt", false);
	if (setResult != 0) {
		OutputDebugStringA("Error: Failed to load data/y/title/title.txt\n");
	}

	// Title screen backdrop (CPlane holds ownership; no dangling pointer from GetPlane() temporary)
	m_title1 = m_vPlaneLoader.GetPlane(5);
	m_title2 = m_vPlaneLoader.GetPlane(6);

	// Button set A
	m_menu1 = m_vPlaneLoader.GetPlane(7);
	m_menu2 = m_vPlaneLoader.GetPlane(8);
	m_menu3 = m_vPlaneLoader.GetPlane(9);
	m_menu4 = m_vPlaneLoader.GetPlane(10);
	m_menu5 = m_vPlaneLoader.GetPlane(11);
	m_menu6 = m_vPlaneLoader.GetPlane(12);
	m_menu7 = m_vPlaneLoader.GetPlane(13);

	// Button set B
	//m_menuA1 = m_vPlaneLoader.GetPlane(14); // POS
	//m_menuA2 = m_vPlaneLoader.GetPlane(15); // POS
	//m_menuA3 = m_vPlaneLoader.GetPlane(16);
	//m_menuA4 = m_vPlaneLoader.GetPlane(17);
	//m_menuA5 = m_vPlaneLoader.GetPlane(18);
	//m_menuA6 = m_vPlaneLoader.GetPlane(19);
	//m_menuA7 = m_vPlaneLoader.GetPlane(20);

	// Puzzle
	/*m_menuPuzzle = m_vPlaneLoader.GetPlane(21);*/

	// Menu/options: center X only; Y from title.txt (640x480) scaled to 800x600
	static const int SCREEN_W = 800, SCREEN_H = 600;
	static const int BUTTON_SPACING = 46;
	const int menuH = 6 * BUTTON_SPACING;
	POINT pos7 = m_vPlaneLoader.GetXY(7);
	int buttonWidth = 0, buttonSheetHeight = 0;
	{
		CPlane firstBtn = m_vPlaneLoader.GetPlane(7);
		if (firstBtn.get()) firstBtn.get()->GetSize(buttonWidth, buttonSheetHeight);
		if (buttonWidth <= 0) buttonWidth = 358;
	}
	int BUTTON_X = (SCREEN_W - buttonWidth) / 2;   // center width-wise
	int BUTTON_Y = (pos7.y * SCREEN_H) / 480;
	if (BUTTON_Y + menuH > SCREEN_H) BUTTON_Y = SCREEN_H - menuH;
	if (BUTTON_Y < 0) BUTTON_Y = 0;

	for (int i = 0; i < 6; i++) {
		m_vButtons[i].SetMouse(smart_ptr<CFixMouse>(&m_mouse, false)); // Non-owning: m_mouse outlives scene

		// Create the button listener as CGUIButtonEventListener type directly
		smart_ptr<CGUIButtonEventListener> buttonListener(new CGUINormalButtonListener());

		// Cast to derived type to access CGUINormalButtonListener methods
		CGUINormalButtonListener* p = static_cast<CGUINormalButtonListener*>(buttonListener.get());
		p->SetPlaneLoader(smart_ptr<CPlaneLoader>(&m_vPlaneLoader, false), 7 + i);

		m_vButtons[i].SetEvent(buttonListener);
		// Per-button position from title.txt (scaled to 800x600)
		m_vButtons[i].SetXY(BUTTON_X, BUTTON_Y + i * BUTTON_SPACING);
		// Bounds: full slice for this button (local 0,0 to width x slice height)
		RECT boundsRect = { 0, 0, buttonWidth, BUTTON_SPACING };
		m_vButtons[i].SetBounds(boundsRect);
	}

    m_nButton = 0;
    //m_nFade.Set(0, 16, 1);  // 16 frames fade
}

void CSceneMainMenu::OnMove(const smart_ptr<ISurface>& lp) {
	key.Input();
	m_mouse.Flush(); // or buttons will stuck

	// Update buttons (no Space->SCENE1; app starts at splash)
    for(int i = 0; i < 6; i++) {
		m_vButtons[i].OnSimpleMove(lp.get());
		CGUIButtonEventListener* e	= m_vButtons[i].GetEvent().get();
		CGUINormalButtonListener* p	= (CGUINormalButtonListener*)e;

        // Check for button clicks
        if (m_nButton == 0 && m_vButtons[i].IsLClick()) {
            m_nButton = i + 1; // Selected button (0=none, 1=yes, 2=no) .......................
            { char buf[64]; sprintf_s(buf, "[OpenJoey] MainMenu: button %d clicked (index %d)\n", m_nButton, i); OutputDebugStringA(buf); }

			//p->SetType(32);
			//p->SetImageOffset(2);
			
        }

		// set highlight gfx sheet
		if (m_vButtons[i].IsIn()) {
			//p->SetImageOffset(1);
			//p->SetPlaneNumber(13); //8-13 (7 is base backdrop without highlight)
		} else{
			//p->SetImageOffset(2);
			//p->SetPlaneNumber(5);
			//m_timer.Reset();
			//m_timer.Pause();
		}
    }

	// Button order depends on title art: index 0 = first row, 1 = second, etc.
	// Case 2 opens Deck Editor so the second menu item (Deck construction) goes to deck editor.
	switch(m_nButton) {
		case 1:
			OutputDebugStringA("[OpenJoey] MainMenu: case 1 -> OnPreClose (exit)\n");
			app->OnPreClose(); // first menu item: quit/placeholder
			ButtonClicked = true;
			break;
		case 2:
			OutputDebugStringA("[OpenJoey] MainMenu: case 2 -> CallSceneFast(DECKEDITOR)\n");
			GetSceneControl()->CallSceneFast(SCENE_DECKEDITOR);
			ButtonClicked = true;
			break;
		case 3:
			GetSceneControl()->CallSceneFast(SCENE_CARDLIST);
			ButtonClicked = true;
			break;
		case 4:
			//app->OnPreClose(); // TODO: placeholder
			//GetSceneControl()->CallSceneFast(SCENE_ISEND);
			GetSceneControl()->CallSceneFast(SCENE_SETTINGS);
			ButtonClicked = true;
			break;
		case 5:
			app->OnPreClose();
			ButtonClicked = true;
			break;
		case 0:
		default:
			break;
	}
	m_nButton = 0; // reset button state
}

void CSceneMainMenu::OnDraw(const smart_ptr<ISurface>& lp) {
	lp->Clear();

	// Center X only; Y from title.txt (640x480) scaled to 800x600. Blt/BlendBlt = color key (no green).
	static const int SCREEN_W = 800, SCREEN_H = 600;
	POINT pos5 = m_vPlaneLoader.GetXY(5), pos6 = m_vPlaneLoader.GetXY(6), pos7 = m_vPlaneLoader.GetXY(7);
	int w = 0, h = 0;

	if (m_timer2.Get() > 0 && m_title1.get()) {
		m_title1->GetSize(w, h);
		int t1x = (SCREEN_W - w) / 2;  if (t1x < 0) t1x = 0;
		int t1y = (pos5.y * SCREEN_H) / 480;  if (t1y < 0) t1y = 0;
		lp->Blt(m_title1.get(), t1x, t1y);
	}
	if (m_timer2.Get() > 300 && m_title2.get()) {
		m_title2->GetSize(w, h);
		int t2x = (SCREEN_W - w) / 2;  if (t2x < 0) t2x = 0;
		int t2y = (pos6.y * SCREEN_H) / 480;  if (t2y < 0) t2y = 0;
		lp->BlendBlt(m_title2.get(), t2x, t2y, (int)m_nFade);
		m_nFade.Inc();
	}
	if(m_timer2.Get() > 1000) {
		const int BUTTON_STACK_H = 6 * 46;
		int m1y = (pos7.y * SCREEN_H) / 480;
		if (m1y + BUTTON_STACK_H > SCREEN_H) m1y = SCREEN_H - BUTTON_STACK_H;
		if (m1y < 0) m1y = 0;
		if (m_menu1.get()) {
			m_menu1->GetSize(w, h);
			int m1x = (SCREEN_W - w) / 2;  if (m1x < 0) m1x = 0;
			lp->BlendBlt(m_menu1.get(), m1x, m1y, (int)m_nFade2);
		}
		m_nFade2.Inc();
	}

	// btn click tracker update
	//if(ButtonClicked){ 		
	//	if (m_timer2.Get() % 500 == 0)
	//	{
	//		//m_nButtonClickTracker.Inc();
	//		CGUIButtonEventListener* e	= m_vButtons[m_nButton].GetEvent().get();
	//		CGUINormalButtonListener* p	= (CGUINormalButtonListener*)e;
	//		if(m_nButtonClickTracker.Get() == 0) p->SetPlaneNumber(8);
	//	}
	//}
	if(m_timer2.Get() > 2000) {
		// Draw buttons: center X; Y from title.txt scaled, clamped
		const int buttonCount = 6;
		const int sliceHeight = 46;
		int width = 0, height = 0;
		ISurface* pFirst = m_vButtons[0].GetPlane();
		if (pFirst) pFirst->GetSize(width, height);
		int DRAW_BUTTON_X = (800 - width) / 2;  if (DRAW_BUTTON_X < 0) DRAW_BUTTON_X = 0;
		int DRAW_BUTTON_Y = (pos7.y * 600) / 480;
		if (DRAW_BUTTON_Y + buttonCount * sliceHeight > 600) DRAW_BUTTON_Y = 600 - buttonCount * sliceHeight;
		if (DRAW_BUTTON_Y < 0) DRAW_BUTTON_Y = 0;
		for (int i = 0; i < buttonCount; ++i) {
			CGUIButtonEventListener* e	= m_vButtons[i].GetEvent().get();
			CGUINormalButtonListener* p	= (CGUINormalButtonListener*)e;

			p->SetPlaneNumber(8+m_nFadeButton.Get()); // update fade button gfx

			ISurface* originalSurface = m_vButtons[i].GetPlane();
			if (!originalSurface) continue;
			originalSurface->GetSize(width, height);

			// Source rect: slice i of the button sheet
			RECT sourceRect = { 0, i * sliceHeight, width, (i + 1) * sliceHeight };
			int destX = DRAW_BUTTON_X;
			int destY = DRAW_BUTTON_Y + (i * sliceHeight);

			if (m_vButtons[i].IsIn())
			{
				lp->BltFast(originalSurface, destX, destY, NULL, &sourceRect, NULL, 0);

				m_timer.Restart();
				if(m_timer.Get() > 100)
				{
					if(ButtonAnimFowardDirection) m_nFadeButton.Inc(); else m_nFadeButton.Dec();

					if(m_nFadeButton.IsEnd()) { ButtonAnimFowardDirection = false; }
					if(m_nFadeButton.IsBegin()) { ButtonAnimFowardDirection = true; }

					m_timer.Reset();
				}
			}
		}
	}
    // Handle fade out effect when button is selected
   // if (m_nButton != 0) {
   //     m_nFade++;
   //     if (m_nFade.IsEnd()) {
   //         if (m_nButton == 1) {
			//	GetSceneControl()->ReturnScene();
			//} else {
			//	app->OnPreClose();
			//}
   //         return;
   //     }
   // }
}