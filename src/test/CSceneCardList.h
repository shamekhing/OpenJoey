// CSceneCardList.h
// Created by derplayer
// Created on 2025-05-23 11:47:22
// Offsets copypasted from https://github.com/CatchABus/ygo-emu-poc (MIT, thanks @CatchABus)

#ifndef CSCENECARDLIST_H
#define CSCENECARDLIST_H

#include "../stdafx.h"
#include "../CApp.h"
#include "../system/CJoeySceneFactory.h"
#include "../system/backport/yaneGUIButton.h"
#include "../system/backport/yaneGUISlider.h"
#include <map> // For card texture caching

// Constants
const int CARD_ROWS = 5;
const int CARD_COLUMNS = 10;
const int CARDS_PER_PAGE = CARD_ROWS * CARD_COLUMNS;
const int CARD_WIDTH = 50;
const int CARD_HEIGHT = 72;

// Original, working animation speeds for individual counters (Adjusted for faster animation based on video)
// These values control how many frames an individual card takes to animate from start to end.
const int ORIGINAL_CARD_ANIM_SPEED = 4;        // Speed for individual card entry/exit (Y-offset) - e.g., 6 frames for the drop
const int ORIGINAL_CARD_ANIM_SCALE_SPEED = 5;  // Speed for individual card scaling - e.g., 5 frames for the scale

// Delay for diagonal staggering (adjust this for wave speed)
const int DIAGONAL_STAGGER_DELAY_PER_STEP = 2; // e.g., 2 frames delay per diagonal step

// Add this line if it's not already there:
const int EXIT_DIAGONAL_STAGGER_DELAY_PER_STEP = 2; // Can be same as DIAGONAL_STAGGER_DELAY_PER_STEP


// Card info structure - ADDED ANIMATION MEMBERS
struct CardInfo {
    int id;
    int templateId;
    bool isNew;
    std::string bmpName;  // Store the actual BMP filename (TODO: use bin DB for this later on)
    
    // NEW: Per-card animation state
    yaneuraoGameSDK3rd::Math::CInteriorCounter m_nScaleAnimation;
    yaneuraoGameSDK3rd::Math::CInteriorCounter m_nYOffsetAnimation;
    bool m_bAnimationStarted;
    bool m_bAnimationCompleted;
};

// Scene animation states
enum SceneAnimState {
    SAS_INITIAL_LOAD,   // Initial scene entry animation (grid appears)
    SAS_IDLE,           // No grid animation active
    SAS_EXITING_GRID,   // Old grid scaling down
    SAS_ENTERING_GRID   // New grid scaling up
};


class CSceneCardList : public CBaseScene {
public:
    virtual void OnInit();
    virtual void OnDraw(const smart_ptr<ISurface>& lp);
    virtual void OnMove(const smart_ptr<ISurface>& lp);
    virtual void Serialize(ISerialize&) {}
    virtual ~CSceneCardList(); // Add destructor for memory cleanup

private:
    // Input handling
    CKey1 key;
    CFixMouse m_mouse;

    // Core components
    smart_ptr<ISurface> m_background;
    CPlaneLoader m_vPlaneLoader;     // For scene elements
    CPlaneLoader m_vDetailPlaneLoader;    // For detail elements

    // Animation state
    yaneuraoGameSDK3rd::Math::CSaturationCounter m_nFade;
    yaneuraoGameSDK3rd::Math::CSaturationCounter nFadeBG;
    CTimer m_timerMain;

    // NEW: Scene-wide animation state
    SceneAnimState m_sceneAnimState;
    bool m_bPageChangeRequested;
    bool m_bForwardPageChange; // True if moving to next page, false for previous

    // NEW: Scene-wide frame counter for staggered animation timing
    int m_currentSceneFrameCount; // Increments every OnMove frame.

    // Card grid and selection
    std::vector<CardInfo> m_ownedCards;
    int m_nCurrentPage;
    int m_nTotalPages;

    // UI Elements
    CPlane m_titlePlane;
    CPlane m_bgPlane;
    CPlane m_cardHoverBorder;
    CFastPlane m_cardPreviewImage;
    CGUIButton* m_backButton; // Will address this memory leak in .cpp for MSVC 2003
    CGUIButton* m_prevPageButton;
    CGUIButton* m_nextPageButton;

    // Card preview state
    int m_nPreviewCardId;
    bool m_bPreviewCardIsMonster;

    std::map<int, smart_ptr<CFastPlane> > m_cardTextures; // Stores raw card images as CFastPlane

    // Helper functions for diagonal step calculation based on origin
    int GetDiagonalStep_FromTopRight(int col, int row);   // Wave starts Top-Right, propagates Bottom-Left
    int GetDiagonalStep_FromTopLeft(int col, int row);    // Wave starts Top-Left, propagates Bottom-Right
    int GetDiagonalStep_FromBottomLeft(int col, int row); // Wave starts Bottom-Left, propagates Top-Right

    // Private methods
    void InitializeUI();
    void InitializePageControls();
    void LoadCardData();
    void LoadCardTexturesForCurrentPage(); // New method to load textures for the current page
    void ClearCardTextures();            // New method to clear textures
    void DrawCardGrid(const smart_ptr<ISurface>& lp);
    void DrawCardPreview(const smart_ptr<ISurface>& lp);
    void DrawPagination(const smart_ptr<ISurface>& lp);
    void DrawCollectionRate(const smart_ptr<ISurface>& lp);
    void UpdateCardAnimations(); // Will be heavily modified
    void ChangePage(bool forward); // Will be heavily modified
    void ResetCardAnimations(bool forNewPage = true); // New helper to init/reset card counters
    void SetHoverButtonPlane(CGUIButton* btn, int id, bool negativeOrder); 
};

#endif // CSCENECARDLIST_H