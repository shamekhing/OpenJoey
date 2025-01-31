// CScene1.h
#ifndef CSCENE1_H
#define CSCENE1_H

#include "../stdafx.h"
#include "../CApp.h"
#include "../system/CJoeySceneFactory.h"
#include "../system/effects/yanePlaneEffectBlt.h"

// Created by derplayer
// Created on 2025-01-19 15:46:24
class CScene1 : public CBaseScene {
public:
    virtual void OnInit();
    virtual void OnDraw(const smart_ptr<ISurface>& lp);
    virtual void OnMove(const smart_ptr<ISurface>& lp);
    virtual void Serialize(ISerialize&) {}

private:
    CPlane bgPlane;
    CTextFastPlane* pTextPtr;
	CPlane pText;
    CKey1 key;
	CRootCounter nFade;
};

#endif