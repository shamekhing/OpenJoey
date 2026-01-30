#include "stdafx.h"
#include "capp.h"

static bool FileExists(const char* path) {
    DWORD attrs = GetFileAttributesA(path);
    return (attrs != INVALID_FILE_ATTRIBUTES) && ((attrs & FILE_ATTRIBUTE_DIRECTORY) == 0);
}

static bool HasRequiredData(const char* basePath, const char* language) {
    char path[MAX_PATH];

    sprintf_s(path, "%s\\card_prop.bin", basePath);
    if (!FileExists(path)) return false;

    sprintf_s(path, "%s\\card_name%s.bin", basePath, language);
    if (!FileExists(path)) return false;

    sprintf_s(path, "%s\\card_id.bin", basePath);
    if (!FileExists(path)) return false;

    sprintf_s(path, "%s\\card_intid.bin", basePath);
    if (!FileExists(path)) return false;

    sprintf_s(path, "%s\\card_desc%s.bin", basePath, language);
    if (!FileExists(path)) return false;

    sprintf_s(path, "%s\\card_indx%s.bin", basePath, language);
    if (!FileExists(path)) return false;

    sprintf_s(path, "%s\\dlg_text%s.bin", basePath, language);
    if (!FileExists(path)) return false;

    sprintf_s(path, "%s\\dlg_indx%s.bin", basePath, language);
    if (!FileExists(path)) return false;

    sprintf_s(path, "%s\\card_pack.bin", basePath);
    if (!FileExists(path)) return false;

    sprintf_s(path, "%s\\..\\card\\list_card.txt", basePath);
    if (!FileExists(path)) return false;

    sprintf_s(path, "%s\\..\\mini\\list_card.txt", basePath);
    if (!FileExists(path)) return false;

    return true;
}

void CApp::MainThread() {
    // Use 800x600 (original game resolution) so UI and assets fit without overflow
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

	// Prefer data next to exe; if missing or incomplete, use repo data folder
	char binBasePath[MAX_PATH];
	sprintf_s(binBasePath, "%s\\data\\bin#", exeDir);
	char dataRoot[MAX_PATH];
	sprintf_s(dataRoot, "%s\\data", exeDir);

	bool exeHasRequired = HasRequiredData(binBasePath, GetLangFull().c_str());

	// If exe-dir is missing required data, try repo root: exe in src\_Build\Debug -> go up to OpenJoey\data
	if (!exeHasRequired) {
		char fallbackBin[MAX_PATH];
		sprintf_s(fallbackBin, "%s\\..\\..\\..\\data\\bin#", exeDir);
		char fallbackResolved[MAX_PATH];
		if (GetFullPathNameA(fallbackBin, MAX_PATH, fallbackResolved, NULL) > 0 && HasRequiredData(fallbackResolved, GetLangFull().c_str())) {
			sprintf_s(binBasePath, "%s", fallbackResolved);
			char repoRoot[MAX_PATH];
			sprintf_s(repoRoot, "%s\\..\\..\\..", exeDir);
			GetFullPathNameA(repoRoot, MAX_PATH, dataRoot, NULL);  // dataRoot = OpenJoey full path
			SetCurrentDirectoryA(dataRoot);  // CWD = OpenJoey so "data/y/title/..." resolves
		} else {
			SetCurrentDirectoryA(exeDir);
		}
	} else {
		SetCurrentDirectoryA(exeDir);
	}
	BOOL setCwdOk = (GetCurrentDirectoryA(MAX_PATH, dataRoot) != 0);
	if (!setCwdOk) {
		OutputDebugStringA("SetCurrentDirectory failed; scene data paths may not resolve.\n");
	}

	bool binInitOk = (binSystem_.Initialize(binBasePath, GetLangFull().c_str()) != 0);
	if (!binInitOk) {
		char fallbackBin[MAX_PATH];
		char fallbackResolved[MAX_PATH];
		sprintf_s(fallbackBin, "%s\\..\\..\\..\\data\\bin#", exeDir);
		if (GetFullPathNameA(fallbackBin, MAX_PATH, fallbackResolved, NULL) > 0 && strcmp(fallbackResolved, binBasePath) != 0) {
			// Retry with repo data (clean state first)
			binSystem_.Reset();
			char repoRoot[MAX_PATH];
			sprintf_s(repoRoot, "%s\\..\\..\\..", exeDir);
			GetFullPathNameA(repoRoot, MAX_PATH, dataRoot, NULL);
			SetCurrentDirectoryA(dataRoot);
			binInitOk = (binSystem_.Initialize(fallbackResolved, GetLangFull().c_str()) != 0);
			if (binInitOk) {
				sprintf_s(binBasePath, "%s", fallbackResolved);
			}
		}
	}

	if(!binInitOk)
    {
        char msg[1024];
		const char* lastFile = binSystem_.GetLastErrorFile();
		if (!lastFile || lastFile[0] == '\0') lastFile = "(unknown)";
        sprintf_s(msg,
            "Failed to initialize BinSystem.\n\n"
            "Card data is required but omitted from the repo to reduce legal risk.\n"
            "Copy the data folder from a legally acquired Power of Chaos installation\n"
            "(e.g. yu-gi-oh_mod_data_stock_template) into the executable directory.\n\n"
            "Place card data in:\n%s\n\n"
			"Missing or unreadable file:\n%s\n\n"
			"Current working directory:\n%s\n\n"
            "Required: card_prop.bin, card_name%s.bin, card_id.bin, card_intid.bin, "
            "card_desc%s.bin, card_indx%s.bin, dlg_text%s.bin, dlg_indx%s.bin, card_pack.bin, "
            "and ..\\card\\list_card.txt, ..\\mini\\list_card.txt",
            binBasePath, lastFile, dataRoot,
			GetLangFull().c_str(), GetLangFull().c_str(), GetLangFull().c_str(), GetLangFull().c_str(), GetLangFull().c_str());
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

	// PLANE TEST (guard: default CPlane may have null get() if factory not set)
    CPlane bgplane;
    if (bgplane.get()) bgplane->Load("data/y/try/end_e.bmp");

    CPlane charaplane;
    if (charaplane.get()) charaplane->Load("data/y/tutorial/fuki/tutorial_win0.yga");

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
	m_sceneControl->JumpScene(SCENE_SPLASH);

	// fake settings (parse later registry)
	settings_.BitCount = 2;
	settings_.Volume = 50;
	settings_.WindowMode = true;

	int nPat = 0;
	int testInt = 0;
    while (IsThreadValid()){
        // If scene stack is empty (e.g. ReturnScene from a scene that was the only one), go to main menu so we never run with no scene
        if (m_sceneControl->IsEnd())
            m_sceneControl->JumpScene(SCENE_MAINMENU);

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
            if (charaplane.get()) pSecondary->BltNatural(charaplane.get(),0,0);
            // For transfers with transparency enabled and YGA images (images with alpha information),
            // it's clearer to use BltNatural
            if (bgplane.get())
                ISurfaceTransBlt::BlindBlt1(pSecondary, bgplane.get(), 0, 0, testInt, 0, 255, NULL);
            break;
                }
        case 1: {
            if (bgplane.get()) pSecondary->BlendBltFast(bgplane.get(),0,0,255-nFade);
            // then draw the background with fade effect!
			//pSecondary->GeneralBlt(CSurfaceInfo::EBltType::)
            nFade++;
            break;
                }
        case 2: {
            if (bgplane.get()) pSecondary->BltFast(bgplane.get(),0,0);
            if (charaplane.get()) {
                int sx,sy;
                charaplane->GetSize(sx,sy);
                SIZE dstsize = { sx*nFade>>8 , sy * nFade>>8 };
                pSecondary->BltNatural(charaplane.get(),sx/2,sy/2,nFade/2+128,&dstsize,NULL,NULL,4);
            }
            // In this case, the last parameter 4 specifies that coordinates
            // are relative to the center of the image
            nFade++;
            break;
                }
        case 3: {
            if (bgplane.get()) pSecondary->BltFast(bgplane.get(),0,0);
            if (charaplane.get()) {
                int sx,sy;
                charaplane->GetSize(sx,sy);
                SIZE dstsize = { sx*nFade>>8 , sy * nFade>>8 };
                pSecondary->AddColorBltFast(charaplane.get(),sx/2,sy/2,&dstsize,NULL,NULL,4);
            }
            // AddColor (additive blending) from alpha surface
            nFade++;
            break;
                }
        case 4: {
            if (bgplane.get()) pSecondary->BltFast(bgplane.get(),0,0);
            if (charaplane.get()) {
                int sx,sy;
                charaplane->GetSize(sx,sy);
                SIZE dstsize = { sx*nFade>>8 , sy * nFade>>8 };
                pSecondary->SubColorBltFast(charaplane.get(),sx/2,sy/2,&dstsize,NULL,NULL,4);
            }
            // SubColor (subtractive blending) from alpha surface
            nFade++;
            break;
                }
        case 5: {
            if (bgplane.get()) pSecondary->BltFast(bgplane.get(),0,0);
            if (charaplane.get()) {
                int sx,sy;
                charaplane->GetSize(sx,sy);
                SIZE dstsize = { sx*nFade>>8 , sy * nFade>>8 };
                pSecondary->AddColorBltFastFade(charaplane.get(),sx/2,sy/2,nFade,&dstsize,NULL,NULL,4);
            }
            // AddColor (additive blending) + Fade (decay specification) from alpha surface
            nFade++;
            break;
                }
        case 6: {
            if (bgplane.get()) pSecondary->BltFast(bgplane.get(),0,0);
            if (charaplane.get()) {
                int sx,sy;
                charaplane->GetSize(sx,sy);
                SIZE dstsize = { sx*nFade>>8 , sy * nFade>>8 };
                pSecondary->SubColorBltFastFade(charaplane.get(),sx/2,sy/2,nFade,&dstsize,NULL,NULL,4);
            }
            // SubColor (subtractive blending) + Fade (decay specification) from alpha surface
            nFade++;
            break;
                }
        }

        //pSecondary->BltNatural(text,20,400);

        GetDraw()->OnDraw();

        key.Input();
        if (key.IsKeyPush(0)) {  // ESC key
            // In a sub-scene (Settings, CardList, etc.): go back. On main menu: exit.
            if (!m_sceneControl->IsEnd() && m_sceneControl->GetSceneNo() != SCENE_MAINMENU && m_sceneControl->GetSceneNo() != SCENE_SPLASH)
                m_sceneControl->ReturnScene();
            else
                break;  // Exit when on splash or main menu
        }
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