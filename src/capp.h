#ifndef __CApp_h__
#define __CApp_h__

#include "system/effects/yanePlaneEffectBlt.h"
#include "system/CJoeySceneFactory.h"

class CApp : public CAppFrame {
public:
    virtual void MainThread();

    /// Drawing
    CFastDraw* GetDraw() { return GetDrawFactory()->GetDraw(); }
    CFastPlaneFactory* GetDrawFactory() { return &planeFactory_; }
	//ISceneControl* GetSceneControl() { return m_sceneControl.get(); }

    // Helper to trigger exit confirmation
    void RequestExit() { m_bWindowClosing = true; }

    // Getters for scenes to use
    CMouse* GetMouse() { return &mouse_; }

	// Language
	string GetLang() { return "e"; }

	// WM_CLOSE
	LRESULT	OnPreClose(void);

protected:
    CFastPlaneFactory    planeFactory_;    // This contains CFastDraw, so you should draw through this
    CSoundFactory        soundFactory_;    // Same applies for Sound
    CMouse               mouse_;           // Mouse input handling
    
    bool m_bWindowClosing;
    smart_ptr<ISceneControl> m_sceneControl; // Store it as member now
};

#endif // __CApp_h__