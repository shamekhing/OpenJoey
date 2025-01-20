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
}

void CScene1::OnMove(const smart_ptr<ISurface>& lp) {
    key.Input();
}

void CScene1::OnDraw(const smart_ptr<ISurface>& lp) {

    //lp->Clear();
    //lp->BltFast(bgPlane.get(), 0, 0);

	ISurfaceTransBlt::CircleBlt1(lp.get(), bgPlane.get(), 0, 0, (int)nFade, 0, 255);
	nFade.Inc();

    if (key.IsKeyPush(5)) {  // Space
		string exampleText = pTextPtr->GetFont()->GetText() + ".";
        pTextPtr->GetFont()->SetText(exampleText);
		pTextPtr->UpdateTextAA();
    }

    if (key.IsKeyPush(6)) {  // Enter
        GetSceneControl()->PushScene(SCENE1);
        GetSceneControl()->JumpScene(SCENE3);
        return;
    }

	// Apply text to scene surface
    lp->BltNatural(pText,20,100);

}