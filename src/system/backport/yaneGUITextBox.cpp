#include "../../stdafx.h" // Corrected path
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
      m_nFooterHeight(0)
{
    m_textColor = ISurface::makeRGB(255, 255, 255, 0); // White, opaque
    ::SetRect(&m_rcTextBoxClip, 0, 0, 0, 0);
}

CGUITextBox::~CGUITextBox() {
    // Smart pointers handle deallocation
}

// --- New Margin Methods Implementation ---
void CGUITextBox::SetMargins(int x, int y) {
    m_nMarginX = max(0, x);
    m_nMarginY = max(0, y);
    UpdateTextPlane(); // Recalculate and redraw text with new margins
}

void CGUITextBox::GetMargins(int& x, int& y) const {
    x = m_nMarginX;
    y = m_nMarginY;
}
// --- End New Margin Methods ---

// --- NEW: Title and Footer Text Implementation ---
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
        m_vTitleFastPlane->GetFont()->SetText(m_strTitleText);
        m_vTitleFastPlane->GetFont()->SetColor(m_textColor);
        m_vTitleFastPlane->UpdateText();
        // Use GetSize on the font for accurate text dimensions
        int tempW, tempH; // Declare temporary ints for GetSize parameters
        m_vTitleFastPlane->GetFont()->GetSize(tempW, tempH);
        m_nTitleHeight = tempH; // Update cached title height
        UpdateTextPlane(); // Recalculate overall content size and slider range
    }
}

string CGUITextBox::GetTextTitle() const {
    return m_strTitleText;
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
            m_vFooterFastPlane->GetFont()->SetText(m_strFooterText);
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
        CGUINormalButtonListener* downListener = static_cast<CGUINormalButtonListener*>(m_vScrollDownButtonListener.get()); // Corrected cast

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
    if (m_vTitleFastPlane != NULL && !m_strTitleText.empty()) {
        m_vTitleFastPlane->GetFont()->SetText(m_strTitleText);
        m_vTitleFastPlane->UpdateText();
        int tempW, tempH;
        m_vTitleFastPlane->GetFont()->GetSize(tempW, tempH);
        m_nTitleHeight = tempH;
    } else {
        m_nTitleHeight = 0;
    }

    if (m_vFooterFastPlane != NULL && !m_strFooterText.empty()) {
        m_vFooterFastPlane->GetFont()->SetText(m_strFooterText);
        m_vFooterFastPlane->UpdateText();
        int tempW, tempH;
        m_vFooterFastPlane->GetFont()->GetSize(tempW, tempH);
        m_nFooterHeight = tempH;
    } else {
        m_nFooterHeight = 0;
    }

    int contentAreaHeight = m_nHeight - (m_nMarginY * 2);
    int mainTextAvailableHeight = contentAreaHeight;

    if (m_nTitleHeight > 0) {
        mainTextAvailableHeight -= (m_nTitleHeight + 4);
    }
    if (m_nFooterHeight > 0) {
        mainTextAvailableHeight -= m_nFooterHeight;
    }

    int textAvailableWidth = m_nWidth - (m_nMarginX * 2);

    if (m_sliderMode == VERTICAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
        textAvailableWidth -= m_nSliderStripWidth;
    }
    if (textAvailableWidth < 0) textAvailableWidth = 0;

    if (m_vTextFastPlane != NULL) {
        std::string wrappedContent = WrapText(m_strCurrentText, textAvailableWidth);
        m_vTextFastPlane->GetFont()->SetText(wrappedContent);
        m_vTextFastPlane->SetTextPos(0, 0);
        m_vTextFastPlane->UpdateText();

        CalculateVisibleContentSize();
    }

    if (m_vSlider.get()) {
        int effectiveDisplayWidth = m_nWidth - (m_nMarginX * 2);
        int effectiveDisplayHeight = m_nHeight - (m_nMarginY * 2);

        // This calculation needs to be consistent with OnSimpleMove and ScrollContent
        int effectiveScrollableHeight = effectiveDisplayHeight;
        if (m_nTitleHeight > 0) {
             effectiveScrollableHeight -= (m_nTitleHeight + 4);
        }
        if (m_nFooterHeight > 0) {
             effectiveScrollableHeight -= m_nFooterHeight;
        }
        if (effectiveScrollableHeight < 0) effectiveScrollableHeight = 0;

        int itemsX = 1;
        int itemsY = 1;

        if (m_sliderMode == HORIZONTAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
            itemsX = max(1, m_nContentWidth - effectiveDisplayWidth + 1);
        }
        if (m_sliderMode == VERTICAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
            itemsY = max(1, m_nContentHeight - effectiveScrollableHeight + 1);
        }
        m_vSlider->SetItemNum(itemsX, itemsY);

        m_nScrollX = max(0, min(m_nScrollX, max(0, m_nContentWidth - effectiveDisplayWidth)));
        m_nScrollY = max(0, min(m_nScrollY, max(0, m_nContentHeight - effectiveScrollableHeight)));

        m_vSlider->SetSelectedItem(m_nScrollX, m_nScrollY);
    }
}

void CGUITextBox::CalculateVisibleContentSize() {
    int mainTextWidth = 0, mainTextHeight = 0;
    if (m_vTextFastPlane != NULL) {
        m_vTextFastPlane->GetFont()->GetSize(mainTextWidth, mainTextHeight);
    }

    m_nContentHeight = mainTextHeight;
    if (m_nTitleHeight > 0) {
        m_nContentHeight += (m_nTitleHeight + 4);
    }
    m_nContentWidth = mainTextWidth;
}

LRESULT CGUITextBox::OnSimpleMove(ISurface* lp) {
    LRESULT result = 0;

    // Declare and calculate effectiveScrollableHeight at the beginning of the function
    int effectiveScrollableHeight = m_nHeight - (m_nMarginY * 2);
    if (m_nTitleHeight > 0) {
        effectiveScrollableHeight -= (m_nTitleHeight + 4);
    }
    if (m_nFooterHeight > 0) {
        effectiveScrollableHeight -= m_nFooterHeight;
    }
    // Ensure effectiveScrollableHeight is not negative
    if (effectiveScrollableHeight < 0) {
        effectiveScrollableHeight = 0;
    }

    // Now calculate needsVerticalScroll
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

        if (m_sliderMode == VERTICAL_SLIDER && needsVerticalScroll) {
            if (m_vScrollUpButton.get()) {
                result |= m_vScrollUpButton->OnSimpleMove(lp);
            }
            if (m_vScrollDownButton.get()) {
                result |= m_vScrollDownButton->OnSimpleMove(lp);
            }

            RECT mouseCheckRect = {m_nX, m_nY, m_nX + m_nWidth, m_nY + m_nHeight};
            if (::PtInRect(&mouseCheckRect, currentMousePos)) {
                CGUINormalSliderListener* pSliderListener = static_cast<CGUINormalSliderListener*>(m_vSliderListener.get());
                int sx, sy;
                pSliderListener->GetMinSize(sx, sy);

                int scrollDeltaY = 0;

                if (GetMouse()->IsWheelUp()) {
                    scrollDeltaY = -sy;
                } else if (GetMouse()->IsWheelDown()) {
                    scrollDeltaY = sy;
                }

                if (scrollDeltaY != 0) {
                    ScrollContent(0, scrollDeltaY);
                    result |= 1;
                }
            }
        }

        if (targetScrollX != m_nScrollX || targetScrollY != m_nScrollY) {
            m_nScrollX = targetScrollX;
            m_nScrollY = targetScrollY;
            result |= 1;
        }
    }
    return result;
}


LRESULT CGUITextBox::OnSimpleDraw(ISurface* lp) {
    int drawX = m_nX + m_nXOffset;
    int drawY = m_nY + m_nYOffset;

    if (!m_vBackgroundPlane.isNull()) {
        SIZE destSize = { m_nWidth, m_nHeight };
        lp->BltNatural(m_vBackgroundPlane.get(), drawX, drawY, &destSize, NULL, &m_rcTextBoxClip, 0);
    }

    int currentYOffset = 0;

    // Draw Title Text
    if (m_vTitleFastPlane != NULL && !m_strTitleText.empty()) {
        RECT titleClippingRect = m_rcTextBoxClip;
        titleClippingRect.top = drawY + m_nMarginY;
        titleClippingRect.bottom = titleClippingRect.top + m_nTitleHeight;

        if (m_sliderMode == VERTICAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
            titleClippingRect.right -= m_nSliderStripWidth;
        }

        RECT titleSrcRect = {0, 0, 0, 0}; // Initialize
        int tempW_title, tempH_title; // Use temporary ints for GetSize
        m_vTitleFastPlane->GetFont()->GetSize(tempW_title, tempH_title);
        titleSrcRect.right = tempW_title;
        titleSrcRect.bottom = tempH_title;

        lp->BltNatural(m_vTitlePlane.get(),
                       drawX + m_nMarginX,
                       drawY + m_nMarginY,
                       NULL,
                       &titleSrcRect,
                       &titleClippingRect,
                       0);
        currentYOffset += (m_nTitleHeight + 4);
    }

    // Draw the main text plane
    if (m_vTextFastPlane != NULL) {
        RECT srcRect;
        int mainTextDisplayHeight = m_nHeight - (m_nMarginY * 2) - currentYOffset - m_nFooterHeight;
        if (m_sliderMode == HORIZONTAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
            mainTextDisplayHeight -= m_nSliderStripHeight;
        }
        if (mainTextDisplayHeight < 0) mainTextDisplayHeight = 0;

        int textContentWidth_main, textContentHeight_main;
        m_vTextFastPlane->GetFont()->GetSize(textContentWidth_main, textContentHeight_main);

        ::SetRect(&srcRect, m_nScrollX, m_nScrollY,
                                         m_nScrollX + m_nWidth - (m_nMarginX * 2),
                                         m_nScrollY + mainTextDisplayHeight);

        srcRect.right = min(srcRect.right, textContentWidth_main);
        srcRect.bottom = min(srcRect.bottom, textContentHeight_main);

        RECT textClippingRect = m_rcTextBoxClip;
        textClippingRect.top = drawY + m_nMarginY + currentYOffset;
        textClippingRect.bottom = drawY + m_nHeight - m_nMarginY - m_nFooterHeight;

        if (m_sliderMode == VERTICAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
            textClippingRect.right -= m_nSliderStripWidth;
        }
        if (m_sliderMode == HORIZONTAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
            textClippingRect.bottom -= m_nSliderStripHeight;
        }
        if (textClippingRect.left > textClippingRect.right) textClippingRect.right = textClippingRect.left;
        if (textClippingRect.top > textClippingRect.bottom) textClippingRect.bottom = textClippingRect.top;

        lp->BltNatural(m_vTextPlane.get(),
                       drawX + m_nTextOffsetX + m_nMarginX,
                       drawY + m_nTextOffsetY + m_nMarginY + currentYOffset,
                       NULL,
                       &srcRect,
                       &textClippingRect,
                       0);
    }

    // Draw Footer Text
    if (m_vFooterFastPlane != NULL && !m_strFooterText.empty()) {
        RECT footerClippingRect = m_rcTextBoxClip;
        footerClippingRect.top = drawY + m_nHeight - m_nMarginY - m_nFooterHeight;
        footerClippingRect.bottom = drawY + m_nHeight - m_nMarginY;

        if (m_sliderMode == VERTICAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
            footerClippingRect.right -= m_nSliderStripWidth;
        }

        int footerDrawX = drawX + m_nMarginX;
        int textWidth_footer, textHeight_footer;
        m_vFooterFastPlane->GetFont()->GetSize(textWidth_footer, textHeight_footer);
        footerDrawX = drawX + m_nWidth - m_nMarginX - textWidth_footer;

        lp->BltNatural(m_vFooterPlane.get(),
                       footerDrawX,
                       drawY + m_nHeight - m_nMarginY - m_nFooterHeight,
                       NULL,
                       NULL,
                       &footerClippingRect,
                       0);
    }

    // Also declare effectiveDisplayHeight here for OnSimpleDraw's calculations
    int effectiveDisplayHeight = m_nHeight - (m_nMarginY * 2);
    if (m_nTitleHeight > 0) {
        effectiveDisplayHeight -= (m_nTitleHeight + 4);
    }
    if (m_nFooterHeight > 0) {
        effectiveDisplayHeight -= m_nFooterHeight;
    }
    // Ensure it's not negative
    if (effectiveDisplayHeight < 0) {
        effectiveDisplayHeight = 0;
    }

    bool needsVerticalScroll = m_nContentHeight > effectiveDisplayHeight;
    if (m_sliderMode == VERTICAL_SLIDER && needsVerticalScroll) {
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
    // if (m_sliderMode == HORIZONTAL_SLIDER && m_nContentWidth > m_nWidth) { ... }

    return 0;
}

void CGUITextBox::Reset() {
    IGUIParts::Reset();

    m_strCurrentText = "";
    m_strTitleText = "";
    m_strFooterText = "";
    m_nScrollX = 0;
    m_nScrollY = 0;
    m_nContentWidth = 0;
    m_nContentHeight = 0;
    m_nMarginX = 0;
    m_nMarginY = 0;
    m_nTitleHeight = 0;
    m_nFooterHeight = 0;

    if (m_vTextFastPlane != NULL) {
        m_vTextFastPlane->GetFont()->SetText("");
        m_vTextFastPlane->UpdateTextAA();
    }
    if (m_vTitleFastPlane != NULL) {
        m_vTitleFastPlane->GetFont()->SetText("");
        m_vTitleFastPlane->UpdateTextAA();
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

    // Declare and calculate effectiveScrollableHeight at the beginning of the function
    int effectiveScrollableHeight = m_nHeight - (m_nMarginY * 2);
    if (m_nTitleHeight > 0) {
        effectiveScrollableHeight -= (m_nTitleHeight + 4);
    }
    if (m_nFooterHeight > 0) {
        effectiveScrollableHeight -= m_nFooterHeight;
    }
    // Ensure effectiveScrollableHeight is not negative
    if (effectiveScrollableHeight < 0) {
        effectiveScrollableHeight = 0;
    }

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
    CFont* font = m_vTextFastPlane->GetFont();

    std::string originalFontText = font->GetText();

    size_t lineStart = 0;

    while (lineStart < rawText.length()) {
        size_t lineEnd = rawText.find('\n', lineStart);
        std::string segmentToWrap;
        bool endsWithExplicitNewline = false;

        if (lineEnd == std::string::npos) {
            segmentToWrap = rawText.substr(lineStart);
            lineStart = rawText.length();
        } else {
            segmentToWrap = rawText.substr(lineStart, lineEnd - lineStart);
            lineStart = lineEnd + 1;
            endsWithExplicitNewline = true;
        }

        std::string wrappedSegmentResult;
        std::string currentLineInSegment;
        size_t wordCurrentPos = 0;
        int tempWidth, tempHeight;

        while (wordCurrentPos < segmentToWrap.length()) {
            size_t wordSegmentStart = segmentToWrap.find_first_not_of(" \t\r\f\v", wordCurrentPos);
            if (std::string::npos == wordSegmentStart) {
                break;
            }

            size_t wordSegmentEnd = segmentToWrap.find_first_of(" \t\r\f\v", wordSegmentStart);
            std::string word;
            if (std::string::npos == wordSegmentEnd) {
                word = segmentToWrap.substr(wordSegmentStart);
                wordCurrentPos = segmentToWrap.length();
            } else {
                word = segmentToWrap.substr(wordSegmentStart, wordSegmentEnd - wordSegmentStart);
                wordCurrentPos = wordSegmentEnd;
            }

            std::string testLine;
            if (!currentLineInSegment.empty()) {
                testLine = currentLineInSegment + " " + word;
            } else {
                testLine = word;
            }

            font->SetText(testLine);
            font->GetSize(tempWidth, tempHeight);

            if (tempWidth > availableWidth && !currentLineInSegment.empty()) {
                wrappedSegmentResult += currentLineInSegment + "\n";
                currentLineInSegment = word;
            } else {
                currentLineInSegment = testLine;
            }
        }

        if (!currentLineInSegment.empty()) {
            wrappedSegmentResult += currentLineInSegment;
        }

        finalWrappedText += wrappedSegmentResult;

        if (endsWithExplicitNewline) {
            finalWrappedText += "\n";
        }
    }

    font->SetText(originalFontText);

    if (!rawText.empty() && rawText[rawText.length()-1] != '\n' &&
        !finalWrappedText.empty() && finalWrappedText[finalWrappedText.length()-1] == '\n') {
        finalWrappedText.erase(finalWrappedText.length()-1);
    }

    return finalWrappedText;
}

} // namespace Draw
} // namespace yaneuraoGameSDK3rd