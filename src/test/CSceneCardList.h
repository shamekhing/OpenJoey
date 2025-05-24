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

// Constants
const int CARD_ROWS = 5;
const int CARD_COLUMNS = 10;
const int CARDS_PER_PAGE = CARD_ROWS * CARD_COLUMNS;
const int CARD_WIDTH = 50;
const int CARD_HEIGHT = 72;

// Card info structure
struct CardInfo {
    int id;
    int templateId;
    bool isNew;
    std::string bmpName;  // Store the actual BMP filename (TODO: use bin DB for this later on)
};

class CSceneCardList : public CBaseScene {
public:
    virtual void OnInit();
    virtual void OnDraw(const smart_ptr<ISurface>& lp);
    virtual void OnMove(const smart_ptr<ISurface>& lp);
    virtual void Serialize(ISerialize&) {}

private:
    // Input handling
    CKey1 key;
    CFixMouse m_mouse;

    // Core components
    smart_ptr<ISurface> m_background;
    CPlaneLoader m_vPlaneLoader;        // For scene elements
    CPlaneLoader m_vDetailPlaneLoader;   // For detail elements

    // Animation state
    CSaturationCounter m_nFade;
	CSaturationCounter nFadeBG;
    CTimer m_timerMain;
    CInteriorCounter m_nCardAnimations[CARD_COLUMNS];
    CInteriorCounter m_nCardScaleAnimations[CARD_COLUMNS];
    bool m_bColumnAnimationStarted[CARD_COLUMNS];
    static const int ANIM_SPEED = 8;

    // Card grid and selection
    std::vector<CardInfo> m_ownedCards;
    int m_nCurrentPage;
    int m_nTotalPages;

    // UI Elements
    CPlane m_titlePlane;
    CPlane m_bgPlane;
    CPlane m_cardHoverBorder;
    CFastPlane m_cardPreviewImage;
    CGUIButton* m_backButton;
    CGUIButton* m_prevPageButton;
    CGUIButton* m_nextPageButton;

    // Card preview state
    int m_nPreviewCardId;
    bool m_bPreviewCardIsMonster;

    // Private methods
    void InitializeUI();
    void InitializePageControls();
    void LoadCardData();
    void DrawCardGrid(const smart_ptr<ISurface>& lp);
    void DrawCardPreview(const smart_ptr<ISurface>& lp);
    void DrawPagination(const smart_ptr<ISurface>& lp);
    void DrawCollectionRate(const smart_ptr<ISurface>& lp);
    void UpdateCardAnimations();
    void ChangePage(bool forward);
	void SetHoverButtonPlane(CGUIButton* btn, int id, bool negativeOrder);
};

#endif // CSCENECARDLIST_H