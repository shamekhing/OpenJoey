// CSceneDeckEditor.cpp
// Deck construction scene. Original implementation; deck file format is our own plain-text design.

#include "stdafx.h"
#include "CSceneDeckEditor.h"
#include "system/cards/CBinSystem.h"
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>

using namespace yaneuraoGameSDK3rd::Draw;

// Layout: Original game was 640x480. We draw at original positions for authentic look.
// The 800x600 gives us extra space on the right for the card list panel.
// Left panel (original deck_c coordinates, 640x480 design)
static const int PREVIEW_LEFT = 20, PREVIEW_TOP = 90, PREVIEW_W = 200, PREVIEW_H = 290;
static const int DETAIL_BOX_LEFT = 20, DETAIL_BOX_TOP = 390, DETAIL_BOX_W = 220, DETAIL_BOX_H = 100;
static const int DETAIL_SCROLLBAR_LEFT = DETAIL_BOX_LEFT + DETAIL_BOX_W + 2, DETAIL_SCROLLBAR_TOP = DETAIL_BOX_TOP, DETAIL_SCROLLBAR_W = 14, DETAIL_SCROLLBAR_H = DETAIL_BOX_H;
static const int DETAIL_LINE_HEIGHT = 11;
static const int COUNTER_LEFT = 220, COUNTER_TOP = 90, COUNTER_W = 36, COUNTER_H = 18;
static const int HEADER_LEFT = 20, HEADER_TOP = 8;
// Button positions will be read from definition files using GetXY()
// Center panel - positions from deck_c definition files (designed for 800x600)
// Zone highlights: light_d at (238,7), light_s at (238,395), light_f at (238,501)
static const int CENTER_LEFT = 238, CENTER_W = 280;
static const int ZONE_BTN_LEFT = 250, ZONE_BTN_TOP = 8, ZONE_BTN_W = 75, ZONE_BTN_H = 24;
static const int MAIN_ZONE_LEFT = 250, MAIN_ZONE_TOP = 25, MAIN_ZONE_W = 256, MAIN_ZONE_H = 360;  // Main deck (highlight at y=7)
static const int SIDE_ZONE_LEFT = 250, SIDE_ZONE_TOP = 405, SIDE_ZONE_W = 256, SIDE_ZONE_H = 85;  // Side deck (highlight at y=395)
static const int FUSION_ZONE_LEFT = 250, FUSION_ZONE_TOP = 510, FUSION_ZONE_W = 256, FUSION_ZONE_H = 80;  // Fusion deck (highlight at y=501)
static const int MINI_SLOT_W = 40, MINI_SLOT_H = 56;
static const int MAIN_COLS = 6, SIDE_COLS = 6, FUSION_COLS = 6;
// Right panel (starts after center panel)
static const int LIST_LEFT = 518, LIST_TOP_BAR = 8, LIST_TOP = 40;
static const int LIST_WIDTH = 256, ROW_HEIGHT = 48, LIST_VISIBLE_LINES = 11;
static const int LIST_ROW_MINI_W = 40, LIST_ROW_MINI_H = 56;
static const int TOP_BTN_W = 32, TOP_BTN_H = 28;
static const int SCROLLBAR_LEFT = 778, SCROLLBAR_TOP = 40, SCROLLBAR_W = 20, SCROLLBAR_HEIGHT = 500;
static const int SCREEN_W = 800, SCREEN_H = 600;
// Drawer
static const int DRAWER_HANDLE_WIDTH = 20;
static const int DRAWER_HANDLE_CLOSED_LEFT = 780;
static const int DRAWER_HANDLE_OPEN_LEFT = 520;

CSceneDeckEditor::~CSceneDeckEditor() {
    if (m_backButton) {
        delete m_backButton;
        m_backButton = nullptr;
    }
}

void CSceneDeckEditor::LoadCardList() {
    m_cardList.clear();
    m_miniFilenameByInternalId.clear();
    if (!m_bin) return;
    std::ifstream cardList("data/card/list_card.txt");
    if (!cardList.is_open()) return;

    std::string line;
    while (std::getline(cardList, line)) {
        if (line.empty()) continue;
        if (line.size() >= 1 && line[line.size() - 1] == '\r') line.erase(line.size() - 1);
        if (line.size() < 2 || line.substr(0, 2) != "//") continue;

        size_t idStart = line.find("[");
        size_t idEnd = line.find("]");
        if (idStart == std::string::npos || idEnd == std::string::npos || idEnd <= idStart) continue;

        std::string idStr = line.substr(idStart + 1, idEnd - idStart - 1);
        int realId = std::atoi(idStr.c_str());
        if (realId <= 0) continue;

        std::string nextLine;
        if (!std::getline(cardList, nextLine)) break;
        if (nextLine.size() >= 1 && nextLine[nextLine.size() - 1] == '\r') nextLine.erase(nextLine.size() - 1);
        if (nextLine.find(".bmp") == std::string::npos) continue;
        if (nextLine == "card_ura.bmp") continue;

        WORD intId = m_bin->GetCardInternalId(realId);
        const Card* c = m_bin->GetCard(intId);
        if (c) {
            m_cardList.push_back((int)intId);
            m_miniFilenameByInternalId[(int)intId] = nextLine;  // store filename for mini load
        }
    }
    cardList.close();
}

void CSceneDeckEditor::LoadDeckFromFile() {
    m_mainDeck.clear();
    m_sideDeck.clear();
    m_fusionDeck.clear();
    std::ifstream f("data/deck.txt");
    if (!f.is_open()) return;

    std::string line;
    int section = 0; // 0=none, 1=main, 2=side, 3=fusion
    while (std::getline(f, line)) {
        if (!line.empty() && line[line.size() - 1] == '\r') line.erase(line.size() - 1);
        if (line.empty()) continue;

        if (line == "main") { section = 1; continue; }
        if (line == "side") { section = 2; continue; }
        if (line == "fusion") { section = 3; continue; }

        int id = std::atoi(line.c_str());
        if (id <= 0) continue;
        if (section == 1 && m_mainDeck.size() < (size_t)MAIN_DECK_MAX) m_mainDeck.push_back(id);
        else if (section == 2 && m_sideDeck.size() < (size_t)SIDE_DECK_MAX) m_sideDeck.push_back(id);
        else if (section == 3 && m_fusionDeck.size() < (size_t)FUSION_DECK_MAX) m_fusionDeck.push_back(id);
    }
    f.close();
}

void CSceneDeckEditor::SaveDeckToFile() {
    std::ofstream f("data/deck.txt");
    if (!f.is_open()) return;
    f << "main\n";
    for (size_t i = 0; i < m_mainDeck.size(); i++) f << m_mainDeck[i] << "\n";
    f << "side\n";
    for (size_t i = 0; i < m_sideDeck.size(); i++) f << m_sideDeck[i] << "\n";
    f << "fusion\n";
    for (size_t i = 0; i < m_fusionDeck.size(); i++) f << m_fusionDeck[i] << "\n";
    f.close();
}

void CSceneDeckEditor::AddCardToZone(int internalId) {
    if (!CanAddToZone((DeckZone)m_selectedZone)) return;
    if (m_selectedZone == 0 && m_mainDeck.size() < (size_t)MAIN_DECK_MAX) m_mainDeck.push_back(internalId);
    else if (m_selectedZone == 1 && m_sideDeck.size() < (size_t)SIDE_DECK_MAX) m_sideDeck.push_back(internalId);
    else if (m_selectedZone == 2 && m_fusionDeck.size() < (size_t)FUSION_DECK_MAX) m_fusionDeck.push_back(internalId);
}

void CSceneDeckEditor::RemoveCardFromZone(DeckZone zone, size_t index) {
    if (zone == ZONE_MAIN && index < m_mainDeck.size()) m_mainDeck.erase(m_mainDeck.begin() + index);
    else if (zone == ZONE_SIDE && index < m_sideDeck.size()) m_sideDeck.erase(m_sideDeck.begin() + index);
    else if (zone == ZONE_FUSION && index < m_fusionDeck.size()) m_fusionDeck.erase(m_fusionDeck.begin() + index);
}

bool CSceneDeckEditor::CanAddToZone(DeckZone zone) const {
    if (zone == ZONE_MAIN) return m_mainDeck.size() < (size_t)MAIN_DECK_MAX;
    if (zone == ZONE_SIDE) return m_sideDeck.size() < (size_t)SIDE_DECK_MAX;
    if (zone == ZONE_FUSION) return m_fusionDeck.size() < (size_t)FUSION_DECK_MAX;
    return false;
}

int CSceneDeckEditor::GetListIndexAt(int screenX, int screenY) {
    int listAreaLeft = LIST_LEFT + DRAWER_HANDLE_WIDTH;
    if (screenX < listAreaLeft || screenX >= LIST_LEFT + LIST_WIDTH) return -1;
    if (screenY < LIST_TOP || screenY >= LIST_TOP + LIST_VISIBLE_LINES * ROW_HEIGHT) return -1;
    std::vector<int> indices = GetFilteredSortedIndices();
    if (indices.empty()) return -1;
    int row = (screenY - LIST_TOP) / ROW_HEIGHT;
    int index = m_listScroll + row;
    if (index < 0 || (size_t)index >= indices.size()) return -1;
    return index;
}

int CSceneDeckEditor::GetZoneSlotIndexAt(DeckZone zone, int screenX, int screenY) {
    int zLeft = 0, zTop = 0, zW = 0, zH = 0, cols = 0;
    const std::vector<int>* v = nullptr;
    if (zone == ZONE_MAIN) {
        zLeft = MAIN_ZONE_LEFT; zTop = MAIN_ZONE_TOP; zW = MAIN_ZONE_W; zH = MAIN_ZONE_H; cols = MAIN_COLS;
        v = &m_mainDeck;
    } else if (zone == ZONE_SIDE) {
        zLeft = SIDE_ZONE_LEFT; zTop = SIDE_ZONE_TOP; zW = SIDE_ZONE_W; zH = SIDE_ZONE_H; cols = SIDE_COLS;
        v = &m_sideDeck;
    } else {
        zLeft = FUSION_ZONE_LEFT; zTop = FUSION_ZONE_TOP; zW = FUSION_ZONE_W; zH = FUSION_ZONE_H; cols = FUSION_COLS;
        v = &m_fusionDeck;
    }
    if (screenX < zLeft || screenX >= zLeft + zW || screenY < zTop || screenY >= zTop + zH) return -1;
    int col = (screenX - zLeft) / MINI_SLOT_W;
    int row = (screenY - zTop) / MINI_SLOT_H;
    int slot = row * cols + col;
    if (slot < 0 || (size_t)slot >= v->size()) return -1;
    return slot;
}

std::vector<int> CSceneDeckEditor::GetFilteredSortedIndices() const {
    std::vector<int> out;
    for (size_t i = 0; i < m_cardList.size(); i++) {
        int id = m_cardList[i];
        const Card* c = m_bin ? m_bin->GetCard(id) : nullptr;
        if (!c) continue;
        MonsterType mt = c->properties.GetMonsterType();
        if (m_listFilter == LIST_FILTER_MONSTER && (mt == TYPE_SPELLCARD || mt == TYPE_TRAPCARD)) continue;
        if (m_listFilter == LIST_FILTER_SPELL && mt != TYPE_SPELLCARD) continue;
        if (m_listFilter == LIST_FILTER_TRAP && mt != TYPE_TRAPCARD) continue;
        out.push_back((int)i);
    }
    auto cmp = [this](int a, int b) {
        const Card* ca = m_bin ? m_bin->GetCard(m_cardList[a]) : nullptr;
        const Card* cb = m_bin ? m_bin->GetCard(m_cardList[b]) : nullptr;
        if (!ca || !cb) return a < b;
        if (m_listSort == LIST_SORT_NAME) return strcmp(ca->name.name, cb->name.name) < 0;
        if (m_listSort == LIST_SORT_ATK) return ca->properties.GetAttackValue() > cb->properties.GetAttackValue();
        if (m_listSort == LIST_SORT_LEVEL) return ca->properties.GetMonsterStars() > cb->properties.GetMonsterStars();
        return a < b;
    };
    std::sort(out.begin(), out.end(), cmp);
    return out;
}

int CSceneDeckEditor::GetQuantityInDeck(int internalId) const {
    int n = 0;
    for (size_t i = 0; i < m_mainDeck.size(); i++) if (m_mainDeck[i] == internalId) n++;
    for (size_t i = 0; i < m_sideDeck.size(); i++) if (m_sideDeck[i] == internalId) n++;
    for (size_t i = 0; i < m_fusionDeck.size(); i++) if (m_fusionDeck[i] == internalId) n++;
    return n;
}

CFastPlane* CSceneDeckEditor::GetOrLoadMini(int internalId) {
    auto it = m_miniCache.find(internalId);
    if (it != m_miniCache.end() && !it->second.isNull()) return it->second.getPointer();
    // Use filename from list_card.txt (matches CSceneCardList - ensures correct mini)
    std::string filename;
    auto fnIt = m_miniFilenameByInternalId.find(internalId);
    if (fnIt != m_miniFilenameByInternalId.end()) {
        filename = fnIt->second;
    } else {
        const Card* c = m_bin ? m_bin->GetCard(internalId) : nullptr;
        if (!c || !c->imageMiniFilename) return nullptr;
        filename = c->imageMiniFilename;
    }
    char path[512];
    sprintf_s(path, sizeof(path), "data/mini/%s", filename.c_str());
    smart_ptr<CFastPlane> p(new CFastPlane());
    if (p->Load(path) != 0) {
        // Fallback: try full card image from data/card/ (show front, not back)
        const Card* c = m_bin ? m_bin->GetCard(internalId) : nullptr;
        if (c && c->imageFilename) {
            sprintf_s(path, sizeof(path), "data/card/%s", c->imageFilename);
            if (p->Load(path) == 0) {
                m_miniCache[internalId] = p;
                return p.getPointer();
            }
        }
        return nullptr;
    }
    m_miniCache[internalId] = p;
    return p.getPointer();
}

void CSceneDeckEditor::OnInit() {
    try {
    m_mouse.Flush();
    m_mouse.SetGuardTime(1);
    m_bin = app ? app->GetBinSystem() : NULL;
    m_background = smart_ptr<ISurface>();
    m_deckCBackground = CPlane();
    m_useDeckCBack = false;

    bool listSceneOk = false;
    if (app) {
        m_vPlaneLoader.SetLang(app->GetLang());
        m_vPlaneLoader.SetReadDir("data/y/list/");
        listSceneOk = (m_vPlaneLoader.Set("data/y/list/list_scene.txt", false) == 0);
        if (!listSceneOk)
            OutputDebugStringA("DeckEditor: Failed to load data/y/list/list_scene.txt\n");
        m_vPlaneLoader.SetColorKey(ISurface::makeRGB(0, 255, 0, 128));

        // Optional: load deck_c assets (data/y/deck_c/) for background, Back, Load/Save, detail bar
        const char* deckCDir = "data/y/deck_c/";
        m_vDeckCKabegami.SetLang(app->GetLang());
        m_vDeckCKabegami.SetReadDir(deckCDir);
        if (m_vDeckCKabegami.Set(std::string(deckCDir) + "kabegami.txt", false) == 0) {
            m_deckCBackground = m_vDeckCKabegami.GetPlane(0);
            m_vDeckCKabegami.SetColorKey(ISurface::makeRGB(0, 255, 0, 128));
        }
        m_vDeckCBack.SetLang(app->GetLang());
        m_vDeckCBack.SetReadDir(deckCDir);
        if (m_vDeckCBack.Set(std::string(deckCDir) + "btn_back.txt", false) == 0) {
            m_vDeckCBack.SetColorKey(ISurface::makeRGB(0, 255, 0, 128));
            m_useDeckCBack = true;
        }
        m_vDeckCLoadBtn.SetLang(app->GetLang());
        m_vDeckCLoadBtn.SetReadDir(deckCDir);
        if (m_vDeckCLoadBtn.Set(std::string(deckCDir) + "btn_load.txt", false) == 0) {
            m_vDeckCLoadBtn.SetColorKey(ISurface::makeRGB(0, 255, 0, 128));
            m_loadBtnOk = true;
        }
        m_vDeckCSaveBtn.SetLang(app->GetLang());
        m_vDeckCSaveBtn.SetReadDir(deckCDir);
        if (m_vDeckCSaveBtn.Set(std::string(deckCDir) + "btn_save.txt", false) == 0) {
            m_vDeckCSaveBtn.SetColorKey(ISurface::makeRGB(0, 255, 0, 128));
            m_saveBtnOk = true;
        }
        m_vDeckCDetailBar.SetLang(app->GetLang());
        m_vDeckCDetailBar.SetReadDir(deckCDir);
        if (m_vDeckCDetailBar.Set(std::string(deckCDir) + "detail_bar.txt", false) == 0) {
            m_vDeckCDetailBar.SetColorKey(ISurface::makeRGB(0, 255, 0, 128));
            m_detailBarOk = true;
        }

        // Load kaban_detail.txt for detail box background (kabegami_l_?.bmp)
        m_vDeckCKabanDetail.SetLang(app->GetLang());
        m_vDeckCKabanDetail.SetReadDir(deckCDir);
        if (m_vDeckCKabanDetail.Set(std::string(deckCDir) + "kaban_detail.txt", false) == 0) {
            m_vDeckCKabanDetail.SetColorKey(ISurface::makeRGB(0, 255, 0, 128));
            m_kabanDetailOk = true;
        }

        // Load kaban_list.txt for list container background
        m_vDeckCKabanList.SetLang(app->GetLang());
        m_vDeckCKabanList.SetReadDir(deckCDir);
        if (m_vDeckCKabanList.Set(std::string(deckCDir) + "kaban_list.txt", false) == 0) {
            m_vDeckCKabanList.SetColorKey(ISurface::makeRGB(0, 255, 0, 128));
            m_kabanListOk = true;
        }

        // Load kaban_swap.txt for counter frames (plane 10 = maisuu.bmp)
        m_vDeckCKabanSwap.SetLang(app->GetLang());
        m_vDeckCKabanSwap.SetReadDir(deckCDir);
        if (m_vDeckCKabanSwap.Set(std::string(deckCDir) + "kaban_swap.txt", false) == 0) {
            m_vDeckCKabanSwap.SetColorKey(ISurface::makeRGB(0, 255, 0, 128));
            m_kabanSwapOk = true;
        }

        // Load font.txt for fonts and icons
        m_vDeckCFonts.SetLang(app->GetLang());
        m_vDeckCFonts.SetReadDir(deckCDir);
        if (m_vDeckCFonts.Set(std::string(deckCDir) + "font.txt", false) == 0) {
            m_vDeckCFonts.SetColorKey(ISurface::makeRGB(0, 255, 0, 128));
            m_fontsOk = true;
        }

        // Load filter buttons (kbtn_*.txt)
        m_vDeckCFilterAll.SetLang(app->GetLang());
        m_vDeckCFilterAll.SetReadDir(deckCDir);
        if (m_vDeckCFilterAll.Set(std::string(deckCDir) + "kbtn_all.txt", false) == 0) {
            m_vDeckCFilterAll.SetColorKey(ISurface::makeRGB(0, 255, 0, 128));
            m_filterAllOk = true;
        }

        m_vDeckCFilterEff.SetLang(app->GetLang());
        m_vDeckCFilterEff.SetReadDir(deckCDir);
        if (m_vDeckCFilterEff.Set(std::string(deckCDir) + "kbtn_eff.txt", false) == 0) {
            m_vDeckCFilterEff.SetColorKey(ISurface::makeRGB(0, 255, 0, 128));
            m_filterEffOk = true;
        }

        m_vDeckCFilterMag.SetLang(app->GetLang());
        m_vDeckCFilterMag.SetReadDir(deckCDir);
        if (m_vDeckCFilterMag.Set(std::string(deckCDir) + "kbtn_mag.txt", false) == 0) {
            m_vDeckCFilterMag.SetColorKey(ISurface::makeRGB(0, 255, 0, 128));
            m_filterMagOk = true;
        }

        m_vDeckCFilterTrp.SetLang(app->GetLang());
        m_vDeckCFilterTrp.SetReadDir(deckCDir);
        if (m_vDeckCFilterTrp.Set(std::string(deckCDir) + "kbtn_trp.txt", false) == 0) {
            m_vDeckCFilterTrp.SetColorKey(ISurface::makeRGB(0, 255, 0, 128));
            m_filterTrpOk = true;
        }

        m_vDeckCFilterNor.SetLang(app->GetLang());
        m_vDeckCFilterNor.SetReadDir(deckCDir);
        if (m_vDeckCFilterNor.Set(std::string(deckCDir) + "kbtn_nor.txt", false) == 0) {
            m_vDeckCFilterNor.SetColorKey(ISurface::makeRGB(0, 255, 0, 128));
            m_filterNorOk = true;
        }

        m_vDeckCFilterFus.SetLang(app->GetLang());
        m_vDeckCFilterFus.SetReadDir(deckCDir);
        if (m_vDeckCFilterFus.Set(std::string(deckCDir) + "kbtn_fus.txt", false) == 0) {
            m_vDeckCFilterFus.SetColorKey(ISurface::makeRGB(0, 255, 0, 128));
            m_filterFusOk = true;
        }

        m_vDeckCFilterRit.SetLang(app->GetLang());
        m_vDeckCFilterRit.SetReadDir(deckCDir);
        if (m_vDeckCFilterRit.Set(std::string(deckCDir) + "kbtn_rit.txt", false) == 0) {
            m_vDeckCFilterRit.SetColorKey(ISurface::makeRGB(0, 255, 0, 128));
            m_filterRitOk = true;
        }

        // Load cycle button (kbtn_win.txt)
        m_vDeckCCycle.SetLang(app->GetLang());
        m_vDeckCCycle.SetReadDir(deckCDir);
        if (m_vDeckCCycle.Set(std::string(deckCDir) + "kbtn_win.txt", false) == 0) {
            m_vDeckCCycle.SetColorKey(ISurface::makeRGB(0, 255, 0, 128));
            m_cycleOk = true;
        }

        // Load sort buttons (filter_small.txt)
        m_vDeckCFilterSort.SetLang(app->GetLang());
        m_vDeckCFilterSort.SetReadDir(deckCDir);
        if (m_vDeckCFilterSort.Set(std::string(deckCDir) + "filter_small.txt", false) == 0) {
            m_vDeckCFilterSort.SetColorKey(ISurface::makeRGB(0, 255, 0, 128));
            m_filterSortOk = true;
        }

        // Load list scrollbar from data/y/list/ (hazard stripe style)
        m_vListScrollbar.SetLang(app->GetLang());
        m_vListScrollbar.SetReadDir("data/y/list/");
        if (m_vListScrollbar.Set("data/y/list/detail_scroll.txt", false) == 0) {
            m_vListScrollbar.SetColorKey(ISurface::makeRGB(0, 255, 0, 128));
            m_listScrollbarOk = true;
        }
    }

    bool backFromList = listSceneOk && !m_useDeckCBack;
    bool backFromDeckC = m_useDeckCBack;
    if (backFromList || backFromDeckC) {
        m_backButton = new CGUIButton();
        m_backButton->SetID(1);
        m_backButton->SetMouse(smart_ptr<CFixMouse>(&m_mouse, false));
        smart_ptr<CGUIButtonEventListener> backListener(new CGUINormalButtonListener());
        CGUINormalButtonListener* pBack = static_cast<CGUINormalButtonListener*>(backListener.getPointer());
        if (backFromDeckC) {
            pBack->SetPlaneLoader(smart_ptr<CPlaneLoader>(&m_vDeckCBack, false), 0);
            POINT backPos = m_vDeckCBack.GetXY(0);
            m_backButton->SetXY(backPos.x, backPos.y);
        } else {
            pBack->SetPlaneLoader(smart_ptr<CPlaneLoader>(&m_vPlaneLoader, false), 1);
            m_backButton->SetXY(61, 548); // Fallback position from original layout
        }
        m_backButton->SetEvent(backListener);
    } else {
        m_backButton = nullptr;
    }

    // Get button positions and sizes from definition files
    POINT loadPos = m_loadBtnOk ? m_vDeckCLoadBtn.GetXY(0) : POINT{112, 59};
    POINT savePos = m_saveBtnOk ? m_vDeckCSaveBtn.GetXY(0) : POINT{10, 59};
    int loadW = 100, loadH = 24, saveW = 100, saveH = 24;
    if (m_loadBtnOk) {
        CPlane lpPlane = m_vDeckCLoadBtn.GetPlane(0);
        if (lpPlane.get()) { int w, h; lpPlane.get()->GetSize(w, h); loadW = w; loadH = h; }
    }
    if (m_saveBtnOk) {
        CPlane spPlane = m_vDeckCSaveBtn.GetPlane(0);
        if (spPlane.get()) { int w, h; spPlane.get()->GetSize(w, h); saveW = w; saveH = h; }
    }
    m_rectLoad = RECT{ loadPos.x, loadPos.y, loadPos.x + loadW, loadPos.y + loadH };
    m_rectSave = RECT{ savePos.x, savePos.y, savePos.x + saveW, savePos.y + saveH };
    m_rectZoneMain  = RECT{ ZONE_BTN_LEFT, ZONE_BTN_TOP, ZONE_BTN_LEFT + ZONE_BTN_W, ZONE_BTN_TOP + ZONE_BTN_H };
    m_rectZoneSide  = RECT{ ZONE_BTN_LEFT + ZONE_BTN_W + 4, ZONE_BTN_TOP, ZONE_BTN_LEFT + 2*ZONE_BTN_W + 4, ZONE_BTN_TOP + ZONE_BTN_H };
    m_rectZoneFusion= RECT{ ZONE_BTN_LEFT + 2*ZONE_BTN_W + 8, ZONE_BTN_TOP, ZONE_BTN_LEFT + 3*ZONE_BTN_W + 8, ZONE_BTN_TOP + ZONE_BTN_H };

    m_previewCardId = -1;
    m_detailScroll = 0;
    m_listFilter = LIST_FILTER_ALL;
    m_listSort = LIST_SORT_NAME;
    m_listScroll = 0;
    m_selectedZone = 0;
    m_highlightListIndex = -1;
    m_scrollbarDragging = false;

    smart_ptr<CFastPlane> ura(new CFastPlane());
    if (ura->Load("data/card/card_ura.bmp") == 0)
        m_previewCardPlaneUra = ura;

    LoadCardList();
    LoadDeckFromFile();
    } catch (...) {
        m_backButton = nullptr;
        m_bin = NULL;
    }
}

void CSceneDeckEditor::OnMove(const smart_ptr<ISurface>& lp) {
    if (lp.isNull()) return;
    try {
    key.Input();
    m_mouse.Flush();

    if (m_backButton) {
        m_backButton->OnSimpleMove(lp.getPointer());
        if (m_backButton->IsLClick()) {
            GetSceneControl()->ReturnScene();
            return;
        }
    }

    int mx, my;
    m_mouse.GetXY(mx, my);
    bool click = m_mouse.IsPushLButton();
    m_loadOver = (mx >= m_rectLoad.left && mx < m_rectLoad.right && my >= m_rectLoad.top && my < m_rectLoad.bottom);
    m_saveOver = (mx >= m_rectSave.left && mx < m_rectSave.right && my >= m_rectSave.top && my < m_rectSave.bottom);
    std::vector<int> indices = GetFilteredSortedIndices();

    if (click) {
        if (mx >= m_rectLoad.left && mx < m_rectLoad.right && my >= m_rectLoad.top && my < m_rectLoad.bottom) {
            LoadDeckFromFile();
        } else if (mx >= m_rectSave.left && mx < m_rectSave.right && my >= m_rectSave.top && my < m_rectSave.bottom) {
            SaveDeckToFile();
        } else if (mx >= m_rectZoneMain.left && mx < m_rectZoneMain.right && my >= m_rectZoneMain.top && my < m_rectZoneMain.bottom) {
            m_selectedZone = 0;
        } else if (mx >= m_rectZoneSide.left && mx < m_rectZoneSide.right && my >= m_rectZoneSide.top && my < m_rectZoneSide.bottom) {
            m_selectedZone = 1;
        } else if (mx >= m_rectZoneFusion.left && mx < m_rectZoneFusion.right && my >= m_rectZoneFusion.top && my < m_rectZoneFusion.bottom) {
            m_selectedZone = 2;
        } else if (mx >= DRAWER_HANDLE_CLOSED_LEFT && mx < SCREEN_W && my >= 0 && my < SCREEN_H) {
            if (!m_drawerOpen) m_drawerOpen = true;
        } else if (m_drawerOpen && mx >= DRAWER_HANDLE_OPEN_LEFT && mx < DRAWER_HANDLE_OPEN_LEFT + DRAWER_HANDLE_WIDTH && my >= 0 && my < SCREEN_H) {
            m_drawerOpen = false;
        } else if (m_drawerOpen && mx >= LIST_LEFT + DRAWER_HANDLE_WIDTH && mx < LIST_LEFT + LIST_WIDTH && my >= LIST_TOP && my < LIST_TOP + LIST_VISIBLE_LINES * ROW_HEIGHT) {
            int row = (my - LIST_TOP) / ROW_HEIGHT;
            int idx = m_listScroll + row;
            if (idx >= 0 && (size_t)idx < indices.size()) {
                int internalId = m_cardList[indices[idx]];
                m_previewCardId = internalId;
                AddCardToZone(internalId);
            }
        } else if (m_drawerOpen && mx >= SCROLLBAR_LEFT && mx < SCROLLBAR_LEFT + SCROLLBAR_W && my >= SCROLLBAR_TOP && my < SCROLLBAR_TOP + SCROLLBAR_HEIGHT) {
            int totalRows = (int)indices.size();
            if (totalRows > LIST_VISIBLE_LINES) {
                int maxScroll = totalRows - LIST_VISIBLE_LINES;
                int thumbH = (SCROLLBAR_HEIGHT * LIST_VISIBLE_LINES) / totalRows;
                if (thumbH < 20) thumbH = 20;
                int thumbY = SCROLLBAR_TOP + (m_listScroll * (SCROLLBAR_HEIGHT - thumbH)) / maxScroll;
                int pageStep = LIST_VISIBLE_LINES / 2;
                if (pageStep < 1) pageStep = 1;
                if (my < thumbY)
                    m_listScroll = (m_listScroll > pageStep) ? m_listScroll - pageStep : 0;
                else if (my >= thumbY + thumbH)
                    m_listScroll = (m_listScroll < maxScroll - pageStep) ? m_listScroll + pageStep : maxScroll;
                else
                    m_scrollbarDragging = true;
                m_scrollbarDragStartY = my;
            }
        } else if (mx >= DETAIL_SCROLLBAR_LEFT && mx < DETAIL_SCROLLBAR_LEFT + DETAIL_SCROLLBAR_W && my >= DETAIL_SCROLLBAR_TOP && my < DETAIL_SCROLLBAR_TOP + DETAIL_SCROLLBAR_H && m_detailScrollMax > 0) {
            int thumbH = (DETAIL_SCROLLBAR_H * DETAIL_BOX_H) / (DETAIL_BOX_H + m_detailScrollMax);
            if (thumbH < 8) thumbH = 8;
            int trackH = DETAIL_SCROLLBAR_H - thumbH;
            int thumbY = DETAIL_SCROLLBAR_TOP + (m_detailScroll * trackH) / m_detailScrollMax;
            int pageStep = DETAIL_LINE_HEIGHT * 3;
            if (my < thumbY)
                m_detailScroll = (m_detailScroll > pageStep) ? m_detailScroll - pageStep : 0;
            else if (my >= thumbY + thumbH)
                m_detailScroll = (m_detailScroll < m_detailScrollMax - pageStep) ? m_detailScroll + pageStep : m_detailScrollMax;
            else
                m_detailScrollDragging = true;
        } else if (m_drawerOpen && mx >= LIST_LEFT + DRAWER_HANDLE_WIDTH + 4 && mx < LIST_LEFT + DRAWER_HANDLE_WIDTH + 4 + TOP_BTN_W && my >= LIST_TOP_BAR && my < LIST_TOP_BAR + TOP_BTN_H) {
            // Cycle button click - currently cycles sort
            m_listSort = (ListSort)(((int)m_listSort + 1) % 3);
        } else if (m_drawerOpen && mx >= LIST_LEFT + DRAWER_HANDLE_WIDTH + 4 + TOP_BTN_W + 4 && mx < LIST_LEFT + DRAWER_HANDLE_WIDTH + 4 + TOP_BTN_W + 64 && my >= LIST_TOP_BAR && my < LIST_TOP_BAR + TOP_BTN_H) {
            // Filter button click
            m_listFilter = (ListFilter)(((int)m_listFilter + 1) % 4);
        } else if (m_drawerOpen && mx >= LIST_LEFT + DRAWER_HANDLE_WIDTH + 4 + TOP_BTN_W + 64 + 8 && mx < LIST_LEFT + DRAWER_HANDLE_WIDTH + 4 + 2*TOP_BTN_W + 72 && my >= LIST_TOP_BAR && my < LIST_TOP_BAR + TOP_BTN_H) {
            // Sort button click
            m_listSort = (ListSort)(((int)m_listSort + 1) % 3);
        } else {
            int zi = GetZoneSlotIndexAt(ZONE_MAIN, mx, my);
            if (zi >= 0) {
                m_previewCardId = (size_t)zi < m_mainDeck.size() ? m_mainDeck[zi] : -1;
                RemoveCardFromZone(ZONE_MAIN, (size_t)zi);
                return;
            }
            zi = GetZoneSlotIndexAt(ZONE_SIDE, mx, my);
            if (zi >= 0) {
                m_previewCardId = (size_t)zi < m_sideDeck.size() ? m_sideDeck[zi] : -1;
                RemoveCardFromZone(ZONE_SIDE, (size_t)zi);
                return;
            }
            zi = GetZoneSlotIndexAt(ZONE_FUSION, mx, my);
            if (zi >= 0) {
                m_previewCardId = (size_t)zi < m_fusionDeck.size() ? m_fusionDeck[zi] : -1;
                RemoveCardFromZone(ZONE_FUSION, (size_t)zi);
                return;
            }
        }
    }

    if (m_scrollbarDragging && (!m_mouse.IsPushLButton() || !m_drawerOpen)) m_scrollbarDragging = false;
    if (m_detailScrollDragging && !m_mouse.IsPushLButton()) m_detailScrollDragging = false;
    if (m_detailScrollDragging && m_detailScrollMax > 0) {
        int thumbH = (DETAIL_SCROLLBAR_H * DETAIL_BOX_H) / (DETAIL_BOX_H + m_detailScrollMax);
        if (thumbH < 8) thumbH = 8;
        int trackH = DETAIL_SCROLLBAR_H - thumbH;
        int relY = my - DETAIL_SCROLLBAR_TOP;
        if (relY < 0) relY = 0;
        if (relY > trackH) relY = trackH;
        m_detailScroll = (relY * m_detailScrollMax) / trackH;
        if (m_detailScroll > m_detailScrollMax) m_detailScroll = m_detailScrollMax;
    }
    if (m_drawerOpen && m_scrollbarDragging) {
        int totalRows = (int)indices.size();
        if (totalRows > LIST_VISIBLE_LINES) {
            int maxScroll = totalRows - LIST_VISIBLE_LINES;
            int thumbH = (SCROLLBAR_HEIGHT * LIST_VISIBLE_LINES) / totalRows;
            if (thumbH < 20) thumbH = 20;
            int trackH = SCROLLBAR_HEIGHT - thumbH;
            int relY = my - SCROLLBAR_TOP;
            if (relY < 0) relY = 0;
            if (relY > trackH) relY = trackH;
            m_listScroll = (relY * maxScroll) / trackH;
            if (m_listScroll < 0) m_listScroll = 0;
            if (m_listScroll > maxScroll) m_listScroll = maxScroll;
        }
    }

    int listIdx = -1;
    if (m_drawerOpen && mx >= LIST_LEFT + DRAWER_HANDLE_WIDTH && mx < LIST_LEFT + LIST_WIDTH && my >= LIST_TOP && my < LIST_TOP + LIST_VISIBLE_LINES * ROW_HEIGHT) {
        int row = (my - LIST_TOP) / ROW_HEIGHT;
        listIdx = m_listScroll + row;
        if (listIdx >= 0 && (size_t)listIdx < indices.size())
            m_previewCardId = m_cardList[indices[listIdx]];
    }
    m_highlightListIndex = m_drawerOpen ? listIdx : -1;

    if (m_previewCardId != m_lastPreviewCardId) {
        m_lastPreviewCardId = m_previewCardId;
        m_detailScroll = 0;
        if (m_previewCardId < 0) {
            m_previewCardPlane = smart_ptr<CFastPlane>();
            m_detailScrollMax = 0;
        } else if (m_bin) {
            const Card* c = m_bin->GetCard(m_previewCardId);
            if (c && c->imageFilename) {
                char path[512];
                sprintf_s(path, sizeof(path), "data/card/%s", c->imageFilename);
                smart_ptr<CFastPlane> p(new CFastPlane());
                if (p->Load(path) == 0) m_previewCardPlane = p;
                else m_previewCardPlane = smart_ptr<CFastPlane>();
            }
            if (c && c->description) {
                size_t len = strlen(c->description);
                int estLines = (int)((len + 39) / 40);
                if (estLines < 1) estLines = 1;
                int contentH = estLines * DETAIL_LINE_HEIGHT;
                m_detailScrollMax = contentH > DETAIL_BOX_H ? contentH - DETAIL_BOX_H : 0;
            } else {
                m_detailScrollMax = 0;
            }
        }
    }

    // Mouse wheel: list scroll (when over list/scrollbar) or detail scroll (when over detail box)
    bool listWheelArea = m_drawerOpen && (
        (mx >= LIST_LEFT + DRAWER_HANDLE_WIDTH && mx < LIST_LEFT + LIST_WIDTH && my >= LIST_TOP && my < LIST_TOP + LIST_VISIBLE_LINES * ROW_HEIGHT) ||
        (mx >= SCROLLBAR_LEFT && mx < SCROLLBAR_LEFT + SCROLLBAR_W && my >= SCROLLBAR_TOP && my < SCROLLBAR_TOP + SCROLLBAR_HEIGHT));
    bool detailWheelArea = (mx >= DETAIL_BOX_LEFT && mx < DETAIL_SCROLLBAR_LEFT + DETAIL_SCROLLBAR_W && my >= DETAIL_BOX_TOP && my < DETAIL_BOX_TOP + DETAIL_BOX_H);
    if (listWheelArea && (m_mouse.IsWheelUp() || m_mouse.IsWheelDown())) {
        int totalRows = (int)indices.size();
        int maxScroll = totalRows - LIST_VISIBLE_LINES;
        if (maxScroll > 0) {
            if (m_mouse.IsWheelUp()) m_listScroll = (m_listScroll > 2) ? m_listScroll - 3 : 0;
            else m_listScroll = (m_listScroll < maxScroll - 2) ? m_listScroll + 3 : maxScroll;
            m_mouse.ResetButton();
        }
    } else if (detailWheelArea && m_detailScrollMax > 0 && (m_mouse.IsWheelUp() || m_mouse.IsWheelDown())) {
        if (m_mouse.IsWheelUp()) m_detailScroll = (m_detailScroll > DETAIL_LINE_HEIGHT * 2) ? m_detailScroll - DETAIL_LINE_HEIGHT * 2 : 0;
        else m_detailScroll = (m_detailScroll < m_detailScrollMax - DETAIL_LINE_HEIGHT * 2) ? m_detailScroll + DETAIL_LINE_HEIGHT * 2 : m_detailScrollMax;
        m_mouse.ResetButton();
    }

    if (m_drawerOpen) {
        if (key.IsKeyPush(5)) {
            if (m_listScroll > 0) m_listScroll--;
        }
        if (key.IsKeyPush(6)) {
            int maxScroll = (int)indices.size() - LIST_VISIBLE_LINES;
            if (maxScroll < 0) maxScroll = 0;
            if (m_listScroll < maxScroll) m_listScroll++;
        }
    }
    } catch (...) { /* keep scene alive if move fails */ }
}

static const char* AttributeAbbrev(MonsterAttribute a) {
    switch (a) {
        case ATTR_DIVINE_BEAST_NO_TRIBUTE: case ATTR_DIVINE_BEAST_2_TRIBUTE: return "DIV";
        case ATTR_LIGHT_NO_TRIBUTE: case ATTR_LIGHT_2_TRIBUTE: return "LGT";
        case ATTR_DARK_NO_TRIBUTE: case ATTR_DARK_2_TRIBUTE: return "DRK";
        case ATTR_WATER_NO_TRIBUTE: case ATTR_WATER_2_TRIBUTE: return "WAT";
        case ATTR_FIRE_NO_TRIBUTE: case ATTR_FIRE_2_TRIBUTE: return "FIR";
        case ATTR_EARTH_NO_TRIBUTE: case ATTR_EARTH_2_TRIBUTE: return "ERT";
        case ATTR_WIND_NO_TRIBUTE: case ATTR_WIND_2_TRIBUTE: return "WND";
        default: return "--";
    }
}

static void DrawTextAt(CFastDraw* draw, const smart_ptr<ISurface>& lp, int x, int y, const char* str, COLORREF rgb, int fontSize = 12, int fontHeight = 14) {
    if (!draw || lp.isNull()) return;
    try {
        CTextFastPlane tp(draw);
        tp.GetFont()->SetSize(fontSize);
        tp.GetFont()->SetHeight(fontHeight);
        tp.GetFont()->SetColor(rgb);
        tp.GetFont()->SetText(str ? str : "");
        if (tp.UpdateTextAA() != 0) return;
        lp->BltFast(&tp, x, y);
    } catch (...) { /* skip */ }
}

void CSceneDeckEditor::DrawLeftPanel(const smart_ptr<ISurface>& lp) {
    CFastDraw* draw = app ? app->GetDraw() : nullptr;
    if (!draw || lp.isNull()) return;
    // REF: Power of Chaos shows brick wall + metal - from kabegami_c.bmp. Do NOT clear over it.
    if (!m_deckCBackground.get()) {
        lp->SetFillColor(ISurface::makeRGB(80, 60, 50, 0));
        { RECT r = { 0, 0, CENTER_LEFT, SCREEN_H }; lp->Clear(&r); }
    }
    DrawTextAt(draw, lp, HEADER_LEFT, HEADER_TOP, "DECK CONSTRUCTION", RGB(255,255,255), 14, 16);

    // Draw Load button at position from btn_load.txt (112, 59)
    if (m_loadBtnOk) {
        CPlane p = m_vDeckCLoadBtn.GetPlane(m_loadOver ? 1 : 0);
        POINT loadPos = m_vDeckCLoadBtn.GetXY(0);
        if (p.get()) lp->BltFast(p.get(), loadPos.x, loadPos.y);
    } else {
        lp->SetFillColor(ISurface::makeRGB(60, 50, 40, 0));
        { RECT r = m_rectLoad; lp->Clear(&r); }
        DrawTextAt(draw, lp, m_rectLoad.left + 4, m_rectLoad.top + 2, "Load", RGB(255,255,255));
    }

    // Draw Save button at position from btn_save.txt (10, 59)
    if (m_saveBtnOk) {
        CPlane p = m_vDeckCSaveBtn.GetPlane(m_saveOver ? 1 : 0);
        POINT savePos = m_vDeckCSaveBtn.GetXY(0);
        if (p.get()) lp->BltFast(p.get(), savePos.x, savePos.y);
    } else {
        lp->SetFillColor(ISurface::makeRGB(60, 50, 40, 0));
        { RECT r = m_rectSave; lp->Clear(&r); }
        DrawTextAt(draw, lp, m_rectSave.left + 4, m_rectSave.top + 2, "Save", RGB(255,255,255));
    }

    if (!m_previewCardPlane.isNull())
        lp->BltFast(m_previewCardPlane.getPointer(), PREVIEW_LEFT, PREVIEW_TOP);
    else if (!m_previewCardPlaneUra.isNull())
        lp->BltFast(m_previewCardPlaneUra.getPointer(), PREVIEW_LEFT, PREVIEW_TOP);

    // Detail box: dark background for readable text (kaban_detail can make text unreadable)
    lp->SetFillColor(ISurface::makeRGB(32, 28, 38, 0));
    { RECT r = { DETAIL_BOX_LEFT, DETAIL_BOX_TOP, DETAIL_BOX_LEFT + DETAIL_BOX_W, DETAIL_BOX_TOP + DETAIL_BOX_H }; lp->Clear(&r); }
    if (m_detailScroll > m_detailScrollMax) m_detailScroll = m_detailScrollMax;
    if (m_detailScroll < 0) m_detailScroll = 0;
    if (m_previewCardId >= 0 && m_bin) {
        const Card* c = m_bin->GetCard(m_previewCardId);
        if (c) {
            const char* typeStr = "[MONSTER]";
            if (c->properties.GetMonsterType() == TYPE_SPELLCARD) typeStr = "[SPELL CARD]";
            else if (c->properties.GetMonsterType() == TYPE_TRAPCARD) typeStr = "[TRAP CARD]";
            DrawTextAt(draw, lp, PREVIEW_LEFT, PREVIEW_TOP + PREVIEW_H + 2, typeStr, RGB(200,200,255), 10, 12);
            DrawTextAt(draw, lp, PREVIEW_LEFT, PREVIEW_TOP + PREVIEW_H + 14, c->name.name, RGB(255,255,255), 10, 12);
            if (c->description) {
                // Draw description in detail box with high-contrast white text
                char buf[256];
                strncpy_s(buf, c->description, 200);
                buf[200] = '\0';
                DrawTextAt(draw, lp, DETAIL_BOX_LEFT + 4, DETAIL_BOX_TOP + 4 - m_detailScroll, buf, RGB(240,240,250), 10, 12);
            }
        }
    }
    if (m_detailBarOk) {
        CPlane track = m_vDeckCDetailBar.GetPlane(0);
        CPlane upArr = m_vDeckCDetailBar.GetPlane(2);
        CPlane downArr = m_vDeckCDetailBar.GetPlane(3);
        if (track.get()) {
            int tw = 0, th = 0;
            track.get()->GetSize(tw, th);
            for (int y = DETAIL_SCROLLBAR_TOP; y < DETAIL_SCROLLBAR_TOP + DETAIL_SCROLLBAR_H; y += (th > 0 ? th : 1))
                lp->BltFast(track.get(), DETAIL_SCROLLBAR_LEFT, y);
        }
        if (upArr.get()) lp->BltFast(upArr.get(), DETAIL_SCROLLBAR_LEFT, DETAIL_SCROLLBAR_TOP);
        if (downArr.get()) {
            int dw = 0, dh = 0;
            downArr.get()->GetSize(dw, dh);
            lp->BltFast(downArr.get(), DETAIL_SCROLLBAR_LEFT, DETAIL_SCROLLBAR_TOP + DETAIL_SCROLLBAR_H - dh);
        }
        if (m_detailScrollMax > 0) {
            int thumbH = (DETAIL_SCROLLBAR_H * DETAIL_BOX_H) / (DETAIL_BOX_H + m_detailScrollMax);
            if (thumbH < 8) thumbH = 8;
            int thumbY = DETAIL_SCROLLBAR_TOP + (m_detailScroll * (DETAIL_SCROLLBAR_H - thumbH)) / m_detailScrollMax;
            lp->SetFillColor(ISurface::makeRGB(120, 100, 80, 0));
            { RECT r = { DETAIL_SCROLLBAR_LEFT + 2, thumbY, DETAIL_SCROLLBAR_LEFT + DETAIL_SCROLLBAR_W - 2, thumbY + thumbH }; lp->Clear(&r); }
        }
    } else {
        lp->SetFillColor(ISurface::makeRGB(50, 48, 60, 0));
        { RECT r = { DETAIL_SCROLLBAR_LEFT, DETAIL_SCROLLBAR_TOP, DETAIL_SCROLLBAR_LEFT + DETAIL_SCROLLBAR_W, DETAIL_SCROLLBAR_TOP + DETAIL_SCROLLBAR_H }; lp->Clear(&r); }
        if (m_detailScrollMax > 0) {
            int thumbH = (DETAIL_SCROLLBAR_H * DETAIL_BOX_H) / (DETAIL_BOX_H + m_detailScrollMax);
            if (thumbH < 8) thumbH = 8;
            int thumbY = DETAIL_SCROLLBAR_TOP + (m_detailScroll * (DETAIL_SCROLLBAR_H - thumbH)) / m_detailScrollMax;
            lp->SetFillColor(ISurface::makeRGB(120, 100, 80, 0));
            { RECT r = { DETAIL_SCROLLBAR_LEFT + 2, thumbY, DETAIL_SCROLLBAR_LEFT + DETAIL_SCROLLBAR_W - 2, thumbY + thumbH }; lp->Clear(&r); }
        }
    }

    // Calculate deck statistics
    int counts[8] = { 0 };
    if (m_bin) {
        for (size_t i = 0; i < m_mainDeck.size(); i++) {
            const Card* c = m_bin->GetCard(m_mainDeck[i]);
            if (c) {
                MonsterType mt = c->properties.GetMonsterType();
                if (mt != TYPE_SPELLCARD && mt != TYPE_TRAPCARD) counts[0]++;
                else if (mt == TYPE_SPELLCARD) counts[1]++;
                else counts[2]++;
            }
        }
        counts[3] = (int)m_mainDeck.size();
        counts[4] = (int)m_sideDeck.size();
        counts[5] = (int)m_fusionDeck.size();
        counts[6] = counts[3] + counts[4] + counts[5];
        counts[7] = counts[0] + counts[1] + counts[2];
    }
    const char* counterLabels[8] = { "MON", "SPL", "TRP", "MAIN", "SIDE", "FUS", "TOT", "DK" };
    
    // Draw counter modules using kaban_swap.txt plane 10 (maisuu.bmp) and font.txt plane 13 (font_maisuu)
    CPlane counterFrame = m_kabanSwapOk ? m_vDeckCKabanSwap.GetPlane(10) : CPlane();
    int frameW = COUNTER_W, frameH = COUNTER_H;
    if (counterFrame.get()) {
        counterFrame.get()->GetSize(frameW, frameH);
    }
    
    for (int i = 0; i < 8; i++) {
        char buf[16];
        sprintf_s(buf, sizeof(buf), "%d", counts[i]);
        int cy = COUNTER_TOP + i * (frameH + 2);
        
        if (counterFrame.get()) {
            // Draw maisuu.bmp frame at proper position
            lp->BltFast(counterFrame.get(), COUNTER_LEFT, cy);
            // Draw label (small text above or beside frame)
            DrawTextAt(draw, lp, COUNTER_LEFT - 36, cy + (frameH - 9) / 2, counterLabels[i], RGB(200,180,140), 8, 9);
            // Draw number inside frame - center horizontally
            DrawTextAt(draw, lp, COUNTER_LEFT + (frameW - (int)strlen(buf) * 6) / 2, cy + (frameH - 12) / 2, buf, RGB(255,220,100), 10, 12);
        } else {
            lp->SetFillColor(ISurface::makeRGB(80, 60, 40, 0));
            { RECT r = { COUNTER_LEFT, cy, COUNTER_LEFT + COUNTER_W, cy + COUNTER_H }; lp->Clear(&r); }
            DrawTextAt(draw, lp, COUNTER_LEFT + 2, cy + 1, buf, RGB(255,220,100), 9, 10);
        }
    }

    if (m_backButton) {
        smart_ptr<CGUIButtonEventListener> ev = m_backButton->GetEvent();
        CGUINormalButtonListener* p = ev.isNull() ? nullptr : static_cast<CGUINormalButtonListener*>(ev.getPointer());
        if (p) p->SetPlaneNumber(m_backButton->IsIn() ? 2 : 1);
        m_backButton->OnSimpleDraw(lp.getPointer());
    }
}

void CSceneDeckEditor::DrawCenterPanel(const smart_ptr<ISurface>& lp) {
    CFastDraw* draw = app ? app->GetDraw() : nullptr;
    if (!draw || lp.isNull()) return;
    
    // Center panel background - only clear if kabegami doesn't cover it
    if (!m_deckCBackground.get()) {
        lp->SetFillColor(ISurface::makeRGB(38, 36, 48, 0));
        { RECT r = { CENTER_LEFT, 0, LIST_LEFT, SCREEN_H }; lp->Clear(&r); }
    }
    
    // Draw zone highlight for selected zone (light_d/s/f.bmp from kabegami.txt planes 24-26)
    // Positions from kabegami.txt: light_d at (238,7), light_s at (238,395), light_f at (238,501)
    // Note: These are drawn UNDER the zones, so draw before zone rects
    if (m_deckCBackground.get()) {
        // Use zone highlights from kabegami.txt
        CPlane lightPlane;
        POINT lightPos = {0, 0};
        if (m_selectedZone == 0) {
            lightPlane = m_vDeckCKabegami.GetPlane(24); // light_d.bmp
            lightPos = m_vDeckCKabegami.GetXY(24);
        } else if (m_selectedZone == 1) {
            lightPlane = m_vDeckCKabegami.GetPlane(25); // light_s.bmp
            lightPos = m_vDeckCKabegami.GetXY(25);
        } else if (m_selectedZone == 2) {
            lightPlane = m_vDeckCKabegami.GetPlane(26); // light_f.bmp
            lightPos = m_vDeckCKabegami.GetXY(26);
        }
        if (lightPlane.get()) {
            lp->BltFast(lightPlane.get(), lightPos.x, lightPos.y);
        }
    }
    
    // Zone selector text labels
    DrawTextAt(draw, lp, ZONE_BTN_LEFT + 4, ZONE_BTN_TOP + 2, "Main", m_selectedZone == 0 ? RGB(255,255,0) : RGB(200,200,200));
    DrawTextAt(draw, lp, ZONE_BTN_LEFT + ZONE_BTN_W + 8, ZONE_BTN_TOP + 2, "Side", m_selectedZone == 1 ? RGB(255,255,0) : RGB(200,200,200));
    DrawTextAt(draw, lp, ZONE_BTN_LEFT + 2*ZONE_BTN_W + 12, ZONE_BTN_TOP + 2, "Fusion", m_selectedZone == 2 ? RGB(255,255,0) : RGB(200,200,200));

    // Draw zone cards; background from kabegami_c.bmp and zone highlights (light_d/s/f)
    auto drawZone = [&](int zLeft, int zTop, int zW, int zH, int cols, const std::vector<int>& deck, const char* label, int maxCount) {
        // Subtle zone background so mini cards are visible (semi-transparent dark)
        if (!m_deckCBackground.get()) {
            lp->SetFillColor(ISurface::makeRGB(40, 38, 48, 0));
            { RECT r = { zLeft, zTop, zLeft + zW, zTop + zH }; lp->Clear(&r); }
        }
        char buf[32];
        sprintf_s(buf, sizeof(buf), "%d / %d", (int)deck.size(), maxCount);
        DrawTextAt(draw, lp, zLeft, zTop - 14, label, RGB(255,255,255), 10, 12);
        DrawTextAt(draw, lp, zLeft + 100, zTop - 14, buf, RGB(200,200,200), 9, 10);
        for (size_t i = 0; i < deck.size(); i++) {
            int col = (int)i % cols;
            int row = (int)i / cols;
            int x = zLeft + col * MINI_SLOT_W;
            int y = zTop + row * MINI_SLOT_H;
            CFastPlane* mini = GetOrLoadMini(deck[i]);
            if (mini) lp->BltFast(mini, x, y);
        }
    };
    drawZone(MAIN_ZONE_LEFT, MAIN_ZONE_TOP, MAIN_ZONE_W, MAIN_ZONE_H, MAIN_COLS, m_mainDeck, "Main Deck", MAIN_DECK_MAX);
    drawZone(SIDE_ZONE_LEFT, SIDE_ZONE_TOP, SIDE_ZONE_W, SIDE_ZONE_H, SIDE_COLS, m_sideDeck, "Side Deck", SIDE_DECK_MAX);
    drawZone(FUSION_ZONE_LEFT, FUSION_ZONE_TOP, FUSION_ZONE_W, FUSION_ZONE_H, FUSION_COLS, m_fusionDeck, "Fusion Deck", FUSION_DECK_MAX);
}

void CSceneDeckEditor::DrawRightPanel(const smart_ptr<ISurface>& lp) {
    CFastDraw* draw = app ? app->GetDraw() : nullptr;
    if (!draw || lp.isNull()) return;
    // REF: Right panel has metal grid texture. Don't clear if kabegami covers it; else use fallback.
    if (!m_deckCBackground.get()) {
        lp->SetFillColor(ISurface::makeRGB(50, 48, 55, 0));
        { RECT r = { LIST_LEFT, 0, SCREEN_W, SCREEN_H }; lp->Clear(&r); }
    }

    if (!m_drawerOpen) {
        // Drawer closed: show only the handle tab on the right edge (click to open)
        lp->SetFillColor(ISurface::makeRGB(70, 60, 90, 0));
        { RECT r = { DRAWER_HANDLE_CLOSED_LEFT, 0, SCREEN_W, SCREEN_H }; lp->Clear(&r); }
        DrawTextAt(draw, lp, DRAWER_HANDLE_CLOSED_LEFT + 2, SCREEN_H / 2 - 8, ">", RGB(255,255,255), 14, 16);
        DrawTextAt(draw, lp, DRAWER_HANDLE_CLOSED_LEFT + 2, SCREEN_H / 2 - 24, "Cards", RGB(200,200,220), 9, 10);
        return;
    }

    // Drawer open: close handle on left edge of panel (click to close)
    lp->SetFillColor(ISurface::makeRGB(55, 48, 72, 0));
    { RECT r = { DRAWER_HANDLE_OPEN_LEFT, 0, DRAWER_HANDLE_OPEN_LEFT + DRAWER_HANDLE_WIDTH, SCREEN_H }; lp->Clear(&r); }
    DrawTextAt(draw, lp, DRAWER_HANDLE_OPEN_LEFT + 2, SCREEN_H / 2 - 8, "<", RGB(255,255,255), 14, 16);

    std::vector<int> indices = GetFilteredSortedIndices();
    int btnCycleLeft = LIST_LEFT + DRAWER_HANDLE_WIDTH + 4;
    int btnFilterLeft = btnCycleLeft + TOP_BTN_W + 4;
    int btnSortLeft = btnFilterLeft + 60 + 4;

    // Draw Cycle button (kbtn_win.txt - yajirusi_1/2.bmp circular arrow)
    if (m_cycleOk) {
        CPlane p = m_vDeckCCycle.GetPlane(0); // plane 0 = normal, plane 1 = pressed
        if (p.get()) lp->BltFast(p.get(), btnCycleLeft, LIST_TOP_BAR);
    } else {
        lp->SetFillColor(ISurface::makeRGB(60, 50, 70, 0));
        { RECT r = { btnCycleLeft, LIST_TOP_BAR, btnCycleLeft + TOP_BTN_W, LIST_TOP_BAR + TOP_BTN_H }; lp->Clear(&r); }
        DrawTextAt(draw, lp, btnCycleLeft + 8, LIST_TOP_BAR + 6, "O", RGB(255,255,255), 10, 12);
    }

    // Draw Filter button based on current filter state
    CPlane filterPlane;
    bool filterDrawn = false;
    if (m_listFilter == LIST_FILTER_ALL && m_filterAllOk) {
        filterPlane = m_vDeckCFilterAll.GetPlane(0);
        filterDrawn = filterPlane.get() != nullptr;
    } else if (m_listFilter == LIST_FILTER_MONSTER && m_filterNorOk) {
        filterPlane = m_vDeckCFilterNor.GetPlane(0); // Show Normal monster button for MON filter
        filterDrawn = filterPlane.get() != nullptr;
    } else if (m_listFilter == LIST_FILTER_SPELL && m_filterMagOk) {
        filterPlane = m_vDeckCFilterMag.GetPlane(0);
        filterDrawn = filterPlane.get() != nullptr;
    } else if (m_listFilter == LIST_FILTER_TRAP && m_filterTrpOk) {
        filterPlane = m_vDeckCFilterTrp.GetPlane(0);
        filterDrawn = filterPlane.get() != nullptr;
    }
    if (filterDrawn && filterPlane.get()) {
        lp->BltFast(filterPlane.get(), btnFilterLeft, LIST_TOP_BAR);
    } else {
        lp->SetFillColor(ISurface::makeRGB(60, 50, 70, 0));
        { RECT r = { btnFilterLeft, LIST_TOP_BAR, btnFilterLeft + 60, LIST_TOP_BAR + TOP_BTN_H }; lp->Clear(&r); }
        const char* filterText = m_listFilter == LIST_FILTER_ALL ? "ALL" : 
                                  (m_listFilter == LIST_FILTER_MONSTER ? "MON" : 
                                   (m_listFilter == LIST_FILTER_SPELL ? "SPL" : "TRP"));
        DrawTextAt(draw, lp, btnFilterLeft + 8, LIST_TOP_BAR + 6, filterText, RGB(255,255,255), 10, 12);
    }

    // Draw Sort button (filter_small.txt - planes 8-9 name, 0-1 atk, 6-7 level)
    if (m_filterSortOk) {
        int sortPlaneIdx = 8; // Default: name (ABC)
        if (m_listSort == LIST_SORT_ATK) sortPlaneIdx = 0;
        else if (m_listSort == LIST_SORT_LEVEL) sortPlaneIdx = 6;
        CPlane sortPlane = m_vDeckCFilterSort.GetPlane(sortPlaneIdx);
        if (sortPlane.get()) {
            lp->BltFast(sortPlane.get(), btnSortLeft, LIST_TOP_BAR);
        }
    } else {
        lp->SetFillColor(ISurface::makeRGB(60, 50, 70, 0));
        { RECT r = { btnSortLeft, LIST_TOP_BAR, btnSortLeft + TOP_BTN_W, LIST_TOP_BAR + TOP_BTN_H }; lp->Clear(&r); }
        const char* sortText = m_listSort == LIST_SORT_NAME ? "ABC" : 
                               (m_listSort == LIST_SORT_ATK ? "ATK" : "LV");
        DrawTextAt(draw, lp, btnSortLeft + 4, LIST_TOP_BAR + 6, sortText, RGB(255,255,255), 10, 12);
    }

    // Draw list container background (kaban_list.txt plane 0 = kaban.bmp)
    if (m_kabanListOk) {
        CPlane listBg = m_vDeckCKabanList.GetPlane(0);
        if (listBg.get()) {
            lp->BltFast(listBg.get(), LIST_LEFT + DRAWER_HANDLE_WIDTH, LIST_TOP);
        }
    }

    for (int i = 0; i < LIST_VISIBLE_LINES; i++) {
        int idx = m_listScroll + i;
        if (idx < 0 || (size_t)idx >= indices.size()) break;
        int internalId = m_cardList[indices[idx]];
        int y = LIST_TOP + i * ROW_HEIGHT;
        bool highlighted = (idx == m_highlightListIndex);
        DrawListRow(lp, idx, internalId, y, highlighted);
    }

    // Draw list scrollbar with hazard-stripe graphics (from data/y/list/detail_scroll.txt)
    int totalRows = (int)indices.size();
    if (m_listScrollbarOk) {
        // Plane 0 = scroll_bar.bmp (track - yellow/black hazard stripe)
        // Planes 1-3 = scroll_box.bmp (thumb)
        // Planes 4-6 = scroll_minus0/1.bmp (down arrows)
        // Planes 7-9 = scroll_plus0/1.bmp (up arrows)
        CPlane track = m_vListScrollbar.GetPlane(0);
        CPlane thumb = m_vListScrollbar.GetPlane(1);
        CPlane upArrow = m_vListScrollbar.GetPlane(7);
        CPlane downArrow = m_vListScrollbar.GetPlane(4);

        // Draw track (tile vertically)
        if (track.get()) {
            int tw = 0, th = 0;
            track.get()->GetSize(tw, th);
            if (th > 0) {
                for (int y = SCROLLBAR_TOP; y < SCROLLBAR_TOP + SCROLLBAR_HEIGHT; y += th)
                    lp->BltFast(track.get(), SCROLLBAR_LEFT, y);
            }
        }

        // Draw up arrow at top
        if (upArrow.get()) {
            lp->BltFast(upArrow.get(), SCROLLBAR_LEFT, SCROLLBAR_TOP);
        }

        // Draw down arrow at bottom
        if (downArrow.get()) {
            int dw = 0, dh = 0;
            downArrow.get()->GetSize(dw, dh);
            lp->BltFast(downArrow.get(), SCROLLBAR_LEFT, SCROLLBAR_TOP + SCROLLBAR_HEIGHT - dh);
        }

        // Draw thumb
        if (totalRows > LIST_VISIBLE_LINES && thumb.get()) {
            int maxScroll = totalRows - LIST_VISIBLE_LINES;
            int thumbH = (SCROLLBAR_HEIGHT * LIST_VISIBLE_LINES) / totalRows;
            if (thumbH < 20) thumbH = 20;
            int thumbY = maxScroll > 0 ? SCROLLBAR_TOP + (m_listScroll * (SCROLLBAR_HEIGHT - thumbH)) / maxScroll : SCROLLBAR_TOP;
            lp->BltFast(thumb.get(), SCROLLBAR_LEFT, thumbY);
        }
    } else {
        // Fallback: yellow/black hazard-stripe style to match Power of Chaos ref
        lp->SetFillColor(ISurface::makeRGB(220, 180, 0, 0));
        { RECT r = { SCROLLBAR_LEFT, SCROLLBAR_TOP, SCROLLBAR_LEFT + SCROLLBAR_W, SCROLLBAR_TOP + SCROLLBAR_HEIGHT }; lp->Clear(&r); }
        if (totalRows > LIST_VISIBLE_LINES) {
            int maxScroll = totalRows - LIST_VISIBLE_LINES;
            int thumbH = (SCROLLBAR_HEIGHT * LIST_VISIBLE_LINES) / totalRows;
            if (thumbH < 20) thumbH = 20;
            int thumbY = maxScroll > 0 ? SCROLLBAR_TOP + (m_listScroll * (SCROLLBAR_HEIGHT - thumbH)) / maxScroll : SCROLLBAR_TOP;
            lp->SetFillColor(ISurface::makeRGB(200, 50, 50, 0));
            { RECT r = { SCROLLBAR_LEFT + 2, thumbY, SCROLLBAR_LEFT + SCROLLBAR_W - 2, thumbY + thumbH }; lp->Clear(&r); }
        }
    }
}

void CSceneDeckEditor::DrawListRow(const smart_ptr<ISurface>& lp, int rowIndex, int internalId, int y, bool highlighted) {
    CFastDraw* draw = app ? app->GetDraw() : nullptr;
    if (!draw || lp.isNull()) return;
    const Card* c = m_bin ? m_bin->GetCard(internalId) : nullptr;
    if (!c) return;
    // REF: Power of Chaos uses alternating white/light grey rows; highlight on hover
    RECT rowRect = { LIST_LEFT + DRAWER_HANDLE_WIDTH, y, LIST_LEFT + LIST_WIDTH, y + ROW_HEIGHT };
    if (highlighted) {
        lp->SetFillColor(ISurface::makeRGB(100, 140, 180, 0));
        lp->Clear(&rowRect);
    } else if ((rowIndex % 2) == 1) {
        lp->SetFillColor(ISurface::makeRGB(60, 58, 68, 0));
        lp->Clear(&rowRect);
    }
    
    int rowLeft = LIST_LEFT + DRAWER_HANDLE_WIDTH + 4;
    CFastPlane* mini = GetOrLoadMini(internalId);
    if (mini) lp->BltFast(mini, rowLeft, y);
    DrawTextAt(draw, lp, rowLeft + LIST_ROW_MINI_W + 4, y + 2, c->name.name, RGB(255,255,255), 9, 11);
    
    char buf[80];
    int qty = GetQuantityInDeck(internalId);
    int statsX = rowLeft + LIST_ROW_MINI_W + 4;
    
    // Get icon planes if available
    CPlane starIcon;
    CPlane attrIcon;
    if (m_kabanListOk) {
        starIcon = m_vDeckCKabanList.GetPlane(5); // icon_star.bmp
    }
    if (m_fontsOk) {
        attrIcon = m_vDeckCFonts.GetPlane(7); // icon_zokusei.bmp (attribute icon)
    }
    
    if (c->properties.GetMonsterType() == TYPE_SPELLCARD || c->properties.GetMonsterType() == TYPE_TRAPCARD) {
        // Spell/Trap card - no stars or ATK/DEF
        const char* typeText = c->properties.GetMonsterType() == TYPE_SPELLCARD ? "SPELL" : "TRAP";
        DrawTextAt(draw, lp, statsX, y + 18, typeText, RGB(140,180,220), 8, 10);
    } else {
        // Monster card - show Lv, ATK/DEF, attribute in compact stats line
        int xOff = statsX;
        
        // Draw star icon (if available) and level
        if (starIcon.get()) {
            int iw = 0, ih = 0;
            starIcon.get()->GetSize(iw, ih);
            lp->BltFast(starIcon.get(), xOff, y + 14);
            xOff += (iw > 0 ? iw : 12) + 1;
        }
        sprintf_s(buf, sizeof(buf), "Lv%d", (int)c->properties.GetMonsterStars());
        DrawTextAt(draw, lp, xOff, y + 16, buf, RGB(255,220,100), 8, 10);
        xOff += 28;
        
        // Draw ATK/DEF
        sprintf_s(buf, sizeof(buf), "%d/%d", (int)c->properties.GetAttackValue(), (int)c->properties.GetDefenseValue());
        DrawTextAt(draw, lp, xOff, y + 16, buf, RGB(180,200,220), 8, 10);
        xOff += 48;
        
        // Draw attribute icon or text
        if (attrIcon.get()) {
            int aw = 0, ah = 0;
            attrIcon.get()->GetSize(aw, ah);
            lp->BltFast(attrIcon.get(), xOff, y + 14);
        } else {
            DrawTextAt(draw, lp, xOff, y + 16, AttributeAbbrev(c->properties.GetMonsterAttribute()), RGB(200,180,140), 8, 10);
        }
    }
    
    // Draw quantity in deck (red "xN") at right edge
    sprintf_s(buf, sizeof(buf), "x%d", qty);
    DrawTextAt(draw, lp, rowLeft + LIST_WIDTH - DRAWER_HANDLE_WIDTH - 36, y + 16, buf, RGB(255,100,100), 8, 10);
}

void CSceneDeckEditor::OnDraw(const smart_ptr<ISurface>& lp) {
    if (lp.isNull()) return;
    try {
        // Always clear to a single solid background first so no other scene or wrong art shows through.
        lp->SetFillColor(ISurface::makeRGB(48, 44, 52, 0));
        lp->Clear();
        // Optional: draw deck_c kabegami on top (can be wrong resolution or wrong asset; our panels draw on top).
        if (m_deckCBackground.get())
            lp->BltFast(m_deckCBackground.get(), 0, 0);
        DrawLeftPanel(lp);
        DrawCenterPanel(lp);
        DrawRightPanel(lp);
    } catch (...) { /* keep scene alive if draw fails */ }
}
