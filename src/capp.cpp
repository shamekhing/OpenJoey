#include "stdafx.h"
#include "capp.h"

void CApp::MainThread() {
    GetDraw()->SetDisplay(false, 800, 600, 16);

    CKey1 key;
    CFPSTimer timer;
    timer.SetFPS(30);
	m_bWindowClosing = false;

    // Make this the main application (close all other windows when exiting)
    SetMainApp(true);

	// Initialize BinSystem: look for data next to the executable
	char exeDir[MAX_PATH];
	if (GetModuleFileNameA(NULL, exeDir, MAX_PATH) == 0) {
		MessageBox(NULL, "Failed to get executable path.", "Error", MB_OK | MB_ICONERROR);
		return;
	}
	char* lastSlash = strrchr(exeDir, '\\');
	if (lastSlash) *lastSlash = '\0';
	char binBasePath[MAX_PATH];
	sprintf_s(binBasePath, "%s\\data\\bin#", exeDir);

	if(!binSystem_.Initialize(binBasePath, GetLangFull().c_str()))
    {
        char msg[1024];
        sprintf_s(msg,
            "Failed to initialize BinSystem.\n\n"
            "Card data is required but omitted from the repo to reduce legal risk.\n"
            "Copy the data folder from a legally acquired Power of Chaos installation\n"
            "(e.g. yu-gi-oh_mod_data_stock_template) into the executable directory.\n\n"
            "Place card data in:\n%s\n\n"
            "Required: card_prop.bin, card_name%s.bin, card_id.bin, card_intid.bin, "
            "card_desc%s.bin, card_indx%s.bin, dlg_text%s.bin, dlg_indx%s.bin, card_pack.bin, "
            "and ..\\card\\list_card.txt, ..\\mini\\list_card.txt",
            binBasePath, GetLangFull().c_str(), GetLangFull().c_str(), GetLangFull().c_str(), GetLangFull().c_str(), GetLangFull().c_str());
        MessageBox(NULL, msg, "Error", MB_OK | MB_ICONERROR);
        return;
    }

	// TEST BIN SYSTEM - Can we get data for a card
    const Card* cardUnitTest = binSystem_.GetCard(582); //582 (normal monster), 428 (effect monster), 235 (trap), 852 (Spell card), 1064 (fusion)
    if(cardUnitTest)
    {
        // Access card's data:
        const char* name = cardUnitTest->name.name;
		BOOL gfxValid = cardUnitTest->hasValidGFX;
        WORD attack = cardUnitTest->properties.GetAttackValue();
        WORD defense = cardUnitTest->properties.GetDefenseValue();
		MonsterType type = cardUnitTest->properties.GetMonsterType();
		CardCategory cat = cardUnitTest->properties.GetCardCategory();
		MonsterAttribute attr = cardUnitTest->properties.GetMonsterAttribute();
		BYTE stars = cardUnitTest->properties.GetMonsterStars();
        const char* desc = cardUnitTest->description;
		const char* gfxPath = cardUnitTest->imageFilename;
		const char* gfxMiniPath = cardUnitTest->imageMiniFilename;
    }

    // Or if you need all cards
    const Card* allCards = binSystem_.GetCards();
    DWORD cardCount = binSystem_.GetCardCount();
    
    for(DWORD i = 0; i < cardCount; i++)
    {
        const Card* currentCard = &allCards[i];
        // Work with currentCard...
    }

	// PLANE TEST
    CPlane bgplane;
    bgplane->Load("data/y/try/end_e.bmp");

    CPlane charaplane;
    charaplane->Load("data/y/tutorial/fuki/tutorial_win0.yga");

    CRootCounter nFade(0, 255, 8);
    CRootCounter nPhase(0, 6, 1);

	/*
    CTextFastPlane* pText = new CTextFastPlane;
    pText->GetFont()->SetText("Press Space to move to next scene!");
    pText->GetFont()->SetSize(30);
    pText->UpdateTextAA();
    CPlane text(pText);
	*/
	//float test = sqrt(32.0);

	// Create scene controller with factory
    smart_ptr<ISceneFactory> factory(new CJoeySceneFactory(this));
    m_sceneControl = smart_ptr<ISceneControl>(new CSceneControl(factory));
	m_sceneControl->JumpScene(SCENE1);

	// fake settings (parse later registry)
	settings_.BitCount = 2;
	settings_.Volume = 50;
	settings_.WindowMode = true;

	int nPat = 0;
	int testInt = 0;
    while (IsThreadValid()){
        ISurface* pSecondary = GetDraw()->GetSecondary();
		smart_ptr<ISurface> surface(pSecondary, false); // false means don't take ownership (don't delete)

		//framebufferCache = surface->cloneFull();
		//testInt += 16;
		//if(testInt >= 256) testInt = 0;

        // If you're always transferring the background to the entire screen, you don't need to clear it
		//pSecondary->Clear();

        // Different drawing for each phase
        switch (nPhase){
        case 0: {

			// Check for exit request
			if (m_bWindowClosing) {
				if (m_sceneControl->GetSceneNo() != SCENE_ISEND) {
					m_sceneControl->CallSceneFast(SCENE_ISEND);
				}
				m_bWindowClosing = false;
			}

			// Update and draw current scene
			m_sceneControl->OnMove(surface);
			m_sceneControl->OnDraw(surface);
            break;
                }
        case 99: {
            //pSecondary->BltFast(bgplane,0,0);
            // Use BltFast for transfers with transparency disabled
            pSecondary->BltNatural(charaplane,0,0);
            // For transfers with transparency enabled and YGA images (images with alpha information),
            // it's clearer to use BltNatural
			LRESULT res = ISurfaceTransBlt::BlindBlt1(
                pSecondary,    // destination surface
                bgplane.get(),    // source surface
                0,                   // x position
                0,                   // y position
                testInt,   // transition phase (0-256)
                0,                   // transition mode
                255,                 // fade rate
                NULL                 // clip rectangle
            );
            break;
                }
        case 1: {
            //pSecondary->BltNatural(charaplane,0,0);
            // Display the character first,
            pSecondary->BlendBltFast(bgplane,0,0,255-nFade);
            // then draw the background with fade effect!
			//pSecondary->GeneralBlt(CSurfaceInfo::EBltType::)
            nFade++;
            break;
                }
        case 2: {
            pSecondary->BltFast(bgplane,0,0);
            // Use BltFast for transfers with transparency disabled
            int sx,sy;
            charaplane->GetSize(sx,sy);    // Get the surface size
            // Transfer using the screen center as the base point
            SIZE dstsize = { sx*nFade>>8 , sy * nFade>>8 };
            pSecondary->BltNatural(charaplane,sx/2,sy/2,nFade/2+128,&dstsize,NULL,NULL,4);
            // In this case, the last parameter 4 specifies that coordinates
            // are relative to the center of the image
            nFade++;
            break;
                }
        case 3: {
            pSecondary->BltFast(bgplane,0,0);
            int sx,sy;
            charaplane->GetSize(sx,sy);    // Get the surface size
            SIZE dstsize = { sx*nFade>>8 , sy * nFade>>8 };
            pSecondary->AddColorBltFast(charaplane,sx/2,sy/2,&dstsize,NULL,NULL,4);
            // AddColor (additive blending) from alpha surface
            nFade++;
            break;
                }
        case 4: {
            pSecondary->BltFast(bgplane,0,0);
            int sx,sy;
            charaplane->GetSize(sx,sy);    // Get the surface size
            SIZE dstsize = { sx*nFade>>8 , sy * nFade>>8 };
            pSecondary->SubColorBltFast(charaplane,sx/2,sy/2,&dstsize,NULL,NULL,4);
            // SubColor (subtractive blending) from alpha surface
            nFade++;
            break;
                }
        case 5: {
            pSecondary->BltFast(bgplane,0,0);
            int sx,sy;
            charaplane->GetSize(sx,sy);    // Get the surface size
            SIZE dstsize = { sx*nFade>>8 , sy * nFade>>8 };
            pSecondary->AddColorBltFastFade(charaplane,sx/2,sy/2,nFade,&dstsize,NULL,NULL,4);
            // AddColor (additive blending) + Fade (decay specification) from alpha surface
            nFade++;
            break;
                }
        case 6: {
            pSecondary->BltFast(bgplane,0,0);
            int sx,sy;
            charaplane->GetSize(sx,sy);    // Get the surface size
            SIZE dstsize = { sx*nFade>>8 , sy * nFade>>8 };
            pSecondary->SubColorBltFastFade(charaplane,sx/2,sy/2,nFade,&dstsize,NULL,NULL,4);
            // SubColor (subtractive blending) + Fade (decay specification) from alpha surface
            nFade++;
            break;
                }
        }

        //pSecondary->BltNatural(text,20,400);

        GetDraw()->OnDraw();

        key.Input();
        if (key.IsKeyPush(0))    // Exit with ESC key
            break;
        if (key.IsKeyPush(5)) {  // Press SPACE key to increment phase
            //nPhase++;
            //nFade.Reset();
        }

        timer.WaitFrame();
    }
}

LRESULT CApp::OnPreClose() {
    m_bWindowClosing = true;
	this->GetMyApp()->Close();
    // Return 1 to prevent immediate close
    return 1;
}

// This is the class for the main window
class CAppMainWindow : public CAppBase {    // Derived from application class
	virtual void MainThread(){              // This is the worker thread
		IWindow* pWin = CAppManager::GetMyWindow();
		CWindowOption* opt = pWin->GetWindowOption();
		CApp().Start();
	};
	virtual LRESULT OnPreCreate(CWindowOption &opt){
		opt.caption = "OpenJoey v0.02";
		// you can also load whole dialog from .rc (for fancy win32 GUI stuff, see sample8 (v3))
		// opt.dialog = MAKEINTRESOURCE(IDD_DIALOG1);
		return 0;
	};
	virtual LRESULT OnPreClose() {
        return 0;
    }
};

// The well-known WinMain
int APIENTRY WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
{
    {
        //*
        {   // Output error log to file
            CTextOutputStreamFile* p = new CTextOutputStreamFile;
            p->SetFileName("Error.txt");
            Err.SelectDevice(smart_ptr<ITextOutputStream>(p));
        }
        //*/

        CAppInitializer init(hInstance,hPrevInstance,lpCmdLine,nCmdShow);
        // �Must always write this

        CSingleApp sapp;
        if (sapp.IsValid()) {
            CThreadManager::CreateThread(new CAppMainWindow());

            // Create the main window defined above
			// CThreadManager::CreateThread(new CAppMainWindow);
			// Writing multiple lines will create multiple windows

        }
        // Here CAppInitializer goes out of scope, at which point
        // it waits for all threads to finish
    }
    return 0;
}