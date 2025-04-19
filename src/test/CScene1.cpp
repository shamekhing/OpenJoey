// CScene1.cpp
#include "CScene1.h"

// Created by derplayer
// Created on 2025-01-19 15:46:24
void CScene1::OnInit() {
    bgPlane->Load("data/y/try/end_e.bmp");
    
    // Create text planes
    pTextPtr = new CTextFastPlane;
    pTextPtr->GetFont()->SetText("OpenJoey scene handling test");
    pTextPtr->GetFont()->SetSize(20);
    pTextPtr->UpdateTextAA();
    pText = CPlane(pTextPtr);
	nFade.Set(0, 255, 16);

	// OpenJoey debug menu registered keys
	key.AddKey(32,0,DIK_0);
	key.AddKey(33,0,DIK_1);
	key.AddKey(34,0,DIK_2);
	key.AddKey(35,0,DIK_3);
	key.AddKey(36,0,DIK_4);
	key.AddKey(37,0,DIK_5);
	key.AddKey(38,0,DIK_6);
	key.AddKey(39,0,DIK_7);
	key.AddKey(40,0,DIK_8);
	key.AddKey(41,0,DIK_9);
	key.AddKey(42,0,DIK_A);
}

void CScene1::OnMove(const smart_ptr<ISurface>& lp) {
    key.Input();
}

void CScene1::OnDraw(const smart_ptr<ISurface>& lp) {
    lp->Clear();
    //lp->BltFast(bgPlane.get(), 0, 0);

	ISurfaceTransBlt::CircleBlt1(lp.get(), bgPlane.get(), 0, 0, (int)nFade, 0, 255);
	nFade.Inc();

    if (key.IsKeyPush(5)) {  // Space
		string exampleText = pTextPtr->GetFont()->GetText() + ".";
        pTextPtr->GetFont()->SetText(exampleText);
		pTextPtr->UpdateTextAA();

		//GetSceneControl()->CallSceneFast(SCENE_ISEND);
        //GetSceneControl()->JumpScene(SCENE_ISEND);
		//app->OnPreClose();
    }

    if (key.IsKeyPush(6)) {  // Enter
		GetSceneControl()->CallSceneFast(SCENE_ISEND);
        return;
    }

	if (key.IsKeyPush(32)) {  // 0
        GetSceneControl()->CallSceneFast(SCENE_SPLASH);
        return;
    }

	if (key.IsKeyPush(33)) {  // 1
        GetSceneControl()->CallSceneFast(SCENE_MAINMENU);
        return;
    }

	if (key.IsKeyPush(34)) {  // 2
        GetSceneControl()->CallSceneFast(SCENE_SETTINGS);
        return;
    }


	// Apply text to scene surface
    lp->BltNatural(pText,20,100);

}