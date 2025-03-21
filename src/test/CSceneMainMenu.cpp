// CSceneMainMenu.cpp
// Created by derplayer
// Created on 2025-03-21 19:02:59

#include "CSceneMainMenu.h"

void CSceneMainMenu::OnInit() {
	// Initialize input
	m_mouse.Flush();
	m_mouse.SetGuardTime(1);

	// Fade timer
	m_nFade = CSaturationCounter(0, 255, 8);

	// Load splash screen define
	m_vPlaneLoader.SetLang(app->GetLang());
	m_vPlaneLoader.SetReadDir("data/y/title/");  // Base directory
	if (m_vPlaneLoader.Set("data/y/title/title.txt", false) != 0) {  // Relative to SetReadDir
		OutputDebugStringA("Error: Failed to load data/y/title/title.txt\n");
	}

	// Title screen backdrop
	m_title1 = m_vPlaneLoader.GetPlane(5);
	m_title2 = m_vPlaneLoader.GetPlane(6);

	// Button set A
	m_menu1 = m_vPlaneLoader.GetPlane(7); // POS
	m_menu2 = m_vPlaneLoader.GetPlane(8); // POS
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

	// Setup buttons
	static const int BUTTON_X = 227+5; //TODO: load from txt
    static const int BUTTON_Y = 331+27;
    static const int BUTTON_SPACING = 37; //BUTTON_Y / 5; // Height of each button slice (button sheet size / entry amount)
    
    for(int i = 0; i < 6; i++) {
        m_vButtons[i].SetMouse(smart_ptr<CFixMouse>(&m_mouse, false));

        // Create the button listener as CGUIButtonEventListener type directly
        smart_ptr<CGUIButtonEventListener> buttonListener(new CGUINormalButtonListener());
        
        // Cast to derived type to access CGUINormalButtonListener methods
        CGUINormalButtonListener* p = static_cast<CGUINormalButtonListener*>(buttonListener.get());
        p->SetPlaneLoader(smart_ptr<CPlaneLoader>(&m_vPlaneLoader, false), 7 + i);
        //p->SetType(1);
		//p->SetImageOffset(35);

        m_vButtons[i].SetEvent(buttonListener);
        m_vButtons[i].SetXY(BUTTON_X, BUTTON_Y);
		RECT boundsRect = { 0, i * BUTTON_SPACING, BUTTON_Y, (i + 1) * BUTTON_SPACING };
		m_vButtons[i].SetBounds(boundsRect);
    }

    m_nButton = 0;
    //m_nFade.Set(0, 16, 1);  // 16 frames fade
}

void CSceneMainMenu::OnMove(const smart_ptr<ISurface>& lp) {
	key.Input();
	m_mouse.Flush(); // or buttons will stuck

	// Update buttons
    for(int i = 0; i < 6; i++) {
		m_vButtons[i].OnSimpleMove(lp.get());
		CGUIButtonEventListener* e	= m_vButtons[i].GetEvent().get();
		CGUINormalButtonListener* p	= (CGUINormalButtonListener*)e;

        // Check for button clicks
        if (m_nButton == 0 && m_vButtons[i].IsLClick()) {
            m_nButton = i + 1; // Selected button (0=none, 1=yes, 2=no) .......................

			//p->SetType(32);
			//p->SetImageOffset(2);
			
        }

		if (m_vButtons[i].IsIn()) {
			//p->SetImageOffset(1);
			p->SetPlaneNumber(8);
		} else{
			//p->SetImageOffset(2);
			p->SetPlaneNumber(7);
		}
    }
}

void CSceneMainMenu::OnDraw(const smart_ptr<ISurface>& lp) {

	lp->BltFast(m_title1, 0, 0);
	lp->BlendBltFast(m_title2, 0, 0, m_nFade);
	
	m_nFade.Inc();

	int x, y, b;
    m_mouse.GetInfo(x, y, b);
	char buf[128];
	sprintf(buf, "MouseLoop menu: %d %d %d\n", x,y,b);
	OutputDebugStringA(buf);

	// base drop
	lp->BltFast(m_menu1, 227, 331);

	// Draw buttons
    const int buttonCount = 6; // Total number of buttons
	const int sliceHeight = 37; // Height of each button slice
	const int BUTTON_X = 227 + 5; // Starting X position on `lp`
	const int BUTTON_Y = 331 + 27; // Starting Y position on `lp`

	// Get the dimensions of the source surface
	int width = 0, height = 0;
	// Loop through and blit each slice of the source surface onto the target `lp`
	for (int i = 0; i < buttonCount; ++i) {
		ISurface* originalSurface = m_vButtons[i].GetPlane();
		originalSurface->GetSize(width, height); // Ensure variables match expected types

		// Define the source rectangle for the current slice
		RECT sourceRect = { 0, i * sliceHeight, width, (i + 1) * sliceHeight };

        // Calculate the destination position on the target surface (lp)
        int destX = BUTTON_X; // X position remains constant
        int destY = BUTTON_Y + (i * sliceHeight); // Increment Y for each button

		// Blit the slice directly onto the primary surface
		if (m_vButtons[i].IsIn())
		{
			//originalSurface = m_vButtons[3].GetPlane();
			lp->BltFast(originalSurface, destX, destY, NULL, &sourceRect, NULL, 0);
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