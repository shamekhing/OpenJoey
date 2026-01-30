// CJoeySceneFactory.cpp
#include "stdafx.h"
#include "../CApp.h"
#include "../system/CJoeySceneFactory.h"

// Scene headers
#include "../test/CScene1.h"
#include "../test/CSceneYesNo.h"
#include "../test/CSceneSplash.h"
#include "../test/CSceneMainMenu.h"
#include "../test/CSceneSettings.h"
#include "../test/CSceneCardList.h"

// Created by derplayer
// Created on 2025-01-19 15:46:24
smart_ptr<IScene> CJoeySceneFactory::CreateScene(int nScene) {
    CBaseScene* scene = NULL;
    
    switch ((SCENE)nScene) {
        case SCENE1: scene = new CScene1(); break;
        case SCENE2: scene = new CScene1(); break;
        case SCENE3: scene = new CScene1(); break;
		case SCENE_SPLASH: scene = new CSceneSplash(); break;
		case SCENE_MAINMENU: scene = new CSceneMainMenu(); break;
		case SCENE_SETTINGS: scene = new CSceneSettings(); break;
		case SCENE_CARDLIST: scene = new CSceneCardList(); break;
		case SCENE_ISEND: scene = new CSceneYesNo(); break;
        default: return smart_ptr<IScene>(); // Return null on error
    }
    
    if (scene) {
        scene->SetApp(m_app);
    }
    
    return smart_ptr<IScene>(scene);
}