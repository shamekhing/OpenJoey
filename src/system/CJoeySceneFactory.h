// CJoeySceneFactory.h
#ifndef JOEY_SCENE_FACTORY_H
#define JOEY_SCENE_FACTORY_H
#include "../stdafx.h"

// Forward declare
class CApp;

// Created by derplayer
// Created on 2025-01-19 15:46:24
// Scene enumeration
enum SCENE {
    SCENE1,
    SCENE2,
    SCENE3,
	SCENE_SPLASH,
    SCENE_ISEND  // "Exit?" confirmation scene
};

// Base scene class
class CBaseScene : public IScene {
protected:
    CApp* app;  // Store reference to main app
public:
    void SetApp(CApp* _app) { app = _app; }
    
    // Required virtual functions from IScene
    virtual void OnInit() {}
    virtual void OnDraw(const smart_ptr<ISurface>& lp) {}
    virtual void OnMove(const smart_ptr<ISurface>& lp) {}
    virtual void OnComeBack(int nScene) {}
    virtual void Serialize(ISerialize&) {}
};

class CJoeySceneFactory : public ISceneFactory {
public:
    CJoeySceneFactory(CApp* app) : m_app(app) {}
    virtual smart_ptr<IScene> CreateScene(int nScene);

private:
    CApp* m_app;
};

#endif