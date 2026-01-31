// CSceneDeckEditor.h
// Deck construction scene. Original implementation; layout inspired by classic deck editor concepts.
// Deck file format is our own plain-text design (see README legal: no reverse-engineered formats).

#ifndef CSCENEDECKEDITOR_H
#define CSCENEDECKEDITOR_H

#include "../stdafx.h"
#include "../CApp.h"
#include "../system/CJoeySceneFactory.h"
#include "../system/backport/yaneGUIButton.h"
#include <map>

enum DeckZone { ZONE_MAIN = 0, ZONE_SIDE = 1, ZONE_FUSION = 2 };
enum ListFilter { LIST_FILTER_ALL = 0, LIST_FILTER_MONSTER, LIST_FILTER_SPELL, LIST_FILTER_TRAP };
enum ListSort { LIST_SORT_NAME = 0, LIST_SORT_ATK, LIST_SORT_LEVEL };
static const int MAIN_DECK_MIN = 40, MAIN_DECK_MAX = 60;
static const int SIDE_DECK_MAX = 15;
static const int FUSION_DECK_MAX = 15;

class CSceneDeckEditor : public CBaseScene {
public:
    virtual void OnInit();
    virtual void OnDraw(const smart_ptr<ISurface>& lp);
    virtual void OnMove(const smart_ptr<ISurface>& lp);
    virtual void Serialize(ISerialize&) {}
    virtual ~CSceneDeckEditor();

private:
    CKey1 key;
    CFixMouse m_mouse;

    smart_ptr<ISurface> m_background;
    CPlaneLoader m_vPlaneLoader;       // data/y/list/ for Back button fallback
    CPlaneLoader m_vDeckCKabegami;    // data/y/deck_c/ background (kabegami.txt)
    CPlaneLoader m_vDeckCBack;        // data/y/deck_c/ Back button (btn_back.txt)
    CPlaneLoader m_vDeckCLoadBtn;    // data/y/deck_c/ Load (Import) button (btn_load.txt)
    CPlaneLoader m_vDeckCSaveBtn;    // data/y/deck_c/ Save (Export) button (btn_save.txt)
    CPlaneLoader m_vDeckCDetailBar;  // data/y/deck_c/ detail scrollbar (detail_bar.txt)
    CPlaneLoader m_vDeckCKabanDetail;// data/y/deck_c/ detail box background (kaban_detail.txt)
    CPlaneLoader m_vDeckCKabanList;  // data/y/deck_c/ list container (kaban_list.txt)
    CPlaneLoader m_vDeckCKabanSwap;  // data/y/deck_c/ counter frames (kaban_swap.txt)
    CPlaneLoader m_vDeckCFonts;      // data/y/deck_c/ fonts and icons (font.txt)
    CPlaneLoader m_vDeckCFilterAll;  // data/y/deck_c/ ALL filter button (kbtn_all.txt)
    CPlaneLoader m_vDeckCFilterEff;  // data/y/deck_c/ Effect filter (kbtn_eff.txt)
    CPlaneLoader m_vDeckCFilterMag;  // data/y/deck_c/ Spell/Magic filter (kbtn_mag.txt)
    CPlaneLoader m_vDeckCFilterTrp;  // data/y/deck_c/ Trap filter (kbtn_trp.txt)
    CPlaneLoader m_vDeckCFilterNor;  // data/y/deck_c/ Normal filter (kbtn_nor.txt)
    CPlaneLoader m_vDeckCFilterFus;  // data/y/deck_c/ Fusion filter (kbtn_fus.txt)
    CPlaneLoader m_vDeckCFilterRit;  // data/y/deck_c/ Ritual filter (kbtn_rit.txt)
    CPlaneLoader m_vDeckCCycle;      // data/y/deck_c/ Cycle button (kbtn_win.txt)
    CPlaneLoader m_vDeckCFilterSort; // data/y/deck_c/ Sort buttons (filter_small.txt)
    CPlaneLoader m_vListScrollbar;   // data/y/list/ Hazard-stripe scrollbar (detail_scroll.txt)
    CPlane m_deckCBackground;        // background plane from deck_c when available
    bool m_useDeckCBack = false;     // true = Back button uses deck_c assets
    bool m_loadBtnOk = false;        // true = Load button graphics loaded
    bool m_saveBtnOk = false;        // true = Save button graphics loaded
    bool m_detailBarOk = false;      // true = detail scrollbar graphics loaded
    bool m_kabanDetailOk = false;    // true = detail box background loaded
    bool m_kabanListOk = false;      // true = list container graphics loaded
    bool m_kabanSwapOk = false;      // true = counter frame graphics loaded
    bool m_fontsOk = false;          // true = fonts and icons loaded
    bool m_filterAllOk = false;      // true = ALL filter button loaded
    bool m_filterEffOk = false;      // true = Effect filter button loaded
    bool m_filterMagOk = false;      // true = Spell/Magic filter button loaded
    bool m_filterTrpOk = false;      // true = Trap filter button loaded
    bool m_filterNorOk = false;      // true = Normal filter button loaded
    bool m_filterFusOk = false;      // true = Fusion filter button loaded
    bool m_filterRitOk = false;      // true = Ritual filter button loaded
    bool m_cycleOk = false;          // true = Cycle button loaded
    bool m_filterSortOk = false;     // true = Sort buttons loaded
    bool m_listScrollbarOk = false;  // true = list scrollbar graphics loaded
    bool m_loadOver = false;         // mouse over Load button
    bool m_saveOver = false;         // mouse over Save button
    CBinSystem* m_bin = nullptr;

    std::vector<int> m_mainDeck;   // internal card IDs
    std::vector<int> m_sideDeck;
    std::vector<int> m_fusionDeck;

    std::vector<int> m_cardList;   // internal IDs from list_card.txt
    int m_listScroll = 0;
    int m_selectedZone = 0;        // 0=main, 1=side, 2=fusion
    int m_highlightListIndex = -1;

    CGUIButton* m_backButton = nullptr;
    RECT m_rectLoad, m_rectSave, m_rectZoneMain, m_rectZoneSide, m_rectZoneFusion;

    smart_ptr<CFastPlane> m_previewCardPlane;
    smart_ptr<CFastPlane> m_previewCardPlaneUra;
    int m_previewCardId = -1;
    int m_lastPreviewCardId = -2;
    int m_detailScroll = 0;

    ListFilter m_listFilter = LIST_FILTER_ALL;
    ListSort m_listSort = LIST_SORT_NAME;
    std::map<int, smart_ptr<CFastPlane>> m_miniCache;
    bool m_scrollbarDragging = false;
    int m_scrollbarDragStartY = 0;
    bool m_detailScrollDragging = false;
    int m_detailScrollMax = 0;
    bool m_drawerOpen = true;   // right panel (card list) drawer: true = expanded, false = collapsed

    void LoadCardList();
    void LoadDeckFromFile();
    void SaveDeckToFile();
    void AddCardToZone(int internalId);
    void RemoveCardFromZone(DeckZone zone, size_t index);
    bool CanAddToZone(DeckZone zone) const;
    int GetListIndexAt(int screenX, int screenY);
    int GetZoneSlotIndexAt(DeckZone zone, int screenX, int screenY);

    std::vector<int> GetFilteredSortedIndices() const;
    int GetQuantityInDeck(int internalId) const;
    CFastPlane* GetOrLoadMini(int internalId);

    void DrawLeftPanel(const smart_ptr<ISurface>& lp);
    void DrawCenterPanel(const smart_ptr<ISurface>& lp);
    void DrawRightPanel(const smart_ptr<ISurface>& lp);
    void DrawListRow(const smart_ptr<ISurface>& lp, int rowIndex, int internalId, int y, bool highlighted = false);
};

#endif
