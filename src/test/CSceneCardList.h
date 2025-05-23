// CSceneCardList.h
// Created by derplayer
// Created on 2025-05-23 11:42:23

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

    // Animation counters
    CSaturationCounter m_nFade;
    CInteriorCounter m_nCardAnimations[CARDS_PER_PAGE];
    CInteriorCounter m_nCardScaleAnimations[CARDS_PER_PAGE];  // Added scale animation
    CTimer m_timerMain;

    // Card grid and selection
    smart_vector_ptr<CGUIButton> m_vButtons;
    std::vector<CardInfo> m_ownedCards;
    int m_nCurrentPage;
    int m_nTotalPages;

    // UI Elements
    CPlane m_titlePlane;                 // Title plane
    CPlane m_bgPlane;                    // Background plane
    CPlane m_cardHoverBorder;            // Card hover border from plane loader
    CFastPlane m_cardPreviewImage;       // Card preview surface
    CGUIButton* m_backButton;
    CGUIButton* m_prevPageButton;
    CGUIButton* m_nextPageButton;

    // Card preview elements
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
    void HandleCardHover(int cardIndex, int x, int y);
    void UpdateCardAnimations();
    void ChangePage(bool forward);
};

#endif // CSCENECARDLIST_H