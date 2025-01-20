//
//  yaneScene.h:
//
//  Scene Management Class
//  Reference: Chapters 8, 9, 11, 13 of "Genius Game Programmer Training Course"
//  from Yaneurao Homepage
//

#ifndef __yaneScene_h__
#define __yaneScene_h__

/**
    Scene Management Class. Essential for games.
    For details, refer to Chapters 8, 9, 11, 13 of 
    "Genius Game Programmer Training Course" from Yaneurao Homepage

    Usage:
    1. Create a scene class by inheriting from both IScene and mediator
    2. Create a factory class by inheriting from both ISceneFactory and mediator,
       then pass its smart_ptr to CSceneControl

    After that, in the IScene-derived class created in step 1,
    you can freely move between scenes (jump/call) by specifying
    the name of the next scene you want to go to.
*/

#include "../Auxiliary/yaneSerialize.h"

namespace yaneuraoGameSDK3rd {
namespace Draw {

class IScene;
class ISurface;     // Abstract surface

class ISceneFactory {
/**
    Parameterized factory for scene construction.
    Derive from this class and register scenes there.
*/
public:
    virtual smart_ptr<IScene> CreateScene(int nScene) = 0;
    /// Creates and returns a scene class with number nScene
};

class ISceneParam {
/**
    Base class for parameters passed between scenes.
    Derive from this to create custom parameters for scene communication.
*/
public:
    virtual ~ISceneParam(){}
};

class ISceneControl {
/**
    Base class for scene control
*/
public:
    /// ------ Scene transition related. Changes take effect at next OnDraw timing.
    /// (This scene won't be drawn in the next OnDraw)
    /// Set the next scene to transition to
    virtual void    JumpScene(int eScene)=0;
    /// Call a scene (returns to this class with ReturnScene)
    /// Note: this class is deleted temporarily
    virtual void    CallScene(int eScene)=0;
    /// Call a scene (returns to this class with ReturnScene)
    /// Note: this class isn't deleted and remains
    /// OnCallSceneFast is called when transitioning to that scene
    virtual void    CallSceneFast(int eScene)=0;
    /// Return to the scene that called via CallScene/CallSceneFast
    virtual void    ReturnScene()=0;
    /// Exit the current scene
    virtual void    ExitScene()=0;
    /// --------------------------------------------------------------
    /// Push a scene onto the stack
    /// (When ReturnScene is called, scenes are called in reverse order)
    virtual void    PushScene(int eScene) =0;
    /// PopScene might be interesting but might not be proper scene management
    /// (violates global jump prohibition logic)
    /// --------------------------------------------------------------
    /// --- Drawing and movement are divided into these two phases
    /// Perform object drawing
    virtual void    OnDraw(const smart_ptr<ISurface>& lp)=0;
    /// Perform object movement
    virtual void    OnMove(const smart_ptr<ISurface>& lp)=0;

    virtual void SetSceneFactory(const smart_ptr<ISceneFactory>& pv)=0;
    virtual smart_ptr<ISceneFactory> GetSceneFactory() const=0;
    virtual bool IsEnd() const=0;
    virtual int GetSceneNo() const=0;
    virtual smart_ptr<IScene> GetScene() const=0;
    virtual ~ISceneControl(){}
};

class CSceneControl : public ISceneControl, public IArchive {
/**
    Controller for IScene (Scene class)
*/
public:
    CSceneControl(const smart_ptr<ISceneFactory>& pvSceneFactory)
        { SetSceneFactory(pvSceneFactory); }
    CSceneControl(){}
    /**
        Pass factory in constructor
        If missed, pass it using SetSceneFactory
    */
    virtual void SetSceneFactory(const smart_ptr<ISceneFactory>& pv)
        { m_pvSceneFactory = pv;}
    virtual smart_ptr<ISceneFactory> GetSceneFactory() const
        { return m_pvSceneFactory; }

    /// --- Control functions have same meaning as in IScene
    virtual void    JumpScene(int nScene)
        { m_nNextScene = nScene; m_nMessage=1; }
    virtual void    CallScene(int nScene)
        { m_nNextScene = nScene; m_nMessage=2; }
    virtual void    CallSceneFast(int nScene)
        { m_nNextScene = nScene; m_nMessage=3; }
    virtual void    ReturnScene(){ m_nMessage=4; }
    virtual void    ExitScene()   { m_nMessage=5; }
    virtual void    PushScene(int nScene);
    virtual void    OnDraw(const smart_ptr<ISurface>& lp);
    virtual void    OnMove(const smart_ptr<ISurface>& lp);

    /// Factory can be set/gotten. From within CScene derived class,
    /// you can create child scenes like:
    /// smart_ptr<IScene> p = GetSceneControl()->GetSceneFactory()->CreateScene(CSCENE1);

    /// ----- Properties
    /// Returns true if ReturnScene is called when there's no calling scene in stack
    virtual bool IsEnd() const;

    /// Returns current scene number
    /// Returns -1 if IsEnd()==true
    virtual int GetSceneNo() const;

    /// Returns current scene
    /// Returns NULL smart_ptr if IsEnd()==true
    virtual smart_ptr<IScene> GetScene() const;

protected:
    // Scene factory
    smart_ptr<ISceneFactory> m_pvSceneFactory;

    virtual void    CreateScene(int nScene);    // Internal scene creation
    // ----- Movement request messages
    int        m_nMessage;
        // 0:No Message 1:JumpScene 2:CallScene
        // 3:CallSceneFast 4:ReturnScene 5:ExitScene
    int            m_nNextScene;
        // Next scene to move to

    class CSceneInfo {
    public:
        smart_ptr<IScene>    m_lpScene;        // Scene pointer
        int                    m_nScene;        // Scene number
        CSceneInfo() {
            m_nScene = -1;    // Don't create anything
        }
    };
    // Stack of calling scenes
    smart_vector_ptr<CSceneInfo>    m_SceneInfoStack;
    smart_vector_ptr<CSceneInfo>*    GetSceneStack()
        { return &m_SceneInfoStack;}

    /// Used for data exchange between scenes
    smart_ptr<ISceneParam>    GetParam() const
        { return m_vParam; }
    void    SetParam(const smart_ptr<ISceneParam>& vParam)
        { m_vParam = vParam;}

    virtual void Serialize(ISerialize&){}
    /// ToDo: Wait! No time to implement this yet.

private:
    /// Used for small data exchanges between scenes (convenient ^^)
    smart_ptr<ISceneParam>    m_vParam;
};

class IScene : public IArchive {
/**
    Base Scene Class

Scene definition example:
enum SCENE {
    SCENE1,            // Scene 1
    SCENE2,            // Scene 2
};
*/
public:
    /// Override these as needed by user
    virtual void    OnDraw(const smart_ptr<ISurface>& lp) {}
    virtual void    OnMove(const smart_ptr<ISurface>& lp) {}

    /// Initialize with this function (outer can't be used in constructor)
    virtual void    OnInit() {}

    /// Called when returning from CallScene/CallSceneFast via ReturnScene
    /// nScene contains which scene we're returning from
    virtual void    OnComeBack(int nScene){}

    /// Set/Get scene controller
    void    SetSceneControl(const smart_ptr<ISceneControl>& lp) { m_lpSceneControl = lp; }
    smart_ptr<ISceneControl> GetSceneControl() const { return m_lpSceneControl; }
    /// --------------------------------------------------------------
    virtual void Serialize(ISerialize&){}
    /// Classes needing serialization should override this
    /// CSceneControl will provide serialization mechanism

    IScene(const smart_ptr<ISceneControl>& lp) : m_lpSceneControl(lp){}
    IScene() {}
    
    virtual ~IScene(){} // place holder

protected:
    smart_ptr<ISceneControl> m_lpSceneControl;
};

} // end of namespace Draw
} // end of namespace yaneuraoGameSDK3rd

#endif