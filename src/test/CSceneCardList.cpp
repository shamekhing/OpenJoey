// CSceneCardList.cpp
// Created by derplayer
// Created on 2025-05-23 10:30:43

#include "CSceneCardList.h"
#include <fstream>
#include <sstream>

void CSceneCardList::OnInit() {
    // Initialize input
    m_mouse.Flush();
    m_mouse.SetGuardTime(1);

    // Init counters and timers
    m_nFade = CSaturationCounter(0, 255, 8);
    m_timerMain = CTimer();
    m_timerMain.Restart();
    m_timerMain.Reset();

    // Initialize page tracking
    m_nCurrentPage = 1;
    m_nPreviewCardId = 0;
    m_bPreviewCardIsMonster = false;

    // Load scene layout data
    m_vPlaneLoader.SetLang(app->GetLang());
    m_vPlaneLoader.SetReadDir("data/y/list/");
    if (m_vPlaneLoader.Set("data/y/list/list_scene.txt", false) != 0) {
        OutputDebugStringA("Error: Failed to load data/y/list/list_scene.txt\n");
    }

    // Load detail layout data
    m_vDetailPlaneLoader.SetLang(app->GetLang());
    m_vDetailPlaneLoader.SetReadDir("data/y/list/");
    if (m_vDetailPlaneLoader.Set("data/y/list/list_detail.txt", false) != 0) {
        OutputDebugStringA("Error: Failed to load data/y/list/list_detail.txt\n");
    }

    // Clone fb from previous scene if needed
    smart_ptr<ISurface> screenPtr = app->GetDraw()->GetSecondary()->cloneFull();
    if (screenPtr.get()) {
        m_background = screenPtr;
        OutputDebugStringA("Framebuffer data cloned and loaded successfully!\n");
    }

    InitializeUI();
    LoadCardData();

    // Initialize card animations - start from above the slots and animate down
    for (int i = 0; i < CARDS_PER_PAGE; i++) {
        // Change from moving down from slot to moving into slot from above
        m_nCardAnimations[i].Set(-CARD_HEIGHT, 0, 20); // Start -CARD_HEIGHT pixels above final position
    }

    // Add scale animation counter
    for (int i = 0; i < CARDS_PER_PAGE; i++) {
        m_nCardScaleAnimations[i].Set(0, 100, 16); // 16 frames for scale animation (80% of 20 frames)
    }
}

void CSceneCardList::InitializeUI() {
    // Load background
    CPlane bgPlane = m_vDetailPlaneLoader.GetPlane(0); // list_bg.bmp
	POINT bgPos = { 0, 0 }; //HACK: this seems to be hardcoded in exe and the txt spec is not used? (POINT bgPos = m_vDetailPlaneLoader.GetXY(0);)
    bgPlane->SetPos(bgPos);
    m_bgPlane = bgPlane;

    // Create title (list_name_?.bmp based on language)
    CPlane titlePlane = m_vPlaneLoader.GetPlane(0);
    POINT titlePos = m_vPlaneLoader.GetXY(0);
    titlePlane->SetPos(titlePos);
    m_titlePlane = titlePlane;

    // Create back button
    CGUIButton* backBtn = new CGUIButton();
    backBtn->SetID(1);
    backBtn->SetMouse(smart_ptr<CFixMouse>(&m_mouse, false));
    smart_ptr<CGUIButtonEventListener> buttonListener(new CGUINormalButtonListener());
    CGUINormalButtonListener* p = static_cast<CGUINormalButtonListener*>(buttonListener.get());
    
    // Back button plane
    CPlane backPlane = m_vPlaneLoader.GetPlane(1); // back_off_?.bmp
    POINT backPos = m_vPlaneLoader.GetXY(1);
    backPlane->SetPos(backPos);
    
    p->SetPlane(smart_ptr<ISurface>(backPlane.get(), false));
    backBtn->SetEvent(buttonListener);
    backBtn->SetXY(backPos.x, backPos.y);
    m_backButton = backBtn;

    // Get card hover border from plane loader
    CPlane cardWakuPlane = m_vPlaneLoader.GetPlane(10); // card_waku_s.bmp from list_scene.txt

    // Create preview card surface
    m_cardPreviewImage.CreateSurface(200, 290, false);
    m_cardPreviewImage.SetFillColor(ISurface::makeRGB(0, 0, 0, 0));

    // Create page navigation arrows
    InitializePageControls();
}

void CSceneCardList::InitializePageControls() {
    // Left arrow button
    CGUIButton* leftBtn = new CGUIButton();
    leftBtn->SetMouse(smart_ptr<CFixMouse>(&m_mouse, false));
    smart_ptr<CGUIButtonEventListener> leftListener(new CGUINormalButtonListener());
    CGUINormalButtonListener* pl = static_cast<CGUINormalButtonListener*>(leftListener.get());
    
    // Left arrow planes
    CPlane leftNormalPlane = m_vPlaneLoader.GetPlane(6); // pe_yaji_l1.bmp
    POINT leftPos = m_vPlaneLoader.GetXY(6);
    leftNormalPlane->SetPos(leftPos);
    
    pl->SetPlane(smart_ptr<ISurface>(leftNormalPlane.get(), false));
    leftBtn->SetEvent(leftListener);
    leftBtn->SetXY(leftPos.x, leftPos.y);
    m_prevPageButton = leftBtn;

    // Right arrow button
    CGUIButton* rightBtn = new CGUIButton();
    rightBtn->SetMouse(smart_ptr<CFixMouse>(&m_mouse, false));
    smart_ptr<CGUIButtonEventListener> rightListener(new CGUINormalButtonListener());
    CGUINormalButtonListener* pr = static_cast<CGUINormalButtonListener*>(rightListener.get());
    
    // Right arrow planes
    CPlane rightNormalPlane = m_vPlaneLoader.GetPlane(8); // pe_yaji_r1.bmp
    POINT rightPos = m_vPlaneLoader.GetXY(8);
    rightNormalPlane->SetPos(rightPos);
    
    pr->SetPlane(smart_ptr<ISurface>(rightNormalPlane.get(), false));
    rightBtn->SetEvent(rightListener);
    rightBtn->SetXY(rightPos.x, rightPos.y);
    m_nextPageButton = rightBtn;
}

void CSceneCardList::LoadCardData() {
    std::ifstream cardList("data/card/list_card.txt");
    std::string line;
    int currentId = -1;
    std::string currentBmp;

    while (std::getline(cardList, line)) {
        if (line.empty() || line[0] == '\n') continue;
        
        if (line.substr(0, 2) == "//") {
            if (line.length() > 8) { // Check for ID line
                size_t idStart = line.find("[");
                size_t idEnd = line.find("]");
                if (idStart != std::string::npos && idEnd != std::string::npos) {
                    std::string idStr = line.substr(idStart + 1, idEnd - idStart - 1);
                    currentId = std::atoi(idStr.c_str());
                }
            }
        } else if (line.find(".bmp") != std::string::npos) {
            currentBmp = line;
            // Skip card_ura.bmp as it's the backing
            if (currentId != -1 && currentBmp != "card_ura.bmp") {
                CardInfo card;
                card.id = currentId;
                card.templateId = currentId;
                card.isNew = false;
                card.bmpName = currentBmp.substr(0, currentBmp.length() - 4); // Remove .bmp extension
                m_ownedCards.push_back(card);
                
                char debugStr[256];
                sprintf(debugStr, "Added card ID: %d, BMP: %s\n", currentId, card.bmpName.c_str());
                OutputDebugStringA(debugStr);
            }
            currentId = -1;
        }
    }

    m_nTotalPages = (m_ownedCards.size() + CARDS_PER_PAGE - 1) / CARDS_PER_PAGE;
    
    char debugStr[256];
    sprintf(debugStr, "Total cards loaded: %d\n", m_ownedCards.size());
    OutputDebugStringA(debugStr);
}

void CSceneCardList::OnMove(const smart_ptr<ISurface>& lp) {
    key.Input();
    m_mouse.Flush();

    if (m_timerMain.Get() < 500) return;

    // Handle back button
    if (key.IsKeyPush(5)) { // Space key
        GetSceneControl()->ReturnScene();
    }

    // Update buttons
    if (m_backButton && m_backButton->OnSimpleMove(lp.get())) {
        if (m_backButton->IsLClick()) {
            GetSceneControl()->ReturnScene();
        }
    }

    // Handle page navigation
    if (m_prevPageButton && m_prevPageButton->OnSimpleMove(lp.get())) {
        if (m_prevPageButton->IsLClick() && m_nCurrentPage > 1) {
            ChangePage(false);
        }
    }

    if (m_nextPageButton && m_nextPageButton->OnSimpleMove(lp.get())) {
        if (m_nextPageButton->IsLClick() && m_nCurrentPage < m_nTotalPages) {
            ChangePage(true);
        }
    }

    // Update card animations
    UpdateCardAnimations();
}

void CSceneCardList::UpdateCardAnimations() {
    for (int i = 0; i < CARDS_PER_PAGE; i++) {
        m_nCardAnimations[i].Inc();
        m_nCardScaleAnimations[i].Inc();
    }
}

void CSceneCardList::ChangePage(bool forward) {
    if (forward) {
        if (m_nCurrentPage < m_nTotalPages) {
            m_nCurrentPage++;
            // Reset animations for new page
            for (int i = 0; i < CARDS_PER_PAGE; i++) {
                m_nCardAnimations[i].Set(-CARD_HEIGHT, 0, 20);
                m_nCardScaleAnimations[i].Set(0, 100, 16);
            }
        }
    } else {
        if (m_nCurrentPage > 1) {
            m_nCurrentPage--;
            // Reset animations for new page
            for (int i = 0; i < CARDS_PER_PAGE; i++) {
                m_nCardAnimations[i].Set(-CARD_HEIGHT, 0, 20);
                m_nCardScaleAnimations[i].Set(0, 100, 16);
            }
        }
    }
}

void CSceneCardList::OnDraw(const smart_ptr<ISurface>& lp) {
    // Draw background
    if (m_bgPlane) {
        lp->BltFast(m_bgPlane.get(), m_bgPlane->GetPosX(), m_bgPlane->GetPosY());
    }

    // Draw title with fade effect
    if (m_timerMain.Get() > 0 && m_titlePlane) {
        lp->BlendBltFast(m_titlePlane.get(), m_titlePlane->GetPosX(), m_titlePlane->GetPosY(), m_nFade);
    }

    if (m_timerMain.Get() > 500) {
        // Draw page navigation buttons
        if (m_prevPageButton) {
            m_prevPageButton->OnSimpleDraw(lp.get());
        }
        if (m_nextPageButton) {
            m_nextPageButton->OnSimpleDraw(lp.get());
        }

        // Draw back button
        if (m_backButton) {
            m_backButton->OnSimpleDraw(lp.get());
        }

        // Draw card grid
        DrawCardGrid(lp);

        // Draw card preview
        DrawCardPreview(lp);

        // Draw pagination
        DrawPagination(lp);

        // Draw collection rate
        DrawCollectionRate(lp);
    }

    m_nFade.Inc();
}

void CSceneCardList::DrawCardGrid(const smart_ptr<ISurface>& lp) {
    const int GRID_START_X = 230;
    const int GRID_START_Y = 108;
    const int CARD_SPACING_X = 5;
    const int CARD_SPACING_Y = 15;

    int startIndex = (m_nCurrentPage - 1) * CARDS_PER_PAGE;
    
    for (int row = 0; row < CARD_ROWS; row++) {
        for (int col = 0; col < CARD_COLUMNS; col++) {
            int index = startIndex + (row * CARD_COLUMNS) + col;
            int x = GRID_START_X + (col * (CARD_WIDTH + CARD_SPACING_X));
            int y = GRID_START_Y + (row * (CARD_HEIGHT + CARD_SPACING_Y));

            // Apply card animation offset - now moves from above into position
            int animIndex = (row * CARD_COLUMNS) + col;
            y += (int)m_nCardAnimations[animIndex];

            // Draw card if we have it
            if (index >= 0 && (size_t)index < m_ownedCards.size()) {
                const CardInfo& card = m_ownedCards[index];
                
                // Create temporary plane for card image
                CFastPlane cardPlane;
                char filepath[256];
                sprintf(filepath, "data/mini/%s.bmp", card.bmpName.c_str());
                
                if (cardPlane.Load(filepath) == 0) {
                    // Get current scale (0-100%)
                    int scalePercent = (int)m_nCardScaleAnimations[animIndex];
                    
                    // Calculate scaled width
                    int scaledWidth = (CARD_WIDTH * scalePercent) / 100;
                    if (scaledWidth < 1) scaledWidth = 1; // Ensure minimum width of 1 pixel
                    
                    // Center the scaled card
                    int xOffset = (CARD_WIDTH - scaledWidth) / 2;
                    
                    // Create temporary surface for scaled card
                    CFastPlane scaledCard;
                    scaledCard.CreateSurface(scaledWidth, CARD_HEIGHT, false);
                    
                    // Scale the card horizontally
                    cardPlane.SetSize(scaledWidth, CARD_HEIGHT);
                    scaledCard.BltFast(&cardPlane, 0, 0);
                    
                    // Draw the scaled card
                    lp->BltFast(&scaledCard, x + xOffset, y);

                    // Draw NEW indicator if card is new
                    if (card.isNew) {
                        CPlane newIndicator = m_vPlaneLoader.GetPlane(11);
                        if (newIndicator) {
                            lp->BltFast(newIndicator.get(), x + 5, y + 26);
                        }
                    }
                }
            }
        }
    }
}

void CSceneCardList::DrawCardPreview(const smart_ptr<ISurface>& lp) {
    // Preview area starts at position from list_detail.txt
    const int PREVIEW_X = 11;
    const int PREVIEW_Y = 381;

    if (m_nPreviewCardId != 0) {
        // Draw card preview
        lp->BltFast(&m_cardPreviewImage, PREVIEW_X, PREVIEW_Y);

        // Draw detail line
        CPlane detailLine = m_vDetailPlaneLoader.GetPlane(3); // pap_line.bmp
        if (detailLine) {
            lp->BltFast(detailLine.get(), detailLine->GetPosX(), detailLine->GetPosY());
        }
    }
}

void CSceneCardList::DrawPagination(const smart_ptr<ISurface>& lp) {
    // Load page number font
    CPlane pageFont = m_vPlaneLoader.GetPlane(4); // page_font.bmp
    if (!pageFont) return;

    // Draw current page number (positions from list_scene.txt)
    char pageNum[8];
    sprintf(pageNum, "%d", m_nCurrentPage);
    int numX = 480;  // From list_scene.txt
    int numY = 561;
    
    for (char* p = pageNum; *p; p++) {
        int digit = *p - '0';
        SIZE srcSize = { digit * 21, 0 }; // Source position
        lp->BltFast(pageFont.get(), numX, numY, &srcSize);
        numX += 21;
    }

    // Draw total pages
    sprintf(pageNum, "%d", m_nTotalPages);
    numX = 527;  // From list_scene.txt
    
    for (char* p = pageNum; *p; p++) {
        int digit = *p - '0';
        SIZE srcSize = { digit * 21, 0 }; // Source position
        lp->BltFast(pageFont.get(), numX, numY, &srcSize);
        numX += 21;
    }
}

void CSceneCardList::DrawCollectionRate(const smart_ptr<ISurface>& lp) {
    // Get font and symbol planes
    CPlane rateFont = m_vPlaneLoader.GetPlane(12);    // get_font.bmp
    CPlane rateSymbol = m_vPlaneLoader.GetPlane(13);  // get_symbol.bmp
    
    if (!rateFont || !rateSymbol) return;

    // Calculate collection rate
    float rate = (float)m_ownedCards.size() / m_nTotalPages * 100.0f;
    char rateStr[16];
    sprintf(rateStr, "%.1f", rate);

    // Draw at position from list_scene.txt (761,080)
    int numX = 761;
    int numY = 80;

    // Draw digits
    for (char* p = rateStr; *p; p++) {
        if (*p == '.') continue;  // Skip decimal point for now
        
        int digit = *p - '0';
        SIZE srcSize = { digit * 10, 0 }; // Source position
        lp->BltFast(rateFont.get(), numX, numY, &srcSize);
        numX += 10;
    }

    // Draw % symbol
    lp->BltFast(rateSymbol.get(), numX, numY);
}