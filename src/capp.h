#ifndef __CApp_h__
#define __CApp_h__

#include "system/effects/yanePlaneEffectBlt.h"
#include "system/CJoeySceneFactory.h"

class CApp : public CAppFrame {

    virtual void MainThread();

    /// Drawing
    CFastDraw* GetDraw() { return GetDrawFactory()->GetDraw(); }
    CFastPlaneFactory* GetDrawFactory() { return &planeFactory_; }

protected:
    CFastPlaneFactory    planeFactory_;
    // This contains CFastDraw, so you should draw through this

    CSoundFactory        soundFactory_;
    // Same applies for Sound
};

#endif // __CApp_h__