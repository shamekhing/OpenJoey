// CSceneYesNo.cpp
// Created by derplayer
// Created on 2025-01-26 23:01:28

#include "CSceneYesNo.h"

void CSceneYesNo::OnInit() {
    // Initialize input
    m_mouse.Flush();
    m_mouse.SetGuardTime(1);
	IsSetLeva = false;

    // Load resources
    m_vPlaneLoader.SetReadDir("test/yesno/");  // Base directory
    if (m_vPlaneLoader.Set("test/yesno/list.txt", false) != 0) {  // Relative to SetReadDir
        OutputDebugStringA("Error: Failed to load test/yesno/list.txt\n");
    }

    //m_pMessageSurface = m_vPlaneLoader.GetPlane(0);  // Plane 0 = first line
	//std::string testo = m_vPlaneLoader.GetFileName(0);
	//CSurfaceInfo* testS = m_pMessageSurface->GetSurfaceInfo();
    if (m_pMessageSurface.get() == NULL) {
        OutputDebugStringA("Error: m_pMessageSurface is NULL\n");
    }

    // Check factory (for background)
    CFastPlaneFactory* factoryX = app->GetDrawFactory();
    if (factoryX) {
		smart_ptr<ISurface> screenPtr = factoryX->GetDraw()->GetSecondary()->cloneFull();

		//CFastPlane* asdf = factoryX->GetDraw()->GetSecondary();
		//m_vBackground1 = app->GetDraw()->GetSecondary();
		//m_vBackground1->Release();
		//m_vBackground1->SubColorFast(172);  // Darken

		//m_lastFrameFB = factoryX->GetDraw()->GetSecondary();
		//CFastPlane* TP = factoryX->GetDraw()->GetSecondary();
		//smart_ptr<ISurface> TPS = smart_ptr<ISurface>(new CFastPlane(TP->GetFastDraw()), false);

		//TPS->Release();
		//TP->Restore();
		//smart_ptr<ISurface> srf = TP->clone();
		//ISurface* TP1 = srf.get();
		//TP1->Release();

		//smart_ptr<ISurface> secondary = smart_ptr<ISurface>(factoryX->GetDraw()->GetSecondary(), false);
        if (screenPtr.get()) {
			screenPtr->SubColorFast(255255255);  // Darken
			m_vBackground = screenPtr;

			//screenPtr->Release();
			//m_vFastBackground = CFastPlane(secondary.get());
            OutputDebugStringA("Background loaded with app factory\n");
        }
    } else {
        OutputDebugStringA("Warning: No draw factory available\n");
    }

    // Use default factory for m_vPlaneLoader (CPlane::GetFactory())
    //m_vPlaneLoader.SetReadDir("");
    //if (m_vPlaneLoader.Set("test/yesno/list.txt", false) != 0) {
    //    OutputDebugStringA("Error: Failed to load data/yesno/list.txt\n");
    //    return;
    //}

    FILE* testFile = fopen("test/yesno/01.yga", "rb");
    if (testFile) {
        fclose(testFile);
        OutputDebugStringA("File test/yesno/01.yga exists\n");
    } else {
        OutputDebugStringA("Error: Cannot open data/yesno/01.yga\n");
    }

	/*
    LRESULT lr = m_vPlaneLoader.Load(0);
    if (lr != 0) {
        char buf[64];
        sprintf(buf, "Error: Load(0) failed, code=%ld\n", lr);
        OutputDebugStringA(buf);
    } else {
        OutputDebugStringA("Load(0) succeeded\n");
    }
	*/
    //CPlane bgPlane = m_vPlaneLoader.GetPlane(0);

	/*
    if (bgPlane.get() == NULL) {
        OutputDebugStringA("Error: bgPlane.get() is NULL\n");
    } else {
        int type = bgPlane->GetType();
        char typeBuf[32];
        sprintf(typeBuf, "bgPlane type=%d\n", type);
        OutputDebugStringA(typeBuf);

        CSurfaceInfo* test = bgPlane->GetSurfaceInfo();
        if (test) {
            int sx, sy;
            bgPlane->GetSize(sx, sy);
            char buf[128];
            sprintf(buf, "bgPlane: IsInit=%d, GetPtr=%p, type=%d, size=%dx%d\n",
                    test->IsInit(), test->GetPtr(), test->GetSurfaceType(), sx, sy);
            OutputDebugStringA(buf);
        }
    }
	*/

    // Load background using PlaneEffect
    CFastPlaneFactory* factory = app->GetDrawFactory();
    if (factory && factory->GetDraw()) {
        // Create a new plane from the current screen
        //m_vBackground = CPlane(factory->GetDraw()->GetSecondary());
		//m_vFastBackground1 = new CFastPlane(app->GetDraw()->GetSecondary()->GetFastDraw());

        // If you want to darken it, do it in OnDraw instead of here
        // This way we avoid surface locking issues during initialization
    }

    //m_pMessageSurface = m_vPlaneLoader.GetPlane(0);  // Load message

    // Setup buttons
    static const int BUTTON_Y = 240;
    static const int BUTTON_SPACING = 120;
    
    for(int i = 0; i < 2; i++) {
        m_vButtons[i].SetMouse(smart_ptr<CFixMouse>(&m_mouse, false));

        // Create the button listener as CGUIButtonEventListener type directly
        smart_ptr<CGUIButtonEventListener> buttonListener(new CGUINormalButtonListener());
        
        // Cast to derived type to access CGUINormalButtonListener methods
        CGUINormalButtonListener* p = static_cast<CGUINormalButtonListener*>(buttonListener.get());
        p->SetPlaneLoader(smart_ptr<CPlaneLoader>(&m_vPlaneLoader, false), 1 + i*2);
        p->SetType(1);

        m_vButtons[i].SetEvent(buttonListener);
        m_vButtons[i].SetXY(216 + i*BUTTON_SPACING, BUTTON_Y);
    }

    m_nButton = 0;
    m_nFade.Set(0, 16, 1);  // 16 frames fade
}

void CSceneYesNo::OnMove(const smart_ptr<ISurface>& lp) {
    key.Input();
	m_mouse.Flush(); // or buttons will stuck

    // Handle ESC key
    if (key.IsKeyPush(VK_ESCAPE)) {
        m_nButton = 2;  // No
        return;
    }

    // Update buttons
    for(int i = 0; i < 2; i++) {
		m_vButtons[i].OnDraw(lp.get());
        
        // Check for button clicks
        if (m_nButton == 0 && m_vButtons[i].IsLClick()) {
            m_nButton = i + 1; // Selected button (0=none, 1=yes, 2=no)
			CGUIButtonEventListener* e	= m_vButtons[i].GetEvent().get();
			CGUINormalButtonListener* p	= (CGUINormalButtonListener*)e;
			//p->SetType(32);
			//p->SetImageOffset(2);
        }
    }
}

void CSceneYesNo::OnDraw(const smart_ptr<ISurface>& lp) {

	lp->Clear();
	//lp->BltFast(m_vBackground, 0, 0);

	if(IsSetLeva == false) {
		//m_vFastBackground1 = app->GetDraw()->GetSecondary();
		//m_vBackground = app->GetDraw()->GetSecondary()->clone();
		//IsSetLeva = true;
		//return;
	}
    // Draw darkened background
	CSurfaceInfo* m_vBackgroundInfo = m_vBackground->GetSurfaceInfo();
	lp->BltFast(m_vBackground.get(), 0, 0);
	//CSurfaceInfo* m_vBackgroundInfo = app->framebufferCache->GetSurfaceInfo();
	//lp->BltFast(app->framebufferCache.get(), 0, 0);

	//factoryZ->GetDraw()->GetSecondary()->Blt(m_vBackground, 0, 0);
	//CPlane bgPlane;
	//bgPlane->Load("test/yesno/01.yga");
	//bgPlane = m_vPlaneLoader.GetPlane(0);
	//CSurfaceInfo* test = bgPlane->GetSurfaceInfo();
	//lp->BltFast(m_pMessageSurface,0,0);
	//return;

    // Draw message centered
    //int sx, sy;
    //m_pMessageSurface->GetSize(sx, sy);
    //lp->BltNatural(m_pMessageSurface.get(), 320 - sx/2, 200 - sy/2);

	int x, y, b;
    m_mouse.GetInfo(x, y, b);
	char buf[128];
	sprintf(buf, "MouseLoop: %d %d %d\n", x,y,b);
	OutputDebugStringA(buf);

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
			} else {
				//app->Exit
				app->OnPreClose();
			}
            return;
        }

        // Apply fade effect
        //BYTE fadeAlpha = (BYTE)(255 - ((int)m_nFade * 16));
        //lp->BlendBltFast(m_vBackground.get(), 0, 0, fadeAlpha);
    }


	// Apply text to scene surface
	CTextFastPlane* pTextPtr = new CTextFastPlane;
    pTextPtr->GetFont()->SetText("This is a fastcall test scene.\nPress YES to return to the scene that called this or NO to exit the app.");
    pTextPtr->GetFont()->SetSize(20);
    pTextPtr->UpdateTextAA();
    CPlane pText = CPlane(pTextPtr);
	lp->BltNatural(pText,20,100);
}