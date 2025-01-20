// CScene1.cpp
#include "CScene1.h"

// Created by derplayer
// Created on 2025-01-19 15:46:24
void CScene1::OnInit() {
    bgPlane->Load("data/y/try/end_e.bmp");
    
    // Create text planes
    textPlane.GetFont()->SetText("I am scene 1!");
    textPlane.UpdateTextAA();
	nFade.Set(0, 255, 8);
}

void CScene1::OnMove(const smart_ptr<ISurface>& lp) {
    key.Input();
}

void CScene1::OnDraw(const smart_ptr<ISurface>& lp) {

    //if (key.IsKeyPush(5)) {  // Space
        // Start transition effect
        ISurfaceTransBlt::CircleBlt1(lp.get(), bgPlane.get(), 0, 0, (int)nFade, 0, 255);
		int testA = (int)nFade;
		LRESULT res = ISurfaceTransBlt::BlindBlt1(
                lp.get(),    // destination surface
                bgPlane.get(),    // source surface
                0,                   // x position
                0,                   // y position
				testA,   // transition phase (0-256)
                0,                   // transition mode
                255,                 // fade rate
                NULL                 // clip rectangle
            );
		nFade.Inc();
    //}

    if (key.IsKeyPush(6)) {  // Return
        ISurfaceTransBlt::WhorlBlt1(lp.get(), bgPlane.get(), 0, 0, 255, 0);
        GetSceneControl()->PushScene(SCENE1);
        GetSceneControl()->JumpScene(SCENE3);
        return;
    }

    //lp->Clear();
    //lp->BltFast(bgPlane.get(), 0, 0);

    CTextFastPlane* pText = new CTextFastPlane;
    pText->GetFont()->SetText("OpenJoey scene handling test");
    pText->GetFont()->SetSize(20);
    pText->UpdateTextAA();
    CPlane text(pText);
    lp->BltNatural(text,20,100);

}