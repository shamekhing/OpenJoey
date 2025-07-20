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
    nFadeBG.Set(0, 255, 16);
    m_timerMain = CTimer();
    m_timerMain.Restart();
    m_timerMain.Reset();

    // Clone fb from previous scene if needed
    smart_ptr<ISurface> screenPtr = app->GetDraw()->GetSecondary()->cloneFull();
    if (screenPtr.get()) {
        m_background = screenPtr;
        // OutputDebugStringA("Framebuffer data cloned and loaded successfully!\n"); // Commented out for performance
    }

    // Initialize column animations
    for (int col = 0; col < CARD_COLUMNS; col++) {
        m_nCardAnimations[col].Set(-CARD_HEIGHT/2, 0, ANIM_SPEED);      // Start at middle of cell
        m_nCardScaleAnimations[col].Set(0, 100, ANIM_SPEED-2);          // Scale animation slightly faster
        m_bColumnAnimationStarted[col] = false;
    }

    // Start rightmost two columns instead of leftmost
    m_bColumnAnimationStarted[CARD_COLUMNS - 1] = true;  // Start rightmost column
    m_bColumnAnimationStarted[CARD_COLUMNS - 2] = true;  // Start second rightmost column

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

    InitializeUI();
    LoadCardData();
    LoadCardTexturesForCurrentPage(); // Load initial page textures
}

void CSceneCardList::SetHoverButtonPlane(CGUIButton* btn, int id, bool negativeOrder = false){
    // Reverting to original logic as GetPlaneNumber is not public.
    CGUIButtonEventListener* e = btn->GetEvent().get();
    CGUINormalButtonListener* p    = (CGUINormalButtonListener*)e;
    if (btn->IsIn())
        if(negativeOrder)
            p->SetPlaneNumber(id-1);
        else
            p->SetPlaneNumber(id+1);
    else
        p->SetPlaneNumber(id);
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
    if (m_backButton) {
        m_backButton->OnSimpleMove(lp.get());
        SetHoverButtonPlane(m_backButton, 1);
        if (m_backButton->IsLClick()) {
            GetSceneControl()->ReturnScene();
        }
    }

    // Handle page navigation
    if (m_prevPageButton) {
        m_prevPageButton->OnSimpleMove(lp.get());
        SetHoverButtonPlane(m_prevPageButton, 6, true);

        if (m_prevPageButton->IsLClick() && m_nCurrentPage > 1) {
            ChangePage(false);
        }
    }

    if (m_nextPageButton) {
        m_nextPageButton->OnSimpleMove(lp.get());
        SetHoverButtonPlane(m_nextPageButton, 8, true);
        if (m_nextPageButton->IsLClick() && m_nCurrentPage < m_nTotalPages) {
            ChangePage(true);
        }
    }

    // Update card animations
    UpdateCardAnimations();
}

void CSceneCardList::OnDraw(const smart_ptr<ISurface>& lp) {
    // Draw background
    if (m_bgPlane) {
        ISurfaceTransBlt::CircleBlt5(lp.get(), m_bgPlane.get(), 0, 0, (int)nFadeBG, 0, 255);
        nFadeBG.Inc();
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
    p->SetPlaneLoader(smart_ptr<CPlaneLoader>(&m_vPlaneLoader, false), 1);
    
    POINT backPos = m_vPlaneLoader.GetXY(1);
    backBtn->SetEvent(buttonListener);
    backBtn->SetXY(backPos.x, backPos.y);
    m_backButton = backBtn;

    // Get card hover border from plane loader
    m_cardHoverBorder = m_vPlaneLoader.GetPlane(9); // card_waku_s.bmp from list_scene.txt

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
    pl->SetPlaneLoader(smart_ptr<CPlaneLoader>(&m_vPlaneLoader, false), 6); // pe_yaji_l1.bmp, pe_yaji_l2.bmp etc.

    POINT leftPos = m_vPlaneLoader.GetXY(6);
    leftBtn->SetEvent(leftListener);
    leftBtn->SetXY(leftPos.x, leftPos.y);
    m_prevPageButton = leftBtn;

    // Right arrow button
    CGUIButton* rightBtn = new CGUIButton();
    rightBtn->SetMouse(smart_ptr<CFixMouse>(&m_mouse, false));
    smart_ptr<CGUIButtonEventListener> rightListener(new CGUINormalButtonListener());
    CGUINormalButtonListener* pr = static_cast<CGUINormalButtonListener*>(rightListener.get());
    pr->SetPlaneLoader(smart_ptr<CPlaneLoader>(&m_vPlaneLoader, false), 8); // pe_yaji_r1.bmp, pe_yaji_r2.bmp etc.

    POINT rightPos = m_vPlaneLoader.GetXY(8);
    rightBtn->SetEvent(rightListener);
    rightBtn->SetXY(rightPos.x, rightPos.y);
    m_nextPageButton = rightBtn;
}

void CSceneCardList::LoadCardData() {
    std::ifstream cardList("data/card/list_card.txt");
    if (!cardList.is_open()) {
        OutputDebugStringA("Error: Failed to open data/card/list_card.txt\n");
        return;
    }

    std::string line;
    while (std::getline(cardList, line)) {
        if (line.empty()) continue;
        if (!line.empty() && line[line.length() - 1] == '\r') {
            line.erase(line.length() - 1);
        }
        if (line.empty()) continue;

        int currentId = -1; // Reset for each potential card entry
        std::string currentBmp;

        // Check if it's an ID line (starts with // and contains [ID])
        if (line.substr(0, 2) == "//") {
            size_t idStart = line.find("[");
            size_t idEnd = line.find("]");
            if (idStart != std::string::npos && idEnd != std::string::npos && idEnd > idStart) {
                std::string idStr = line.substr(idStart + 1, idEnd - idStart - 1);
                currentId = std::atoi(idStr.c_str());
                // The next non-comment line should be the BMP
                std::string nextLine;
                if (std::getline(cardList, nextLine)) {
                    if (!nextLine.empty() && nextLine[nextLine.length() - 1] == '\r') {
                        nextLine.erase(nextLine.length() - 1);
                    }
                    if (nextLine.find(".bmp") != std::string::npos) {
                        currentBmp = nextLine;
                        if (currentId != -1 && currentBmp != "card_ura.bmp") {
                            CardInfo card;
                            card.id = currentId;
                            card.templateId = currentId;
                            card.isNew = false;
                            card.bmpName = currentBmp.substr(0, currentBmp.length() - 4); // Remove .bmp extension
                            m_ownedCards.push_back(card);
                        }
                    }
                }
            }
        }
    }
    cardList.close();

    m_nTotalPages = (m_ownedCards.size() + CARDS_PER_PAGE - 1) / CARDS_PER_PAGE;
}

void CSceneCardList::ClearCardTextures() {
    m_cardTextures.clear();
}

void CSceneCardList::LoadCardTexturesForCurrentPage() {
    ClearCardTextures(); // Clear previously loaded textures

    int startIndex = (m_nCurrentPage - 1) * CARDS_PER_PAGE;
    int endIndex = startIndex + CARDS_PER_PAGE;
    if ((size_t)endIndex > m_ownedCards.size()) {
        endIndex = m_ownedCards.size();
    }

    char filepath[256];
    for (int i = startIndex; i < endIndex; ++i) {
        const CardInfo& card = m_ownedCards[i];
        
        // FIX: Store smart_ptr<CFastPlane> directly.
        // Allocate CFastPlane on heap, smart_ptr will manage its lifetime.
        smart_ptr<CFastPlane> cardPlane = smart_ptr<CFastPlane>(new CFastPlane());
        sprintf(filepath, "data/mini/%s.bmp", card.bmpName.c_str());
        
        if (cardPlane->Load(filepath) == 0) {
            m_cardTextures[card.id] = cardPlane; // Assignment is now type-compatible
        } else {
            // Clean up the allocated CFastPlane if loading failed.
            // smart_ptr would normally delete, but if it's not inserted into map, it won't be owned.
            // If smart_ptr always takes ownership even if not assigned (depends on its constructor),
            // then `delete cardPlane.get()` is wrong.
            // Let's assume the smart_ptr constructor takes ownership when a raw pointer is passed.
            // If `cardPlane` goes out of scope and wasn't added to the map, it will be deleted.
            // So no explicit `delete` is needed here for `cardPlane`.
            
            char debugStr[256];
            sprintf(debugStr, "Warning: Failed to load card texture: %s\n", filepath);
            OutputDebugStringA(debugStr);
        }
    }
}


void CSceneCardList::UpdateCardAnimations() {
    // Update started columns
    for (int col = CARD_COLUMNS - 1; col >= 0; col--) {
        if (m_bColumnAnimationStarted[col]) {
            m_nCardAnimations[col].Inc();
            m_nCardScaleAnimations[col].Inc();
        }
    }

    // Check if we should start new columns (going right to left)
    for (int col = CARD_COLUMNS - 3; col >= 0; col--) {
        if (!m_bColumnAnimationStarted[col]) {
            // Start new column if next column is mostly done (80% complete)
            if (m_bColumnAnimationStarted[col + 2] &&
                m_nCardAnimations[col + 2].GetFrameNow() >= (int)(ANIM_SPEED * 0.8)) {
                m_bColumnAnimationStarted[col] = true;
            }
        }
    }
}

void CSceneCardList::ChangePage(bool forward) {
    int oldPage = m_nCurrentPage;
    if (forward) {
        if (m_nCurrentPage < m_nTotalPages) {
            m_nCurrentPage++;
        }
    } else {
        if (m_nCurrentPage > 1) {
            m_nCurrentPage--;
        }
    }

    if (m_nCurrentPage != oldPage) {
        // Reset animations for new page
        for (int col = 0; col < CARD_COLUMNS; col++) {
            m_nCardAnimations[col].Set(-CARD_HEIGHT/2, 0, ANIM_SPEED);
            m_nCardScaleAnimations[col].Set(0, 100, ANIM_SPEED-2);
            m_bColumnAnimationStarted[col] = (col >= CARD_COLUMNS - 2); // Start rightmost two columns
        }
        LoadCardTexturesForCurrentPage(); // Load new textures for the changed page
    }
}

void CSceneCardList::DrawCardGrid(const smart_ptr<ISurface>& lp) {
    const int GRID_START_X = 230;
    const int GRID_START_Y = 108;
    const int CARD_SPACING_X = 5;
    const int CARD_SPACING_Y = 15;

    int startIndex = (m_nCurrentPage - 1) * CARDS_PER_PAGE;
    
    // Loop through each card position from right to left, top to bottom
    for (int col = CARD_COLUMNS - 1; col >= 0; col--) {
        // Skip if column animation hasn't started yet
        if (!m_bColumnAnimationStarted[col]) continue;

        // Pre-calculate column-specific animation values once per column
        int currentYOffset = (int)m_nCardAnimations[col];
        int scalePercent = (int)m_nCardScaleAnimations[col];
        int scaledWidth = (CARD_WIDTH * scalePercent) / 100;
        if (scaledWidth < 1) scaledWidth = 1; // Ensure width is at least 1 pixel
        int xOffset = (CARD_WIDTH - scaledWidth) / 2;

        // For each column, go through rows top to bottom
        for (int row = 0; row < CARD_ROWS; row++) {
            // Calculate position on screen
            int x = GRID_START_X + (col * (CARD_WIDTH + CARD_SPACING_X));
            int y = GRID_START_Y + (row * (CARD_HEIGHT + CARD_SPACING_Y)) + currentYOffset;

            // Calculate the index in the collection
            int collectionIndex = startIndex + (row * CARD_COLUMNS) + col;

            // Draw card if we have it
            if (collectionIndex >= 0 && (size_t)collectionIndex < m_ownedCards.size()) {
                const CardInfo& card = m_ownedCards[collectionIndex];
                
                // Get the card texture from the cache - now smart_ptr<CFastPlane>
                std::map<int, smart_ptr<CFastPlane> >::iterator it = m_cardTextures.find(card.id);
                if (it != m_cardTextures.end() && it->second.get()) {
                    smart_ptr<CFastPlane> cachedCardPlane = it->second;

                    // Source rectangle (entire original card)
                    RECT srcRect = {0, 0, CARD_WIDTH, CARD_HEIGHT}; 

                    // Destination size (scaled dimensions)
                    // This is the `SIZE` struct that BltFast expects for `pDstSize`
                    SIZE dstSize = {scaledWidth, CARD_HEIGHT};

                    // Blit the card to the screen, applying scaling
                    // The `lp` surface (screen) is the destination for BltFast.
                    // Parameters: pSrc, x, y, pDstSize, pSrcRect, pDstClip, nBasePoint
                    lp->BltFast(
                        cachedCardPlane.get(), // Source surface
                        x + xOffset, y,        // Destination coordinates
                        &dstSize,              // Pointer to destination size (for scaling)
                        &srcRect,              // Pointer to source rectangle
                        NULL,                  // No destination clip
                        0                      // No base point
                    );
                
                    if (card.isNew) {
                        CPlane newIndicator = m_vPlaneLoader.GetPlane(10);
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
    CPlane pageFont = m_vPlaneLoader.GetPlane(3); // page_font.bmp

    if (!pageFont) return;

    int pOffsetX = 8;
    int pOffsetY = 9;

    // Draw current page number (positions from list_scene.txt)
    char pageNum[8];
    sprintf(pageNum, "%d", m_nCurrentPage);
    int numX = 480;  // From list_scene.txt
    int numY = 561;
    
    for (char* p = pageNum; *p; p++) {
        int digit = *p - '0';
        RECT srcSize = { digit * 15, 0, digit * 15 + 15, 19 }; // Font character width is 15px, height 19px
        lp->BltNatural(pageFont.get(), numX - pOffsetX, numY - pOffsetY, 0, &srcSize);
        numX += 21; // Advance X for next digit
    }

    // Draw total pages
    sprintf(pageNum, "%d", m_nTotalPages);
    numX = 527;  // From list_scene.txt
    
    for (char* p = pageNum; *p; p++) {
        int digit = *p - '0';
        RECT srcSize = { digit * 15, 0, digit * 15 + 15, 19 }; // Font character width is 15px, height 19px
        lp->BltNatural(pageFont.get(), numX - pOffsetX, numY - pOffsetY, 0, &srcSize);
        numX += 21; // Advance X for next digit
    }
}

// TODO: This needs further code. Switch all few seconds from Percentage% to OwnedCards/AllCards label
void CSceneCardList::DrawCollectionRate(const smart_ptr<ISurface>& lp) {
    // Get font and symbol planes
    CPlane rateFont = m_vPlaneLoader.GetPlane(11);    // get_font.bmp
    CPlane rateSymbol = m_vPlaneLoader.GetPlane(12);  // get_symbol.bmp
    
    if (!rateFont || !rateSymbol) return;

    float rate = 0.0f;
    if (m_nTotalPages > 0) {
        rate = (float)m_ownedCards.size() / (m_nTotalPages * CARDS_PER_PAGE) * 100.0f;
    }
    
    char rateStr[16];
    sprintf(rateStr, "%.1f", rate);

    // Draw at position from list_scene.txt (761,080)
    int numX = 670;
    int numY = 80;

    // Draw digits
    for (char* p = rateStr; *p; p++) {
        if (*p == '.') {
            RECT srcSize = { 11, 0, 17, 18 };
            lp->BltNatural(rateSymbol.get(), numX, numY + 12, 0, &srcSize);
            numX += 6;
            continue;
        }
        
        int digit = *p - '0';
        RECT srcSize = { digit * 12, 0, digit * 12 + 12, 15 };
        lp->BltNatural(rateFont.get(), numX, numY, 0, &srcSize);
        numX += 10;
    }

    // Draw % symbol
    RECT rectPercent = { 15, 0, 36, 18 };
    lp->BltNatural(rateSymbol.get(), numX + 2, numY - 1, 0, &rectPercent);
}