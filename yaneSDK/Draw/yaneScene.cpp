#include "stdafx.h"
#include "yaneScene.h"

namespace yaneuraoGameSDK3rd {
namespace Draw {

void CSceneControl::CreateScene(int nScene) {
    if (nScene == -1) return; // Invalid scene number

    // Create a new scene info container
    CSceneInfo* pInfo = new CSceneInfo;
    pInfo->m_lpScene = GetSceneFactory()->CreateScene(nScene);
    pInfo->m_nScene = nScene;

    if (!pInfo->m_lpScene.isNull()) {
        // Set the scene control and initialize
        pInfo->m_lpScene->SetSceneControl(smart_ptr<ISceneControl>(this, false));
        pInfo->m_lpScene->OnInit();
    }

    // Add to scene stack
    GetSceneStack()->push_back(smart_ptr<CSceneInfo>(pInfo));
}

void CSceneControl::OnMove(const smart_ptr<ISurface>& lpDrawContext) {
    // Process all pending scene changes since OnInit might trigger multiple scene jumps
    do {
        int nMessage = m_nMessage;
        m_nMessage = 0;

        // Handle scene transition messages
        switch (nMessage) {
            // 0: No Message
            // 1: JumpScene - Destroy current, create new
            // 2: CallScene - Destroy current, create new (different cleanup)
            // 3: CallSceneFast - Keep current, create new
            // 4: ReturnScene - Return to previous scene
            // 5: ExitScene - Exit immediately
            case 0: break;

            case 1: // JumpScene
                if (!IsEnd()) {
                    GetSceneStack()->pop_back();
                }
                CreateScene(m_nNextScene);
                break;

            case 2: { // CallScene
                if (!IsEnd()) {
                    (*GetSceneStack()->rbegin())->m_lpScene.Delete();
                }
                CreateScene(m_nNextScene);
                break;
            }

            case 3: // CallSceneFast
                // Keep current scene in stack (smart pointer handles cleanup)
                CreateScene(m_nNextScene);
                break;

            case 4: { // ReturnScene
                if (IsEnd()) {
                    #ifdef USE_EXCEPTION
                    throw CSyntaxException("Can't return any further - no more scenes");
                    #endif
                } else {
                    GetSceneStack()->pop_back();
                }
                
                if (IsEnd()) break; // No more scenes to return to

                CSceneInfo& pInfo = **GetSceneStack()->rbegin();
                int nScene = pInfo.m_nScene;
                
                if (pInfo.m_lpScene.isNull()) {
                    // Scene was destroyed, recreate it
                    GetSceneStack()->pop_back();
                    CreateScene(nScene);
                    
                    // Notify scene it's being returned to
                    if (!(*GetSceneStack()->rbegin())->m_lpScene.isNull()) {
                        (*GetSceneStack()->rbegin())->m_lpScene->OnComeBack(nScene);
                    }
                } else {
                    // Scene exists, just notify it's being returned to
                    (*GetSceneStack()->rbegin())->m_lpScene->OnComeBack(nScene);
                }
                break;
            }

            case 5: // ExitScene
                return; // Exit without drawing
        }

        // Update current scene if it exists
        if (!IsEnd() && !(*GetSceneStack()->rbegin())->m_lpScene.isNull()) {
            (*GetSceneStack()->rbegin())->m_lpScene->OnMove(lpDrawContext);
        }

    } while (m_nMessage != 0); // Continue processing until no more messages
}

void CSceneControl::OnDraw(const smart_ptr<ISurface>& lpDrawContext) {
    // Draw current scene if it exists
    if (!IsEnd() && !(*GetSceneStack()->rbegin())->m_lpScene.isNull()) {
        (*GetSceneStack()->rbegin())->m_lpScene->OnDraw(lpDrawContext);
    }
}

void CSceneControl::PushScene(int nScene) {
    // Save scene number to stack (without creating the scene yet)
    CSceneInfo* pInfo = new CSceneInfo;
    pInfo->m_nScene = nScene;
    GetSceneStack()->push_back(smart_ptr<CSceneInfo>(pInfo));
}

bool CSceneControl::IsEnd() const {
    return const_cast<CSceneControl*>(this)->GetSceneStack()->size() == 0;
}

int CSceneControl::GetSceneNo() const {
    if (IsEnd()) return -1;
    return (*const_cast<CSceneControl*>(this)->GetSceneStack()->rbegin())->m_nScene;
}

smart_ptr<IScene> CSceneControl::GetScene() const {
    if (IsEnd()) return smart_ptr<IScene>();
    return (*const_cast<CSceneControl*>(this)->GetSceneStack()->rbegin())->m_lpScene;
}

} // end of namespace Draw
} // end of namespace yaneuraoGameSDK3rd