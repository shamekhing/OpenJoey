#include "../../stdafx.h"
#include "yaneGUITextBox.h"

// Using standard min/max with explicit qualification.
// If ambiguity or definition issues persist in MSVC 2003,
// switch to ternary operator: (a < b ? a : b)
// Or ensure NOMINMAX is defined project-wide before including <windows.h>
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif


namespace yaneuraoGameSDK3rd {
namespace Draw {

///////////////////////////////////////////////////////////////////////////////
// CGUITextBoxArrowButtonListener Implementation
///////////////////////////////////////////////////////////////////////////////

CGUITextBoxArrowButtonListener::CGUITextBoxArrowButtonListener(ScrollDirection direction)
    : m_direction(direction) {}

void CGUITextBoxArrowButtonListener::SetTextBox(YTL::smart_ptr<CGUITextBox> textBox) {
    m_vTextBox = textBox;
}

void CGUITextBoxArrowButtonListener::OnLBClick(void) {
    if (m_vTextBox.get() && m_vTextBox->GetFont()) {
        int lineHeight = m_vTextBox->GetFont()->GetHeight();
        if (m_direction == SCROLL_UP) {
            m_vTextBox->ScrollContent(0, -lineHeight);
        } else {
            m_vTextBox->ScrollContent(0, lineHeight);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// CGUITextBoxSliderListener Implementation
///////////////////////////////////////////////////////////////////////////////

CGUITextBoxSliderListener::CGUITextBoxSliderListener() {
    // Default min slider size, can be overridden if needed
    SetMinSize(5, 5);
}

void CGUITextBoxSliderListener::OnPageUp() {
    if (m_vTextBox.get()) {
        if (m_vTextBox->GetSlider()->GetButton() == 2) {
            m_vTextBox->ScrollContent(0, -m_nMinY);
        }
    }
}

void CGUITextBoxSliderListener::OnPageDown() {
    if (m_vTextBox.get()) {
        if (m_vTextBox->GetSlider()->GetButton() == 2) {
            m_vTextBox->ScrollContent(0, m_nMinY);
        }
    }
}

void CGUITextBoxSliderListener::OnPageLeft() {
    if (m_vTextBox.get()) {
        m_vTextBox->ScrollContent(-m_nMinX, 0);
    }
}

void CGUITextBoxSliderListener::OnPageRight() {
    if (m_vTextBox.get()) {
        m_vTextBox->ScrollContent(m_nMinX, 0);
    }
}

///////////////////////////////////////////////////////////////////////////////
// CGUITextBox Implementation
///////////////////////////////////////////////////////////////////////////////

CGUITextBox::CGUITextBox()
    : m_nWidth(0), m_nHeight(0), m_nContentWidth(0), m_nContentHeight(0),
      m_nScrollX(0), m_nScrollY(0), m_sliderMode(NO_SLIDER),
      m_nTextOffsetX(0), m_nTextOffsetY(0),
      m_nSliderStripWidth(15),
      m_nSliderStripHeight(15),
      m_nMarginX(0),
      m_nMarginY(0),
      m_nTitleHeight(0),
      m_nTitleTypeHeight(0), // **NEW LABEL**: Initialize
      m_nFooterHeight(0)
{
    m_textColor = ISurface::makeRGB(255, 255, 255, 0); // White, opaque
    ::SetRect(&m_rcTextBoxClip, 0, 0, 0, 0);
}

CGUITextBox::~CGUITextBox() {
    // Smart pointers handle deallocation
    // Raw pointers CTextFastPlane* are likely managed by smart_ptr CPlane.
    // Ensure proper deletion if CTextFastPlane* is not implicitly handled by CPlane's destructor.
    // Given the YTL::smart_ptr, it's generally safe to assume they manage their objects.
}

// --- Margin Methods Implementation ---
void CGUITextBox::SetMargins(int x, int y) {
    m_nMarginX = max(0, x);
    m_nMarginY = max(0, y);
    UpdateTextPlane(); // Recalculate and redraw text with new margins
}

void CGUITextBox::GetMargins(int& x, int& y) const {
    x = m_nMarginX;
    y = m_nMarginY;
}
// --- End Margin Methods ---

// --- Title, TitleType and Footer Text Implementation ---
void CGUITextBox::SetTextTitle(const string& title) {
    if (m_strTitleText != title) {
        m_strTitleText = title;
        if (m_vTitleFastPlane == NULL) {
            m_vTitleFastPlane = new CTextFastPlane;
            m_vTitlePlane = CPlane(m_vTitleFastPlane);
            if (m_vTextFastPlane != NULL) {
                *(m_vTitleFastPlane->GetFont()) = *(m_vTextFastPlane->GetFont());
            }
        }
        // Determine available width for wrapping (same logic as in UpdateTextPlane)
        int textAvailableWidth = m_nWidth - (m_nMarginX * 2);
        if (m_sliderMode == VERTICAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
            textAvailableWidth -= m_nSliderStripWidth;
        }
        if (textAvailableWidth < 0) textAvailableWidth = 0;

        std::string wrappedTitle = WrapText(m_strTitleText, textAvailableWidth);

        m_vTitleFastPlane->GetFont()->SetText(wrappedTitle);
        m_vTitleFastPlane->GetFont()->SetColor(m_textColor);
        m_vTitleFastPlane->UpdateText();

        int tempW, tempH;
        m_vTitleFastPlane->GetFont()->GetSize(tempW, tempH);
        m_nTitleHeight = tempH; // Update cached title height
        UpdateTextPlane(); // Recalculate overall content size and slider range
    }
}

string CGUITextBox::GetTextTitle() const {
    return m_strTitleText;
}

// **NEW LABEL METHOD**: SetTextTitleType Implementation
void CGUITextBox::SetTextTitleType(const string& titleType) {
    if (m_strTitleTypeText != titleType) {
        m_strTitleTypeText = titleType;
        if (m_vTitleTypeFastPlane == NULL) {
            m_vTitleTypeFastPlane = new CTextFastPlane;
            m_vTitleTypePlane = CPlane(m_vTitleTypeFastPlane);
            if (m_vTextFastPlane != NULL) {
                *(m_vTitleTypeFastPlane->GetFont()) = *(m_vTextFastPlane->GetFont());
            }
        }
        // Determine available width for wrapping
        int textAvailableWidth = m_nWidth - (m_nMarginX * 2);
        if (m_sliderMode == VERTICAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
            textAvailableWidth -= m_nSliderStripWidth;
        }
        if (textAvailableWidth < 0) textAvailableWidth = 0;

        std::string wrappedTitleType = WrapText(m_strTitleTypeText, textAvailableWidth);

        m_vTitleTypeFastPlane->GetFont()->SetText(wrappedTitleType);
        m_vTitleTypeFastPlane->GetFont()->SetColor(m_textColor);
        m_vTitleTypeFastPlane->UpdateText();

        int tempW, tempH;
        m_vTitleTypeFastPlane->GetFont()->GetSize(tempW, tempH);
        m_nTitleTypeHeight = tempH; // Update cached title type height
        UpdateTextPlane(); // Recalculate overall content size and slider range
    }
}

string CGUITextBox::GetTextTitleType() const {
    return m_strTitleTypeText;
}


void CGUITextBox::SetTextFooter(const string& footer) {
    if (m_strFooterText != footer) {
        m_strFooterText = footer;
        if (m_vFooterFastPlane == NULL) {
            m_vFooterFastPlane = new CTextFastPlane;
            m_vFooterPlane = CPlane(m_vFooterFastPlane);
            if (m_vTextFastPlane != NULL) {
                *(m_vFooterFastPlane->GetFont()) = *(m_vTextFastPlane->GetFont());
            }
        }
        if (!m_strFooterText.empty()) {
            // Determine available width for wrapping
            int textAvailableWidth = m_nWidth - (m_nMarginX * 2);
            if (m_sliderMode == VERTICAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
                textAvailableWidth -= m_nSliderStripWidth;
            }
            if (textAvailableWidth < 0) textAvailableWidth = 0;

            std::string wrappedFooter = WrapText(m_strFooterText, textAvailableWidth);

            m_vFooterFastPlane->GetFont()->SetText(wrappedFooter);
            m_vFooterFastPlane->GetFont()->SetColor(m_textColor);
            m_vFooterFastPlane->UpdateText();
            int tempW, tempH; // Declare temporary ints for GetSize parameters
            m_vFooterFastPlane->GetFont()->GetSize(tempW, tempH);
            m_nFooterHeight = tempH; // Update cached footer height
        } else {
            m_nFooterHeight = 0; // Hide footer if text is empty
        }
        UpdateTextPlane(); // Recalculate overall content size and slider range
    }
}

string CGUITextBox::GetTextFooter() const {
    return m_strFooterText;
}
// --- END NEW ---

void CGUITextBox::SetSliderGFX(smart_ptr<ISurface> sliderThumbGraphic){
    m_vSliderThumbGraphic = sliderThumbGraphic;

    if (m_vSlider.get() && m_vSliderListener.get() && m_vSliderThumbGraphic.get()) {
        // Corrected: Call SetMinSizeFromGraphic directly on m_vSliderListener (which is a CGUITextBoxSliderListener)
        m_vSliderListener->SetMinSizeFromGraphic(m_vSliderThumbGraphic.get());

        // Now, cast listener to its specific type to access SetPlane
        CGUINormalSliderListener* pSliderListener = static_cast<CGUINormalSliderListener*>(m_vSliderListener.get());
        pSliderListener->SetPlane(m_vSliderThumbGraphic);

        int actualThumbWidth = 0;
        int actualThumbHeight = 0;

        m_vSliderThumbGraphic->GetSize(actualThumbWidth, actualThumbHeight);

        m_nSliderStripWidth = actualThumbWidth;
        m_nSliderStripHeight = actualThumbHeight;

        RECT sliderMovementRect;
        int effectiveWidth = m_nWidth - (m_nMarginX * 2);
        int effectiveHeight = m_nHeight - (m_nMarginY * 2);

        ::SetRect(&sliderMovementRect, 0, 0, effectiveWidth, effectiveHeight);

        if (m_sliderMode == VERTICAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
            sliderMovementRect.left = effectiveWidth - m_nSliderStripWidth;
            sliderMovementRect.right = effectiveWidth;

            int buttonSize = m_nSliderStripWidth; // Assuming up/down arrows are width of slider strip
            sliderMovementRect.top = buttonSize;
            sliderMovementRect.bottom = effectiveHeight - buttonSize;

            if (m_sliderMode == BOTH_SLIDERS) {
                sliderMovementRect.bottom -= m_nSliderStripHeight;
            }
        }
        if (m_sliderMode == HORIZONTAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
            sliderMovementRect.top = effectiveHeight - m_nSliderStripHeight;
            sliderMovementRect.bottom = effectiveHeight;

            if (m_sliderMode == BOTH_SLIDERS) {
                sliderMovementRect.right -= m_nSliderStripWidth;
            }
        }
        m_vSlider->SetRect(&sliderMovementRect);
    }
}

void CGUITextBox::SetArrowGFX(smart_ptr<CPlaneLoader> pv, int upIndex, int downIndex) {
    if (m_vScrollUpButton.get() && m_vScrollDownButton.get() && pv.get()) {
        YTL::smart_ptr<CGUITextBox> this_ptr(this, false);

        if (m_vScrollUpButtonListener.isNull()) {
            m_vScrollUpButtonListener = YTL::smart_ptr<CGUITextBoxArrowButtonListener>(new CGUITextBoxArrowButtonListener(SCROLL_UP));
            m_vScrollUpButtonListener->SetTextBox(this_ptr);
            m_vScrollUpButton->SetEvent(YTL::smart_ptr_static_cast<CGUIButtonEventListener>(m_vScrollUpButtonListener));
        }

        if (m_vScrollDownButtonListener.isNull()) {
            m_vScrollDownButtonListener = YTL::smart_ptr<CGUITextBoxArrowButtonListener>(new CGUITextBoxArrowButtonListener(SCROLL_DOWN));
            m_vScrollDownButtonListener->SetTextBox(this_ptr);
            m_vScrollDownButton->SetEvent(YTL::smart_ptr_static_cast<CGUIButtonEventListener>(m_vScrollDownButtonListener));
        }

        // Corrected: Cast to CGUINormalButtonListener, not CGUINormalSliderListener
        CGUINormalButtonListener* upListener = static_cast<CGUINormalButtonListener*>(m_vScrollUpButtonListener.get());
        CGUINormalButtonListener* downListener = static_cast<CGUINormalButtonListener*>(m_vScrollDownButtonListener.get());

        pv->SetColorKey(ISurface::makeRGB(255, 255, 255, 0));
        upListener->SetPlaneLoader(pv, upIndex);
        downListener->SetPlaneLoader(pv, downIndex);

        int buttonWidth = 0;
        int buttonHeight = 0;
        CPlane upPlane = pv->GetPlane(upIndex);
        upPlane->GetSize(buttonWidth, buttonHeight);
        int arrowButtonHeight = buttonHeight;

        m_vScrollUpButton->SetXY(m_nX + m_nWidth - m_nSliderStripWidth - m_nMarginX, m_nY + m_nMarginY);
        m_vScrollDownButton->SetXY(m_nX + m_nWidth - m_nSliderStripWidth - m_nMarginX, m_nY + m_nHeight - arrowButtonHeight - m_nMarginY);
    }
}

void CGUITextBox::SetSliderLoader(string folder, string path){
    CGUINormalSliderListener* pSliderListener = static_cast<CGUINormalSliderListener*>(m_vSliderListener.get());

    CPlaneLoader m_vPlaneScrollLoader;
    m_vPlaneScrollLoader.SetReadDir(folder);
    if (m_vPlaneScrollLoader.Set(path, false) != 0) {
        OutputDebugStringA("Error: Failed to load\n");
    }

    CPlane pln = m_vPlaneScrollLoader.GetPlane(5);
    smart_ptr<ISurface> plnPtr(pln.get(), false);
    m_vSliderThumbGraphic = plnPtr;
    pSliderListener->SetPlane(m_vSliderThumbGraphic);
}


void CGUITextBox::Create(int x, int y, int width, int height, SliderMode mode) {
    SetXY(x, y);

    m_nWidth = width;
    m_nHeight = height;
    m_sliderMode = mode;

    ::SetRect(&m_rcTextBoxClip, x + m_nMarginX, y + m_nMarginY,
                                 x + width - m_nMarginX, y + height - m_nMarginY);

    m_vTextFastPlane = new CTextFastPlane;
    m_vTextPlane = CPlane(m_vTextFastPlane);

    m_vTitleFastPlane = new CTextFastPlane;
    m_vTitlePlane = CPlane(m_vTitleFastPlane);

    m_vTitleTypeFastPlane = new CTextFastPlane; // **NEW LABEL**: Initialize
    m_vTitleTypePlane = CPlane(m_vTitleTypeFastPlane); // **NEW LABEL**

    m_vFooterFastPlane = new CTextFastPlane;
    m_vFooterPlane = CPlane(m_vFooterFastPlane);

    if (m_vTextFastPlane != NULL) {
        m_vTextFastPlane->GetFont()->SetColor(m_textColor);
        m_vTextFastPlane->GetFont()->SetSize(16);
    }
    if (m_vTitleFastPlane != NULL && m_vTextFastPlane != NULL) {
        *(m_vTitleFastPlane->GetFont()) = *(m_vTextFastPlane->GetFont());
        m_vTitleFastPlane->GetFont()->SetColor(m_textColor);
    }
    // **NEW LABEL**: Apply font from main text plane to title type plane
    if (m_vTitleTypeFastPlane != NULL && m_vTextFastPlane != NULL) {
        *(m_vTitleTypeFastPlane->GetFont()) = *(m_vTextFastPlane->GetFont());
        m_vTitleTypeFastPlane->GetFont()->SetColor(m_textColor);
    }
    if (m_vFooterFastPlane != NULL && m_vTextFastPlane != NULL) {
        *(m_vFooterFastPlane->GetFont()) = *(m_vTextFastPlane->GetFont());
        m_vFooterFastPlane->GetFont()->SetColor(m_textColor);
    }

    if (m_sliderMode != NO_SLIDER) {
        m_vSlider = YTL::smart_ptr<CGUISlider>(new CGUISlider());
        m_vSliderListener = YTL::smart_ptr<CGUITextBoxSliderListener>(new CGUITextBoxSliderListener());

        YTL::smart_ptr<CGUITextBox> this_ptr(this, false);
        m_vSliderListener->SetTextBox(this_ptr);

        m_vSlider->SetEvent(YTL::smart_ptr_static_cast<CGUISliderEventListener>(m_vSliderListener));
        m_vSlider->SetType(m_sliderMode);

        m_vSlider->SetXY(m_nX + m_nMarginX, m_nY + m_nMarginY);

        if (m_sliderMode == VERTICAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
            m_vScrollUpButton = YTL::smart_ptr<CGUIButton>(new CGUIButton());
            m_vScrollDownButton = YTL::smart_ptr<CGUIButton>(new CGUIButton());
        }

        RECT tempSliderRect;
        const int defaultSliderStripSize = 15;

        int effectiveWidth = m_nWidth - (m_nMarginX * 2);
        int effectiveHeight = m_nHeight - (m_nMarginY * 2);

        if (m_sliderMode == VERTICAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
            ::SetRect(&tempSliderRect, effectiveWidth - defaultSliderStripSize, 0 + defaultSliderStripSize, effectiveWidth, effectiveHeight - defaultSliderStripSize);

            RECT upButtonBounds = { effectiveWidth - defaultSliderStripSize, 0, effectiveWidth, defaultSliderStripSize };
            m_vScrollUpButton->SetBounds(upButtonBounds);
            RECT downButtonBounds = { effectiveWidth - defaultSliderStripSize, effectiveHeight - defaultSliderStripSize, effectiveWidth, effectiveHeight };
            m_vScrollDownButton->SetBounds(downButtonBounds);
        } else if (m_sliderMode == HORIZONTAL_SLIDER) {
            ::SetRect(&tempSliderRect, 0, effectiveHeight - defaultSliderStripSize, effectiveWidth, effectiveHeight);
        }

        m_vSlider->SetRect(&tempSliderRect);
    }
}

void CGUITextBox::SetText(const string& text) {
    if (m_strCurrentText != text) {
        m_strCurrentText = text;
        UpdateTextPlane();
    }
}

string CGUITextBox::GetText() const {
    return m_strCurrentText;
}

void CGUITextBox::SetFont(YTL::smart_ptr<CFont> font) {
    if (m_vTextFastPlane != NULL) {
        *(m_vTextFastPlane->GetFont()) = *font;
    }
    if (m_vTitleFastPlane != NULL) {
        *(m_vTitleFastPlane->GetFont()) = *font;
    }
    // **NEW LABEL**: Apply font to title type plane
    if (m_vTitleTypeFastPlane != NULL) {
        *(m_vTitleTypeFastPlane->GetFont()) = *font;
    }
    if (m_vFooterFastPlane != NULL) {
        *(m_vFooterFastPlane->GetFont()) = *font;
    }
    UpdateTextPlane();
}

CFont* CGUITextBox::GetFont() {
    return m_vTextFastPlane != NULL ? m_vTextFastPlane->GetFont() : NULL;
}

void CGUITextBox::SetTextColor(ISurfaceRGB color) {
    m_textColor = color;
    if (m_vTextFastPlane != NULL) {
        m_vTextFastPlane->GetFont()->SetColor(m_textColor);
    }
    if (m_vTitleFastPlane != NULL) {
        m_vTitleFastPlane->GetFont()->SetColor(m_textColor);
    }
    // **NEW LABEL**: Apply color to title type plane
    if (m_vTitleTypeFastPlane != NULL) {
        m_vTitleTypeFastPlane->GetFont()->SetColor(m_textColor);
    }
    if (m_vFooterFastPlane != NULL) {
        m_vFooterFastPlane->GetFont()->SetColor(m_textColor);
    }
    UpdateTextPlane();
}

void CGUITextBox::SetTextOffset(int x, int y) {
    m_nTextOffsetX = x;
    m_nTextOffsetY = y;
}

void CGUITextBox::SetBackgroundPlane(YTL::smart_ptr<ISurface> pv) {
    m_vBackgroundPlane = pv;
}

void CGUITextBox::UpdateTextPlane() {
    // Determine available width for text wrapping (common for all text elements)
    int textAvailableWidth = m_nWidth - (m_nMarginX * 2);
    if (m_sliderMode == VERTICAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
        textAvailableWidth -= m_nSliderStripWidth;
    }
    if (textAvailableWidth < 0) textAvailableWidth = 0;


    // Update Title Text Plane
    if (m_vTitleFastPlane != NULL && !m_strTitleText.empty()) {
        std::string wrappedTitle = WrapText(m_strTitleText, textAvailableWidth);
        m_vTitleFastPlane->GetFont()->SetText(wrappedTitle);
        m_vTitleFastPlane->UpdateText();
        int tempW, tempH;
        m_vTitleFastPlane->GetFont()->GetSize(tempW, tempH);
        m_nTitleHeight = tempH;
    } else {
        m_nTitleHeight = 0;
    }

    // **NEW LABEL**: Update Title Type Text Plane
    if (m_vTitleTypeFastPlane != NULL && !m_strTitleTypeText.empty()) {
        std::string wrappedTitleType = WrapText(m_strTitleTypeText, textAvailableWidth);
        m_vTitleTypeFastPlane->GetFont()->SetText(wrappedTitleType);
        m_vTitleTypeFastPlane->UpdateText();
        int tempW, tempH;
        m_vTitleTypeFastPlane->GetFont()->GetSize(tempW, tempH);
        m_nTitleTypeHeight = tempH;
    } else {
        m_nTitleTypeHeight = 0;
    }

    // Update Footer Text Plane
    if (m_vFooterFastPlane != NULL && !m_strFooterText.empty()) {
        std::string wrappedFooter = WrapText(m_strFooterText, textAvailableWidth);
        m_vFooterFastPlane->GetFont()->SetText(wrappedFooter);
        m_vFooterFastPlane->GetFont()->SetColor(m_textColor);
        m_vFooterFastPlane->UpdateText();
        int tempW, tempH; // Declare temporary ints for GetSize parameters
        m_vFooterFastPlane->GetFont()->GetSize(tempW, tempH);
        m_nFooterHeight = tempH; // Update cached footer height
    } else {
        m_nFooterHeight = 0; // Hide footer if text is empty
    }

    // Calculate available height for the main text content
    int contentAreaHeight = m_nHeight - (m_nMarginY * 2);
    int mainTextAvailableHeight = contentAreaHeight;

    if (m_nTitleHeight > 0) {
        mainTextAvailableHeight -= (m_nTitleHeight + 4); // +4 for spacing between elements
    }
    // **NEW LABEL**: Account for title type height in main text area calculation
    if (m_nTitleTypeHeight > 0) {
        mainTextAvailableHeight -= (m_nTitleTypeHeight + 4); // +4 for spacing
    }
    if (m_nFooterHeight > 0) {
        mainTextAvailableHeight -= m_nFooterHeight;
    }

    if (m_vTextFastPlane != NULL) {
        std::string wrappedContent = WrapText(m_strCurrentText, textAvailableWidth);
        m_vTextFastPlane->GetFont()->SetText(wrappedContent);
        m_vTextFastPlane->SetTextPos(0, 0); // Reset text position for plane update
        m_vTextFastPlane->UpdateText();

        CalculateVisibleContentSize(); // This will recalculate m_nContentHeight and m_nContentWidth
    }

    // Update Slider ItemNum and SelectedItem based on new content dimensions
    if (m_vSlider.get()) {
        int effectiveDisplayWidth = m_nWidth - (m_nMarginX * 2);
        // This effective display height calculation must match the one in CalculateVisibleContentSize
        // for consistent scrollable height.
        int effectiveScrollableHeight = m_nHeight - (m_nMarginY * 2);
        if (m_nTitleHeight > 0) {
             effectiveScrollableHeight -= (m_nTitleHeight + 4);
        }
        // **NEW LABEL**: Account for title type height in effective scrollable height
        if (m_nTitleTypeHeight > 0) {
            effectiveScrollableHeight -= (m_nTitleTypeHeight + 4);
        }
        if (m_nFooterHeight > 0) {
             effectiveScrollableHeight -= m_nFooterHeight;
        }
        if (effectiveScrollableHeight < 0) effectiveScrollableHeight = 0;


        int itemsX = 1; // Default to 1 item if no horizontal scroll
        int itemsY = 1; // Default to 1 item if no vertical scroll

        if (m_sliderMode == HORIZONTAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
            // If content width exceeds effective display width, we need to scroll horizontally
            itemsX = max(1, m_nContentWidth - effectiveDisplayWidth + 1);
        }
        if (m_sliderMode == VERTICAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
            // If content height exceeds effective scrollable height, we need to scroll vertically
            itemsY = max(1, m_nContentHeight - effectiveScrollableHeight + 1);
        }
        m_vSlider->SetItemNum(itemsX, itemsY);

        // Clamp scroll positions to ensure they are within valid range
        m_nScrollX = max(0, min(m_nScrollX, max(0, m_nContentWidth - effectiveDisplayWidth)));
        m_nScrollY = max(0, min(m_nScrollY, max(0, m_nContentHeight - effectiveScrollableHeight)));

        // Update the slider's position to reflect the current scroll
        m_vSlider->SetSelectedItem(m_nScrollX, m_nScrollY);
    }
}

void CGUITextBox::CalculateVisibleContentSize() {
    int mainTextWidth = 0, mainTextHeight = 0;
    if (m_vTextFastPlane != NULL) {
        m_vTextFastPlane->GetFont()->GetSize(mainTextWidth, mainTextHeight);
    }

    // Total content height now includes title, title type, and footer
    m_nContentHeight = mainTextHeight;
    if (m_nTitleHeight > 0) {
        m_nContentHeight += (m_nTitleHeight + 4); // +4 for spacing
    }
    // **NEW LABEL**: Add title type height to total content height
    if (m_nTitleTypeHeight > 0) {
        m_nContentHeight += (m_nTitleTypeHeight + 4); // +4 for spacing
    }
    if (m_nFooterHeight > 0) {
        m_nContentHeight += m_nFooterHeight;
    }

    m_nContentWidth = mainTextWidth; // Assuming main text dictates overall width for now
                                     // (can be adjusted if title/footer width also contributes to max content width)
}

LRESULT CGUITextBox::OnSimpleMove(ISurface* lp) {
    LRESULT result = 0;

    // Declare and calculate effectiveScrollableHeight at the beginning of the function
    int effectiveScrollableHeight = m_nHeight - (m_nMarginY * 2);
    if (m_nTitleHeight > 0) {
        effectiveScrollableHeight -= (m_nTitleHeight + 4);
    }
    // **NEW LABEL**: Account for title type height
    if (m_nTitleTypeHeight > 0) {
        effectiveScrollableHeight -= (m_nTitleTypeHeight + 4);
    }
    if (m_nFooterHeight > 0) {
        effectiveScrollableHeight -= m_nFooterHeight;
    }
    // Ensure effectiveScrollableHeight is not negative
    if (effectiveScrollableHeight < 0) {
        effectiveScrollableHeight = 0;
    }

    // Now calculate needsVerticalScroll based on the adjusted content height
    // m_nContentHeight includes Title, TitleType, and Footer heights
    bool needsVerticalScroll = m_nContentHeight > effectiveScrollableHeight;

    if (m_vSlider.get() && m_pvMouse.get()) {
        result |= m_vSlider->OnSimpleMove(lp);

        int sliderSelectedItemX, sliderSelectedItemY;
        m_vSlider->GetSelectedItem(sliderSelectedItemX, sliderSelectedItemY);

        int targetScrollX = sliderSelectedItemX;
        int targetScrollY = sliderSelectedItemY;

        int effectiveDisplayWidth = m_nWidth - (m_nMarginX * 2);
        int maxScrollX = max(0, m_nContentWidth - effectiveDisplayWidth);
        int maxScrollY = max(0, m_nContentHeight - effectiveScrollableHeight); // Now effectiveScrollableHeight is defined
        targetScrollX = max(0, min(targetScrollX, maxScrollX));
        targetScrollY = max(0, min(targetScrollY, maxScrollY));

        int mouseX, mouseY;
        GetMouse()->GetXY(mouseX, mouseY);
        POINT currentMousePos = { mouseX, mouseY };

        // Only handle vertical scroll related elements if vertical slider is enabled
        // and content actually needs to scroll vertically.
        if ((m_sliderMode == VERTICAL_SLIDER || m_sliderMode == BOTH_SLIDERS) && needsVerticalScroll) {
            if (m_vScrollUpButton.get()) {
                result |= m_vScrollUpButton->OnSimpleMove(lp);
            }
            if (m_vScrollDownButton.get()) {
                result |= m_vScrollDownButton->OnSimpleMove(lp);
            }

            // Check if mouse wheel is used while over the textbox
            // The mouse position check should be against the overall textbox bounds
            RECT mouseCheckRect = {m_nX, m_nY, m_nX + m_nWidth, m_nY + m_nHeight};
            if (::PtInRect(&mouseCheckRect, currentMousePos)) {
                // Get the line height from the slider listener's min_y size, which is set from the thumb graphic
                CGUINormalSliderListener* pSliderListener = static_cast<CGUINormalSliderListener*>(m_vSliderListener.get());
                int sx, sy; // sx and sy here represent min_x and min_y set on slider listener (likely a single line/unit scroll)
                pSliderListener->GetMinSize(sx, sy);

                int scrollDeltaY = 0;

                if (GetMouse()->IsWheelUp()) {
                    scrollDeltaY = -sy; // Scroll up by min_y (e.g., one line height)
                } else if (GetMouse()->IsWheelDown()) {
                    scrollDeltaY = sy;  // Scroll down by min_y
                }

                if (scrollDeltaY != 0) {
                    ScrollContent(0, scrollDeltaY);
                    result |= 1; // Indicate that something changed and a redraw might be needed
                }
            }
        }

        if (targetScrollX != m_nScrollX || targetScrollY != m_nScrollY) {
            m_nScrollX = targetScrollX;
            m_nScrollY = targetScrollY;
            result |= 1; // Indicate change
        }
    }
    return result;
}


LRESULT CGUITextBox::OnSimpleDraw(ISurface* lp) {
    int drawX = m_nX + m_nXOffset;
    int drawY = m_nY + m_nYOffset;
    int currentYOffset = m_nMarginY; // Start drawing from the top margin

    // Draw Background Plane
    if (!m_vBackgroundPlane.isNull()) {
        SIZE destSize = { m_nWidth, m_nHeight };
        // The clipping rectangle for the background is the entire textbox area
        RECT bgClipRect = {m_nX, m_nY, m_nX + m_nWidth, m_nY + m_nHeight};
        lp->BltNatural(m_vBackgroundPlane.get(), drawX, drawY, &destSize, NULL, &bgClipRect, 0);
    }

    // Draw Title Text
    if (m_vTitleFastPlane != NULL && !m_strTitleText.empty()) {
        RECT titleClippingRect = m_rcTextBoxClip; // Start with the overall clip
        titleClippingRect.top = drawY + currentYOffset;
        titleClippingRect.bottom = titleClippingRect.top + m_nTitleHeight;

        // Adjust clipping for slider area
        if (m_sliderMode == VERTICAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
            titleClippingRect.right = min(titleClippingRect.right, drawX + m_nWidth - m_nMarginX - m_nSliderStripWidth);
        }

        // The source rectangle covers the entire rendered title plane
        RECT titleSrcRect = {0, 0, 0, 0};
        int tempW_title, tempH_title;
        m_vTitleFastPlane->GetFont()->GetSize(tempW_title, tempH_title);
        titleSrcRect.right = tempW_title;
        titleSrcRect.bottom = tempH_title;

        lp->BltNatural(m_vTitlePlane.get(),
                       drawX + m_nMarginX, // Draw at textbox X + margin
                       drawY + currentYOffset, // Draw at textbox Y + current vertical offset
                       NULL,
                       &titleSrcRect,
                       &titleClippingRect,
                       0);
        currentYOffset += (m_nTitleHeight + 4); // Advance offset for next element, +4 for spacing
    }

    // **NEW LABEL**: Draw Title Type Text
    if (m_vTitleTypeFastPlane != NULL && !m_strTitleTypeText.empty()) {
        RECT titleTypeClippingRect = m_rcTextBoxClip; // Start with the overall clip
        titleTypeClippingRect.top = drawY + currentYOffset;
        titleTypeClippingRect.bottom = titleTypeClippingRect.top + m_nTitleTypeHeight;

        // Adjust clipping for slider area
        if (m_sliderMode == VERTICAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
            titleTypeClippingRect.right = min(titleTypeClippingRect.right, drawX + m_nWidth - m_nMarginX - m_nSliderStripWidth);
        }

        RECT titleTypeSrcRect = {0, 0, 0, 0};
        int tempW_titleType, tempH_titleType;
        m_vTitleTypeFastPlane->GetFont()->GetSize(tempW_titleType, tempH_titleType);
        titleTypeSrcRect.right = tempW_titleType;
        titleTypeSrcRect.bottom = tempH_titleType;

        lp->BltNatural(m_vTitleTypePlane.get(),
                       drawX + m_nMarginX, // Draw at textbox X + margin
                       drawY + currentYOffset, // Draw at textbox Y + current vertical offset
                       NULL,
                       &titleTypeSrcRect,
                       &titleTypeClippingRect,
                       0);
        currentYOffset += (m_nTitleTypeHeight + 4); // Advance offset for next element, +4 for spacing
    }


    // Draw the main text plane
    if (m_vTextFastPlane != NULL) {
        RECT srcRect;
        // Calculate the height available for the main text, accounting for margins, title, title type, and footer
        int mainTextDisplayHeight = m_nHeight - (m_nMarginY * 2) - m_nTitleHeight - m_nTitleTypeHeight - m_nFooterHeight - (m_nTitleHeight > 0 ? 4 : 0) - (m_nTitleTypeHeight > 0 ? 4 : 0);
        if (m_sliderMode == HORIZONTAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
            mainTextDisplayHeight -= m_nSliderStripHeight; // Subtract space for horizontal slider if present
        }
        if (mainTextDisplayHeight < 0) mainTextDisplayHeight = 0;

        int textContentWidth_main, textContentHeight_main;
        m_vTextFastPlane->GetFont()->GetSize(textContentWidth_main, textContentHeight_main);

        // Source rectangle for the main text, based on current scroll position and available display area
        ::SetRect(&srcRect, m_nScrollX, m_nScrollY,
                                         m_nScrollX + (m_nWidth - (m_nMarginX * 2) - ( (m_sliderMode == VERTICAL_SLIDER || m_sliderMode == BOTH_SLIDERS) ? m_nSliderStripWidth : 0)),
                                         m_nScrollY + mainTextDisplayHeight);

        // Clamp source rectangle to actual content dimensions
        srcRect.right = min(srcRect.right, textContentWidth_main);
        srcRect.bottom = min(srcRect.bottom, textContentHeight_main);

        // Clipping rectangle for the main text
        RECT textClippingRect = m_rcTextBoxClip; // Start with overall clip
        textClippingRect.top = drawY + currentYOffset;
        textClippingRect.bottom = drawY + m_nHeight - m_nMarginY - m_nFooterHeight;

        if (m_sliderMode == VERTICAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
            textClippingRect.right = min(textClippingRect.right, drawX + m_nWidth - m_nMarginX - m_nSliderStripWidth);
        }
        if (m_sliderMode == HORIZONTAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
            textClippingRect.bottom = min(textClippingRect.bottom, drawY + m_nHeight - m_nMarginY - m_nSliderStripHeight);
        }
        // Ensure valid clipping rect
        if (textClippingRect.left > textClippingRect.right) textClippingRect.right = textClippingRect.left;
        if (textClippingRect.top > textClippingRect.bottom) textClippingRect.bottom = textClippingRect.top;


        lp->BltNatural(m_vTextPlane.get(),
                       drawX + m_nTextOffsetX + m_nMarginX, // Draw X with offset and margin
                       drawY + m_nTextOffsetY + currentYOffset, // Draw Y with offset and current cumulative Y offset
                       NULL,
                       &srcRect,
                       &textClippingRect,
                       0);
        // currentYOffset is not incremented here for the main text as its height is dynamic and used for scrolling.
        // It's already factored into mainTextDisplayHeight and m_nContentHeight.
    }

    // Draw Footer Text
    if (m_vFooterFastPlane != NULL && !m_strFooterText.empty()) {
        RECT footerClippingRect = m_rcTextBoxClip; // Start with overall clip
        footerClippingRect.top = drawY + m_nHeight - m_nMarginY - m_nFooterHeight; // Position from bottom
        footerClippingRect.bottom = drawY + m_nHeight - m_nMarginY;

        // --- IMPORTANT FIX HERE: Adjust footer's effective right boundary for slider ---
        int effectiveRightBound = m_nX + m_nWidth - m_nMarginX;
        if (m_sliderMode == VERTICAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
            effectiveRightBound -= m_nSliderStripWidth;
        }
        // --- END IMPORTANT FIX ---

        // Adjust clipping for slider area
        footerClippingRect.right = min(footerClippingRect.right, effectiveRightBound);


        int footerDrawX;
        int textWidth_footer, textHeight_footer;
        m_vFooterFastPlane->GetFont()->GetSize(textWidth_footer, textHeight_footer);
        
        // Calculate footerDrawX for right alignment, accounting for slider width
        footerDrawX = effectiveRightBound - textWidth_footer;
        // Also subtract drawX from footerDrawX if drawX is not 0 (relative to overall window origin)
        // This makes it relative to the textbox's top-left corner
        footerDrawX = (m_nX + m_nXOffset) + (effectiveRightBound - textWidth_footer - (m_nX + m_nXOffset));


        lp->BltNatural(m_vFooterPlane.get(),
                       footerDrawX,
                       drawY + m_nHeight - m_nMarginY - m_nFooterHeight, // Draw at bottom minus margin and footer height
                       NULL,
                       NULL, // Draw entire footer plane
                       &footerClippingRect,
                       0);
    }

    // Recalculate effectiveDisplayHeight for rendering scrollbar, ensuring consistency
    int effectiveDisplayHeight = m_nHeight - (m_nMarginY * 2);
    if (m_nTitleHeight > 0) {
        effectiveDisplayHeight -= (m_nTitleHeight + 4);
    }
    // **NEW LABEL**: Account for title type height in effectiveDisplayHeight
    if (m_nTitleTypeHeight > 0) {
        effectiveDisplayHeight -= (m_nTitleTypeHeight + 4);
    }
    if (m_nFooterHeight > 0) {
        effectiveDisplayHeight -= m_nFooterHeight;
    }
    if (effectiveDisplayHeight < 0) {
        effectiveDisplayHeight = 0;
    }

    bool needsVerticalScroll = m_nContentHeight > effectiveDisplayHeight;

    // Draw Scrollbar and Buttons
    if ((m_sliderMode == VERTICAL_SLIDER || m_sliderMode == BOTH_SLIDERS) && needsVerticalScroll) {
        if (m_vScrollUpButton.get()) {
            m_vScrollUpButton->OnSimpleDraw(lp);
        }
        if (m_vScrollDownButton.get()) {
            m_vScrollDownButton->OnSimpleDraw(lp);
        }
        if (m_vSlider.get()) {
            m_vSlider->OnSimpleDraw(lp);
        }
    }
    // TODO: Add similar logic for horizontal scrollbars if needed
    // if ((m_sliderMode == HORIZONTAL_SLIDER || m_sliderMode == BOTH_SLIDERS) && m_nContentWidth > effectiveDisplayWidth) { ... }

    return 0;
}

void CGUITextBox::Reset() {
    IGUIParts::Reset();

    m_strCurrentText = "";
    m_strTitleText = "";
    m_strTitleTypeText = ""; // **NEW LABEL**: Reset
    m_strFooterText = "";
    m_nScrollX = 0;
    m_nScrollY = 0;
    m_nContentWidth = 0;
    m_nContentHeight = 0;
    m_nMarginX = 0;
    m_nMarginY = 0;
    m_nTitleHeight = 0;
    m_nTitleTypeHeight = 0; // **NEW LABEL**: Reset
    m_nFooterHeight = 0;

    if (m_vTextFastPlane != NULL) {
        m_vTextFastPlane->GetFont()->SetText("");
        m_vTextFastPlane->UpdateTextAA(); // Assuming UpdateTextAA is for updating the plane
    }
    if (m_vTitleFastPlane != NULL) {
        m_vTitleFastPlane->GetFont()->SetText("");
        m_vTitleFastPlane->UpdateTextAA();
    }
    // **NEW LABEL**: Reset title type plane
    if (m_vTitleTypeFastPlane != NULL) {
        m_vTitleTypeFastPlane->GetFont()->SetText("");
        m_vTitleTypeFastPlane->UpdateTextAA();
    }
    if (m_vFooterFastPlane != NULL) {
        m_vFooterFastPlane->GetFont()->SetText("");
        m_vFooterFastPlane->UpdateTextAA();
    }

    if (m_vSlider.get()) {
        m_vSlider->Reset();
    }

    if (m_vScrollUpButton.get()) {
        m_vScrollUpButton->Reset();
    }
    if (m_vScrollDownButton.get()) {
        m_vScrollDownButton->Reset();
    }
}

void CGUITextBox::SetMouse(YTL::smart_ptr<CFixMouse> pv) {
    IGUIParts::SetMouse(pv);
    if (m_vSlider.get()) {
        m_vSlider->SetMouse(pv);
    }

    if (m_vScrollUpButton.get()) {
        m_vScrollUpButton->SetMouse(pv);
    }
    if (m_vScrollDownButton.get()) {
        m_vScrollDownButton->SetMouse(pv);
    }
}

void CGUITextBox::ScrollContent(int dx, int dy) {
    int oldScrollX = m_nScrollX;
    int oldScrollY = m_nScrollY;

    m_nScrollX += dx;
    m_nScrollY += dy;

    int effectiveDisplayWidth = m_nWidth - (m_nMarginX * 2);

    // Declare and calculate effectiveScrollableHeight here, consistent with OnSimpleMove and UpdateTextPlane
    int effectiveScrollableHeight = m_nHeight - (m_nMarginY * 2);
    if (m_nTitleHeight > 0) {
        effectiveScrollableHeight -= (m_nTitleHeight + 4);
    }
    // **NEW LABEL**: Account for title type height
    if (m_nTitleTypeHeight > 0) {
        effectiveScrollableHeight -= (m_nTitleTypeHeight + 4);
    }
    if (m_nFooterHeight > 0) {
        effectiveScrollableHeight -= m_nFooterHeight;
    }
    // Ensure effectiveScrollableHeight is not negative
    if (effectiveScrollableHeight < 0) {
        effectiveScrollableHeight = 0;
    }

    // Clamp scroll positions to ensure they are within valid range
    m_nScrollX = max(0, min(m_nScrollX, max(0, m_nContentWidth - effectiveDisplayWidth)));
    m_nScrollY = max(0, min(m_nScrollY, max(0, m_nContentHeight - effectiveScrollableHeight)));

    if (oldScrollX != m_nScrollX || oldScrollY != m_nScrollY) {
        if (m_vSlider.get()) {
            m_vSlider->SetSelectedItem(m_nScrollX, m_nScrollY);
        }
    }
}

std::string CGUITextBox::TrimLeadingSpaces(const std::string& s) {
    size_t first = s.find_first_not_of(" \t\n\r\f\v");
    if (std::string::npos == first) {
        return s;
    }
    return s.substr(first);
}

std::string yaneuraoGameSDK3rd::Draw::CGUITextBox::WrapText(const std::string& rawText, int availableWidth) {
    if (rawText.empty() || availableWidth <= 0) {
        return "";
    }

    std::string finalWrappedText;
    CFont* font = m_vTextFastPlane->GetFont(); // Assuming this font is used for all text measurements

    // Store the original text to restore it later, to avoid side effects on the font object
    std::string originalFontText = font->GetText();

    size_t lineStart = 0;

    while (lineStart < rawText.length()) {
        size_t lineEnd = rawText.find('\n', lineStart);
        std::string segmentToWrap;
        bool endsWithExplicitNewline = false;

        if (lineEnd == std::string::npos) {
            segmentToWrap = rawText.substr(lineStart);
            lineStart = rawText.length(); // Move to end to exit loop
        } else {
            segmentToWrap = rawText.substr(lineStart, lineEnd - lineStart);
            lineStart = lineEnd + 1; // Move past the newline
            endsWithExplicitNewline = true;
        }

        std::string wrappedSegmentResult;
        std::string currentLineInSegment;
        size_t wordCurrentPos = 0;
        int tempWidth, tempHeight;

        // Process each word in the segment
        while (wordCurrentPos < segmentToWrap.length()) {
            // Find the start of the next word (skip leading whitespace)
            size_t wordSegmentStart = segmentToWrap.find_first_not_of(" \t\r\f\v", wordCurrentPos);
            if (std::string::npos == wordSegmentStart) {
                break; // No more words in this segment
            }

            // Find the end of the current word (first whitespace after word, or end of segment)
            size_t wordSegmentEnd = segmentToWrap.find_first_of(" \t\r\f\v", wordSegmentStart);
            std::string word;
            if (std::string::npos == wordSegmentEnd) {
                word = segmentToWrap.substr(wordSegmentStart);
                wordCurrentPos = segmentToWrap.length(); // Move to end of segment
            } else {
                word = segmentToWrap.substr(wordSegmentStart, wordSegmentEnd - wordSegmentStart);
                wordCurrentPos = wordSegmentEnd; // Move to the whitespace after the word
            }

            std::string testLine;
            if (!currentLineInSegment.empty()) {
                testLine = currentLineInSegment + " " + word; // Add space before word if not the first
            } else {
                testLine = word;
            }

            font->SetText(testLine);
            font->GetSize(tempWidth, tempHeight);

            // If the test line exceeds available width AND we have something in currentLineInSegment
            // (meaning the 'word' itself isn't too long to fit on a fresh line),
            // then add currentLineInSegment to wrapped result and start a new line with 'word'.
            if (tempWidth > availableWidth && !currentLineInSegment.empty()) {
                wrappedSegmentResult += currentLineInSegment + "\n";
                currentLineInSegment = word;
            } else {
                currentLineInSegment = testLine;
            }
        }

        // Add any remaining text in currentLineInSegment
        if (!currentLineInSegment.empty()) {
            wrappedSegmentResult += currentLineInSegment;
        }

        finalWrappedText += wrappedSegmentResult;

        // If the original segment ended with an explicit newline, preserve it
        if (endsWithExplicitNewline) {
            finalWrappedText += "\n";
        }
    }

    // Restore the font's original text
    font->SetText(originalFontText);

    // Edge case: Remove trailing newline if the original text didn't end with one
    // and wrapping caused one to be added unnecessarily at the very end.
    if (!rawText.empty() && rawText[rawText.length()-1] != '\n' &&
        !finalWrappedText.empty() && finalWrappedText[finalWrappedText.length()-1] == '\n') {
        finalWrappedText.erase(finalWrappedText.length()-1);
    }

    return finalWrappedText;
}

} // namespace Draw
} // namespace yaneuraoGameSDK3rd