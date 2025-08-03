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

// No longer need SetPlaneLoader or GetDrawSurface here; CGUINormalButtonListener handles it.
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
    // Assuming SetMinSize is part of CGUINormalSliderListener or CGUISliderEventListener
    SetMinSize(5, 5);
}

void CGUITextBoxSliderListener::OnPageUp() {
    if (m_vTextBox.get()) {
		if (m_vTextBox->GetSlider()->GetButton() == 2) //m_vTextBox->GetSlider()->GetButton() != 0 && m_vTextBox->GetMouse()->IsPushLButton()
		{
			m_vTextBox->ScrollContent(0, -m_nMinY); // Scroll up by min slider height
			//ResetEventFlag(); // Reset event flag after handling
		}
    }
}

void CGUITextBoxSliderListener::OnPageDown() {
    if (m_vTextBox.get()) {
		if (m_vTextBox->GetSlider()->GetButton() == 2) //m_vTextBox->GetSlider()->GetButton() != 0 && m_vTextBox->GetMouse()->IsPushLButton()
		{
			m_vTextBox->ScrollContent(0, m_nMinY); // Scroll down by min slider height
			//ResetEventFlag(); // Reset event flag after handling
		}
    }
}

void CGUITextBoxSliderListener::OnPageLeft() {
	// TODO: not working
    if (m_vTextBox.get()) {
        m_vTextBox->ScrollContent(-m_nMinX, 0); // Scroll left by min slider width
        //ResetEventFlag(); // Reset event flag after handling
    }
}

void CGUITextBoxSliderListener::OnPageRight() {
	// TODO: not working
    if (m_vTextBox.get()) {
        m_vTextBox->ScrollContent(m_nMinX, 0); // Scroll right by min slider width
        //ResetEventFlag(); // Reset event flag after handling
    }
}

///////////////////////////////////////////////////////////////////////////////
// CGUITextBox Implementation
///////////////////////////////////////////////////////////////////////////////

CGUITextBox::CGUITextBox()
    : m_nWidth(0), m_nHeight(0), m_nContentWidth(0), m_nContentHeight(0),
      m_nScrollX(0), m_nScrollY(0), m_sliderMode(NO_SLIDER),
      m_nTextOffsetX(0), m_nTextOffsetY(0),
	  m_nSliderStripWidth(15),  // Initialize with a default, will be updated in SetSliderGFX
      m_nSliderStripHeight(15), // Initialize with a default, will be updated in SetSliderGFX
      m_nMarginX(0), // Initialize new member
      m_nMarginY(0),  // Initialize new member
      m_vTitleTextFastPlane(NULL), // Initialize new title plane
      m_vFooterTextFastPlane(NULL), // Initialize new footer plane
      m_nFooterHeight(0) // Initialize footer height
{
    // Default text color. Use ISurface::makeRGB since ISurfaceRGB is a DWORD typedef.
    // Assuming 0 for alpha (fully opaque) for common text rendering, adjust if different blend mode needed.
    m_textColor = ISurface::makeRGB(255, 255, 255, 0); // White, opaque

    // Initialize m_rcTextBoxClip (local to CGUITextBox)
    ::SetRect(&m_rcTextBoxClip, 0, 0, 0, 0);
}

CGUITextBox::~CGUITextBox() {
    // Smart pointers handle deallocation, but raw CTextFastPlane pointers need delete
    if (m_vTextFastPlane) delete m_vTextFastPlane;
    if (m_vTitleTextFastPlane) delete m_vTitleTextFastPlane;
    if (m_vFooterTextFastPlane) delete m_vFooterTextFastPlane;
}

// --- New Margin Methods Implementation ---
void CGUITextBox::SetMargins(int x, int y) {
    m_nMarginX = max(0, x); // Margins shouldn't be negative
    m_nMarginY = max(0, y);
    // Recalculate everything that depends on dimensions
    UpdateTextPlane();
    UpdateTitleAndFooterPlanes();
}

void CGUITextBox::GetMargins(int& x, int& y) const {
    x = m_nMarginX;
    y = m_nMarginY;
}
// --- End New Margin Methods ---

// --- NEW: Title and Footer Text methods implementation ---
void CGUITextBox::SetTextTitle(const string& text) {
    if (m_strTitleText != text) {
        m_strTitleText = text;
        UpdateTitleAndFooterPlanes(); // Update title rendering
        UpdateTextPlane(); // Re-evaluate main text area and scroll bars
    }
}

void CGUITextBox::SetTextFooter(const string& text) {
    if (m_strFooterText != text) {
        m_strFooterText = text;
        UpdateTitleAndFooterPlanes(); // Update footer rendering
        UpdateTextPlane(); // Re-evaluate main text area and scroll bars
    }
}
// --- END NEW ---

void CGUITextBox::SetSliderGFX(smart_ptr<ISurface> sliderThumbGraphic){
    m_vSliderThumbGraphic = sliderThumbGraphic;
    
    // Ensure all necessary components are available before proceeding with updates.
    // m_vSlider and m_vSliderListener should have been created in CGUITextBox::Create().
    // m_vSliderThumbGraphic must be valid to get its size.
    if (m_vSlider.get() && m_vSliderListener.get() && m_vSliderThumbGraphic.get()) {
        // Cast listener to its specific type to access SetPlane or GetMinSize if needed.
        CGUINormalSliderListener* pSliderListener = static_cast<CGUINormalSliderListener*>(m_vSliderListener.get());
        
        // This call is important: it tells the listener the actual dimensions of the graphic,
        // which might be used for its own internal logic or for drawing.
        m_vSliderListener->SetMinSizeFromGraphic(m_vSliderThumbGraphic.get()); 

        // TODO: (As per your original code) Do the real graphic setup here.
        // This typically involves associating the graphic with the listener or slider for rendering.
        pSliderListener->SetPlane(m_vSliderThumbGraphic);

        // Recalculate and re-set slider's movement RECT ---
        // This ensures the slider's operational boundaries (m_rcRect) match the actual thumb graphic.
        int actualThumbWidth = 0;
        int actualThumbHeight = 0;
        
        // Get the true size directly from the graphic now that it's successfully set.
        m_vSliderThumbGraphic->GetSize(actualThumbWidth, actualThumbHeight); 

        // These constants represent the actual dimensions of the scrollbar strip (the thumb).
        m_nSliderStripWidth = actualThumbWidth;
        m_nSliderStripHeight = actualThumbHeight;

        RECT sliderMovementRect;
        // Start with the full textbox area, relative to the slider's own (m_nX, m_nY) origin.
        // The textbox size minus margins
        int effectiveWidth = m_nWidth - (m_nMarginX * 2); // Consider horizontal margins
        int effectiveHeight = m_nHeight - (m_nMarginY * 2); // Consider vertical margins
        // Also account for footer height for the main scrollable area
        effectiveHeight -= m_nFooterHeight;

        ::SetRect(&sliderMovementRect, 0, 0, effectiveWidth, effectiveHeight); 

        if (m_sliderMode == VERTICAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
            // For a vertical slider, its track is at the right side of the textbox.
            // Its left boundary is the textbox's total width minus the width of the slider strip itself.
            sliderMovementRect.left = effectiveWidth - m_nSliderStripWidth; 
            sliderMovementRect.right = effectiveWidth; // Its right boundary is the textbox's full width.
            
            int buttonSize = m_nSliderStripWidth;
            sliderMovementRect.top = buttonSize;
            sliderMovementRect.bottom = effectiveHeight - buttonSize;

            if (m_sliderMode == BOTH_SLIDERS) {
                // If both sliders are present, the vertical track needs to stop before the horizontal one.
                sliderMovementRect.bottom -= m_nSliderStripHeight; 
            }
        }
        if (m_sliderMode == HORIZONTAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
            // For a horizontal slider, its track is at the bottom side of the textbox.
            // Its top boundary is the textbox's total height minus the height of the slider strip.
            sliderMovementRect.top = effectiveHeight - m_nSliderStripHeight;
            sliderMovementRect.bottom = effectiveHeight; // Its bottom boundary is the textbox's full height.
            
            if (m_sliderMode == BOTH_SLIDERS) {
                // If both sliders are present, the horizontal track needs to stop before the vertical one.
                sliderMovementRect.right -= m_nSliderStripWidth; 
            }
        }
        // This is the crucial step: apply the newly calculated, precise movement rectangle to the slider.
        m_vSlider->SetRect(&sliderMovementRect);
    }
}

void CGUITextBox::SetArrowGFX(smart_ptr<CPlaneLoader> pv, int upIndex, int downIndex) {
    if (m_vScrollUpButton.get() && m_vScrollDownButton.get() && pv.get()) {
        YTL::smart_ptr<CGUITextBox> this_ptr(this, false); // Temp smart_ptr for 'this'

        // Instantiate listeners if not already done
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

        // Now set the plane loaders on the listeners.
        // CGUINormalButtonListener has a SetPlaneLoader, so we just call that.
        CGUINormalButtonListener* upListener = static_cast<CGUINormalButtonListener*>(m_vScrollUpButtonListener.get());
        CGUINormalButtonListener* downListener = static_cast<CGUINormalButtonListener*>(m_vScrollDownButtonListener.get());
		pv->SetColorKey(ISurface::makeRGB(255, 255, 255, 0)); // gfx have while alpha value offset
        upListener->SetPlaneLoader(pv, upIndex);
        downListener->SetPlaneLoader(pv, downIndex);

		int buttonWidth = 0;
		int buttonHeight = 0;
		CPlane upPlane = pv->GetPlane(upIndex); // Get graphic for the up button
		upPlane->GetSize(buttonWidth, buttonHeight); // Get its actual size
		int arrowButtonHeight = buttonHeight; // Store its actual height

        // Position arrow buttons considering margins and potential footer height
		m_vScrollUpButton->SetXY(m_nX + m_nWidth - m_nSliderStripWidth - m_nMarginX, m_nY + m_nMarginY);
        // The down button should be above the footer if it exists.
        m_vScrollDownButton->SetXY(m_nX + m_nWidth - m_nSliderStripWidth - m_nMarginX, m_nY + m_nHeight - arrowButtonHeight - m_nMarginY - m_nFooterHeight);
    }
}

void CGUITextBox::SetSliderLoader(string folder, string path){
    //m_vSliderThumbGraphic = loader.GetPlane(0);
    //if (m_vSlider.get() && m_vSliderListener.get() && m_vSliderThumbGraphic.get()) {
        CGUINormalSliderListener* pSliderListener = static_cast<CGUINormalSliderListener*>(m_vSliderListener.get());
        
		CPlaneLoader m_vPlaneScrollLoader;
		m_vPlaneScrollLoader.SetReadDir(folder);  // Base directory
		if (m_vPlaneScrollLoader.Set(path, false) != 0) {  // Relative to SetReadDir
			OutputDebugStringA("Error: Failed to load\n");
		}

		// TODO: do the real gfx here
		CPlane pln = m_vPlaneScrollLoader.GetPlane(5);
		//POINT pos = m_vPlaneScrollLoader.GetXY(5);
		//pln->SetPos(pos); // rendering pos
		smart_ptr<ISurface> plnPtr(pln.get(), false); // no ownership
		m_vSliderThumbGraphic = plnPtr;
		pSliderListener->SetPlane(m_vSliderThumbGraphic);

		//smart_ptr<CPlaneLoader> smartLdr(loader.get(), false); // no ownership
		//pSliderListener->SetPlaneLoader(smartLdr);
    //}
}


void CGUITextBox::Create(int x, int y, int width, int height, SliderMode mode) {
    SetXY(x, y);

    m_nWidth = width;
    m_nHeight = height;
    m_sliderMode = mode;

    // Adjust clipping rectangle to account for margins. Footer height will be factored in UpdateTextPlane/OnSimpleDraw
    ::SetRect(&m_rcTextBoxClip, x + m_nMarginX, y + m_nMarginY,
                             x + width - m_nMarginX, y + height - m_nMarginY);

    m_vTextFastPlane = new CTextFastPlane;
    m_vTextPlane = CPlane(m_vTextFastPlane);

    // Initialize title and footer text planes
    m_vTitleTextFastPlane = new CTextFastPlane;
    m_vTitleTextPlane = CPlane(m_vTitleTextFastPlane);

    m_vFooterTextFastPlane = new CTextFastPlane;
    m_vFooterTextPlane = CPlane(m_vFooterTextFastPlane);

    if (m_vTextFastPlane != NULL) {
        m_vTextFastPlane->GetFont()->SetColor(m_textColor);
        m_vTextFastPlane->GetFont()->SetSize(16);
    }

    // Set default font properties for title (bold, black)
    if (m_vTitleTextFastPlane != NULL) {
        CFont* titleFont = m_vTitleTextFastPlane->GetFont();
        titleFont->SetColor(ISurface::makeRGB(0, 0, 0, 0)); // Black, opaque
        titleFont->SetSize(18); // Slightly larger
        titleFont->SetWeight(FW_BOLD); // Bold
    }

    // Set default font properties for footer (same as main text, but will be right aligned)
    if (m_vFooterTextFastPlane != NULL) {
        CFont* footerFont = m_vFooterTextFastPlane->GetFont();
        footerFont->SetColor(m_textColor); // Same as main text initially
        footerFont->SetSize(16); // Same size as main text initially
    }

    if (m_sliderMode != NO_SLIDER) {
        m_vSlider = YTL::smart_ptr<CGUISlider>(new CGUISlider());
        m_vSliderListener = YTL::smart_ptr<CGUITextBoxSliderListener>(new CGUITextBoxSliderListener());

        YTL::smart_ptr<CGUITextBox> this_ptr(this, false);
        m_vSliderListener->SetTextBox(this_ptr);

        m_vSlider->SetEvent(YTL::smart_ptr_static_cast<CGUISliderEventListener>(m_vSliderListener));
        m_vSlider->SetType(m_sliderMode);
        
        // Position slider relative to textbox, adjusted for margins. Footer height will be dynamic.
        m_vSlider->SetXY(m_nX + m_nMarginX, m_nY + m_nMarginY);

        // ONLY initialize the CGUIButton objects here.
        // Their listeners and events will be set in SetArrowGFX.
        if (m_sliderMode == VERTICAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
            m_vScrollUpButton = YTL::smart_ptr<CGUIButton>(new CGUIButton());
            m_vScrollDownButton = YTL::smart_ptr<CGUIButton>(new CGUIButton());
        }

        RECT tempSliderRect;
        const int defaultSliderStripSize = 15;
        
        // Calculate slider bounds based on effective (margin-adjusted) textbox area.
        // Footer height will be subtracted from effective height when the slider rect is finalized.
        int effectiveWidth = m_nWidth - (m_nMarginX * 2);
        int effectiveHeight = m_nHeight - (m_nMarginY * 2);

        if (m_sliderMode == VERTICAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
            ::SetRect(&tempSliderRect, effectiveWidth - defaultSliderStripSize, 0 + defaultSliderStripSize, effectiveWidth, effectiveHeight - defaultSliderStripSize);
            
            // Adjust button bounds for margins
            RECT upButtonBounds = { effectiveWidth - defaultSliderStripSize, 0, effectiveWidth, defaultSliderStripSize };
            m_vScrollUpButton->SetBounds(upButtonBounds);
            RECT downButtonBounds = { effectiveWidth - defaultSliderStripSize, effectiveHeight - defaultSliderStripSize, effectiveWidth, effectiveHeight };
            m_vScrollDownButton->SetBounds(downButtonBounds);
        } else if (m_sliderMode == HORIZONTAL_SLIDER) {
            ::SetRect(&tempSliderRect, 0, effectiveHeight - defaultSliderStripSize, effectiveWidth, effectiveHeight);
        }
        
        // This initial SetRect might be slightly off if footer is present, but UpdateTextPlane will correct it.
        m_vSlider->SetRect(&tempSliderRect);
    }
    // Perform initial updates for title/footer and main text to ensure correct layout on creation
    UpdateTitleAndFooterPlanes();
    UpdateTextPlane();
}

void CGUITextBox::SetText(const string& text) {
    if (m_strCurrentText != text) {
        m_strCurrentText = text;
        UpdateTextPlane(); // This will recalculate content size and scroll ranges
    }
}

string CGUITextBox::GetText() const {
    return m_strCurrentText;
}

void CGUITextBox::SetFont(YTL::smart_ptr<CFont> font) {
    if (m_vTextFastPlane != NULL) {
        *(m_vTextFastPlane->GetFont()) = *font; // Copy font properties
        UpdateTextPlane();
    }
}

CFont* CGUITextBox::GetFont() {
    return m_vTextFastPlane != NULL ? m_vTextFastPlane->GetFont() : NULL;
}

void CGUITextBox::SetTextColor(ISurfaceRGB color) {
    m_textColor = color; // Direct assignment of the DWORD color value
    if (m_vTextFastPlane != NULL) {
        m_vTextFastPlane->GetFont()->SetColor(m_textColor);
        UpdateTextPlane();
    }
    // Also update footer color to match if not explicitly set separately later
    if (m_vFooterTextFastPlane != NULL) {
        m_vFooterTextFastPlane->GetFont()->SetColor(m_textColor);
        UpdateTitleAndFooterPlanes(); // Re-render footer with new color
    }
}

void CGUITextBox::SetTextOffset(int x, int y) {
    m_nTextOffsetX = x;
    m_nTextOffsetY = y;
    // No need to call UpdateTextPlane immediately, as this is a drawing offset.
    // Text drawing will use these offsets.
}

void CGUITextBox::SetBackgroundPlane(YTL::smart_ptr<ISurface> pv) {
    m_vBackgroundPlane = pv;
}

void CGUITextBox::UpdateTitleAndFooterPlanes() {
    // Render Title Text
    if (m_vTitleTextFastPlane != NULL) {
        if (!m_strTitleText.empty()) {
            CFont* titleFont = m_vTitleTextFastPlane->GetFont();
            // Available width for title is total width minus horizontal margins
            int titleAvailableWidth = m_nWidth - (m_nMarginX * 2);
            if (m_sliderMode == VERTICAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
                titleAvailableWidth -= m_nSliderStripWidth; // Reduce if vertical slider is present
            }
            titleAvailableWidth = max(0, titleAvailableWidth);

            std::string wrappedTitle = WrapText(m_strTitleText, titleAvailableWidth);
            titleFont->SetText(wrappedTitle);
            m_vTitleTextFastPlane->SetTextPos(0, 0); // Position within its own plane
            m_vTitleTextFastPlane->UpdateText(); // Render
        } else {
            m_vTitleTextFastPlane->GetFont()->SetText(""); // Clear text
            m_vTitleTextFastPlane->UpdateText(); // Clear rendering
        }
    }

    // Render Footer Text
    if (m_vFooterTextFastPlane != NULL) {
        if (!m_strFooterText.empty()) {
            CFont* footerFont = m_vFooterTextFastPlane->GetFont();
            // Available width for footer is total width minus horizontal margins
            int footerAvailableWidth = m_nWidth - (m_nMarginX * 2);
            if (m_sliderMode == VERTICAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
                footerAvailableWidth -= m_nSliderStripWidth; // Reduce if vertical slider is present
            }
            footerAvailableWidth = max(0, footerAvailableWidth);

            // Footer is right-aligned, so we don't wrap it with WrapText for now,
            // but just set the text and calculate its dimensions to know its height.
            // If wrapping is desired, it needs custom logic for right alignment.
            footerFont->SetText(m_strFooterText);
            m_vFooterTextFastPlane->SetTextPos(0, 0); // Position within its own plane for calculation
            m_vFooterTextFastPlane->UpdateText(); // Render

            int tempFooterWidth, tempFooterHeight;
            m_vFooterTextFastPlane->GetSize(tempFooterWidth, tempFooterHeight);
            m_nFooterHeight = tempFooterHeight; // Store rendered height
        } else {
            m_vFooterTextFastPlane->GetFont()->SetText(""); // Clear text
            m_vFooterTextFastPlane->UpdateText(); // Clear rendering
            m_nFooterHeight = 0; // No footer, so height is 0
        }
    }
}


void CGUITextBox::UpdateTextPlane() {
    if (m_vTextFastPlane != NULL) {
        // Calculate available width for main text, considering textbox width, margins, and slider.
        int textAvailableWidth = m_nWidth - (m_nMarginX * 2);
        if (m_sliderMode == VERTICAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
            textAvailableWidth -= m_nSliderStripWidth;
        }
        textAvailableWidth = max(0, textAvailableWidth);

        std::string wrappedContent = WrapText(m_strCurrentText, textAvailableWidth);
        m_vTextFastPlane->GetFont()->SetText(wrappedContent);
        m_vTextFastPlane->SetTextPos(0, 0); // Text position relative to its own plane
        m_vTextFastPlane->UpdateText(); // Render without anti-aliasing

        CalculateVisibleContentSize(); // Recalculate content size after text update
    }

    if (m_vSlider.get()) {
        // Adjust slider's total scrollable range based on content size vs effective textbox display size.
        // The effective display size for main text is the textbox size minus margins, title height, and footer height.
        int effectiveDisplayWidth = m_nWidth - (m_nMarginX * 2);
        int effectiveDisplayHeight = m_nHeight - (m_nMarginY * 2);

        int titleRenderedHeight = 0;
        if (!m_strTitleText.empty() && m_vTitleTextFastPlane) {
            int w, h;
            m_vTitleTextFastPlane->GetSize(w, h);
            titleRenderedHeight = h;
        }
        effectiveDisplayHeight -= titleRenderedHeight; // Subtract title height
        effectiveDisplayHeight -= m_nFooterHeight; // Subtract footer height

        // Clamp effectiveDisplayHeight/Width to ensure positive values if text box is too small
        effectiveDisplayWidth = max(0, effectiveDisplayWidth);
        effectiveDisplayHeight = max(0, effectiveDisplayHeight);

        int itemsX = 1;
        int itemsY = 1;

        if (m_sliderMode == HORIZONTAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
            itemsX = max(1, m_nContentWidth - effectiveDisplayWidth + 1); // Content width already includes title width
        }
        if (m_sliderMode == VERTICAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
            itemsY = max(1, m_nContentHeight - effectiveDisplayHeight + 1); // Content height already includes title height
        }
        m_vSlider->SetItemNum(itemsX, itemsY);

        // Reset scroll position if content shrinks, to avoid out-of-bounds scrolling
        m_nScrollX = max(0, min(m_nScrollX, max(0, m_nContentWidth - effectiveDisplayWidth)));
        m_nScrollY = max(0, min(m_nScrollY, max(0, m_nContentHeight - effectiveDisplayHeight)));

        // Update slider's visual position to match the clamped scroll
        m_vSlider->SetSelectedItem(m_nScrollX, m_nScrollY);

        // Re-position slider based on updated effective height for its track
        RECT sliderMovementRect;
        int sliderTrackWidth = m_nSliderStripWidth; // Use actual thumb width
        int sliderTrackHeight = m_nSliderStripHeight; // Use actual thumb height

        // Account for margins and footer in slider's vertical range
        int sliderAreaTop = m_nMarginY;
        int sliderAreaBottom = m_nHeight - m_nMarginY - m_nFooterHeight;

        if (m_sliderMode == VERTICAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
            // Vertical slider track: right edge of effective content area
            sliderMovementRect.left = m_nWidth - m_nMarginX - sliderTrackWidth;
            sliderMovementRect.right = m_nWidth - m_nMarginX;
            sliderMovementRect.top = sliderAreaTop + sliderTrackHeight; // Below top arrow
            sliderMovementRect.bottom = sliderAreaBottom - sliderTrackHeight; // Above bottom arrow

            if (m_sliderMode == BOTH_SLIDERS) {
                sliderMovementRect.bottom -= sliderTrackHeight; // Reduce if horizontal slider also present
            }
        }
        // TODO: Implement similar logic for horizontal slider positioning if needed
        // based on effectiveDisplayWidth and effectiveDisplayHeight.

        m_vSlider->SetRect(&sliderMovementRect); // Apply the new slider track rect
    }
    // Removed: SetArrowGFX(m_vScrollUpButton->GetPlaneLoader(), 0, 0); // This was the problematic line for buttons.
    // Button positioning should be handled elsewhere if dynamic updates are needed,
    // or assume SetArrowGFX handles initial setup only.
}

void CGUITextBox::CalculateVisibleContentSize() {
    int mainTextWidth = 0;
    int mainTextHeight = 0;
    if (m_vTextFastPlane != NULL) {
        m_vTextFastPlane->GetSize(mainTextWidth, mainTextHeight);
    }

    int titleRenderedHeight = 0;
    int titleRenderedWidth = 0;
    if (!m_strTitleText.empty() && m_vTitleTextFastPlane != NULL) {
        m_vTitleTextFastPlane->GetSize(titleRenderedWidth, titleRenderedHeight);
    }

    // The total content width is determined by the widest of the main text or title.
    // Assuming title is not meant to scroll horizontally independently.
    m_nContentWidth = max(mainTextWidth, titleRenderedWidth);

    // The total content height is the sum of title height and main text height.
    m_nContentHeight = titleRenderedHeight + mainTextHeight;
}

LRESULT CGUITextBox::OnSimpleMove(ISurface* lp) {
    LRESULT result = 0;

    // Calculate effective height for content to determine if vertical scrolling is needed.
    // This is the available height for *main text*, after margins, title, and footer.
    int effectiveMainTextDisplayHeight = m_nHeight - (m_nMarginY * 2);
    int titleRenderedHeight = 0;
    if (!m_strTitleText.empty() && m_vTitleTextFastPlane) {
        int w, h;
        m_vTitleTextFastPlane->GetSize(w, h);
        titleRenderedHeight = h;
    }
    effectiveMainTextDisplayHeight -= titleRenderedHeight;
    effectiveMainTextDisplayHeight -= m_nFooterHeight;
    effectiveMainTextDisplayHeight = max(0, effectiveMainTextDisplayHeight); // Ensure non-negative

    bool needsVerticalScroll = m_nContentHeight > effectiveMainTextDisplayHeight;

    // First, process mouse input for the slider
    if (m_vSlider.get() && m_pvMouse.get()) {
        result |= m_vSlider->OnSimpleMove(lp); // Process slider's own movement logic

        // Update textbox scroll position based on slider's selected item
        int sliderSelectedItemX, sliderSelectedItemY;
        // Correct usage of GetSelectedItem, it takes two references
        m_vSlider->GetSelectedItem(sliderSelectedItemX, sliderSelectedItemY);

        // The slider's selected item directly represents the pixel scroll position.
        int targetScrollX = sliderSelectedItemX;
        int targetScrollY = sliderSelectedItemY;

        // Clamp target scroll positions to valid range
        int effectiveDisplayWidth = m_nWidth - (m_nMarginX * 2);
        if (m_sliderMode == VERTICAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
            effectiveDisplayWidth -= m_nSliderStripWidth; // If vertical slider present
        }
        effectiveDisplayWidth = max(0, effectiveDisplayWidth);

        int maxScrollX = max(0, m_nContentWidth - effectiveDisplayWidth);
        int maxScrollY = max(0, m_nContentHeight - effectiveMainTextDisplayHeight);
        targetScrollX = max(0, min(targetScrollX, maxScrollX));
        targetScrollY = max(0, min(targetScrollY, maxScrollY));

        // Mouse wheel operation: Check if mouse is inside the textbox
        int mouseX, mouseY;
        GetMouse()->GetXY(mouseX, mouseY); // Get current mouse coordinates
        POINT currentMousePos = { mouseX, mouseY };

		// Only process events for the scroll buttons and slider if they are displayed
		if (m_sliderMode == VERTICAL_SLIDER && needsVerticalScroll) {
			// Process mouse input for scroll buttons first
			if (m_vScrollUpButton.get()) {
				result |= m_vScrollUpButton->OnSimpleMove(lp);
			}
			if (m_vScrollDownButton.get()) {
				result |= m_vScrollDownButton->OnSimpleMove(lp);
			}

			// Mouse wheel operation (needs to be happen there because it shall work when mouse is anywhere in the box, not only at slider event)
            // Adjust the clipping rect for mouse wheel check to include margins if desired,
            // or just check against the content area within margins.
            RECT mouseCheckRect = {m_nX, m_nY, m_nX + m_nWidth, m_nY + m_nHeight}; // Use full textbox area for wheel
			if (::PtInRect(&mouseCheckRect, currentMousePos)) {
				CGUINormalSliderListener* pSliderListener = static_cast<CGUINormalSliderListener*>(m_vSliderListener.get());
				int sx, sy;
				pSliderListener->GetMinSize(sx, sy); // Get the scroll step amount (slider thumb height)

				int scrollDeltaY = 0; // Initialize scroll delta

				// Determine scroll direction and amount
				if (GetMouse()->IsWheelUp()) {
					scrollDeltaY = -sy; // Scroll up (negative Y)
				} else if (GetMouse()->IsWheelDown()) {
					scrollDeltaY = sy;	// Scroll down (positive Y)
				}

				// If a scroll event occurred, apply it
				if (scrollDeltaY != 0) {
					ScrollContent(0, scrollDeltaY);
					result |= 1; // Indicate a change has occurred
				}
			}
		}

        // Only update if scroll position actually changes to avoid unnecessary re-draws
        if (targetScrollX != m_nScrollX || targetScrollY != m_nScrollY) {
            m_nScrollX = targetScrollX;
            m_nScrollY = targetScrollY;
            // Indicate a change has occurred, potentially triggering a redraw
            result |= 1;
        }
    }
    return result;
}


LRESULT CGUITextBox::OnSimpleDraw(ISurface* lp) {
    // Calculate global drawing position for the textbox
    int drawX = m_nX + m_nXOffset;
    int drawY = m_nY + m_nYOffset;

    // Draw background plane if it has been set and is valid
    if (!m_vBackgroundPlane.isNull()) {
        SIZE destSize = { m_nWidth, m_nHeight };
        // Clipping rect for background uses the full textbox area
        RECT backgroundClipRect = {drawX, drawY, drawX + m_nWidth, drawY + m_nHeight};
        lp->BltNatural(m_vBackgroundPlane.get(), drawX, drawY, &destSize, NULL, &backgroundClipRect, 0);
    } else {
        // If no background plane is set, you might want to draw a solid color background or simply do nothing.
    }

    // Calculate effective height for the main text and title rendering area
    // This is the textbox height minus the top/bottom margins and the footer height.
    int mainContentDisplayHeight = m_nHeight - (m_nMarginY * 2) - m_nFooterHeight;
    mainContentDisplayHeight = max(0, mainContentDisplayHeight);

    // Calculate clipping rectangle for scrolling content (title + main text)
    RECT contentClipRect = {
        drawX + m_nMarginX,
        drawY + m_nMarginY,
        drawX + m_nWidth - m_nMarginX,
        drawY + m_nHeight - m_nMarginY - m_nFooterHeight
    };
    // Adjust clip rect if vertical slider is present
    if (m_sliderMode == VERTICAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
        contentClipRect.right -= m_nSliderStripWidth;
    }
    // Ensure valid clipping rect
    if (contentClipRect.left > contentClipRect.right) contentClipRect.right = contentClipRect.left;
    if (contentClipRect.top > contentClipRect.bottom) contentClipRect.bottom = contentClipRect.top;


    // Draw the Title Plane (if set)
    int titleRenderedHeight = 0;
    if (!m_strTitleText.empty() && m_vTitleTextPlane.get()) {
        int w, h;
        m_vTitleTextPlane->GetSize(w, h); // Get size from the plane itself
        titleRenderedHeight = h;

        RECT titleSrcRect = {0, 0, w, h}; // Entire title plane
        // Title position is relative to textbox origin + margins, and scrolls with content.
        lp->BltNatural(m_vTitleTextPlane.get(),
                         drawX + m_nMarginX + m_nTextOffsetX - m_nScrollX, // X pos with margin, offset, and scroll
                         drawY + m_nMarginY + m_nTextOffsetY - m_nScrollY, // Y pos with margin, offset, and scroll
                         NULL,
                         &titleSrcRect,
                         &contentClipRect, // Clip to main content area
                         0);
    }

    // Draw the main text plane
    if (m_vTextPlane.get()) {
        RECT srcRect;
        // Define the "window" into the m_vTextPlane (the entire rendered text content)
        // This window starts after the title (if any)
        int mainTextSrcY = m_nScrollY - titleRenderedHeight; // Adjust scroll for title height
        mainTextSrcY = max(0, mainTextSrcY); // Ensure it doesn't go negative

        // Get the actual rendered height and width of the main text plane
        int mainTextPlaneRenderedWidth = 0;
        int mainTextPlaneRenderedHeight = 0;
        m_vTextPlane->GetSize(mainTextPlaneRenderedWidth, mainTextPlaneRenderedHeight);

        ::SetRect(&srcRect, m_nScrollX, mainTextSrcY,
                                     m_nScrollX + (contentClipRect.right - contentClipRect.left), // Width of display area
                                     mainTextSrcY + (contentClipRect.bottom - contentClipRect.top)); // Height of display area

        // Clamp the source rectangle to the actual dimensions of the rendered text content
        srcRect.right = min(srcRect.right, mainTextPlaneRenderedWidth);
        srcRect.bottom = min(srcRect.bottom, mainTextPlaneRenderedHeight); // Use actual rendered height of m_vTextPlane

        // Main text drawing position: after margins, after title (if present), plus internal offsets.
        lp->BltNatural(m_vTextPlane.get(),
                         drawX + m_nTextOffsetX + m_nMarginX,
                         drawY + m_nTextOffsetY + m_nMarginY + titleRenderedHeight, // Shift down by title height
                         NULL,
                         &srcRect,
                         &contentClipRect, // Clip to main content area
                         0);
    }

    // Draw the Footer Plane (if set) - static at the bottom, right-aligned
    if (!m_strFooterText.empty() && m_vFooterTextPlane.get()) {
        int footerWidth, footerHeight;
        m_vFooterTextPlane->GetSize(footerWidth, footerHeight);

        // Footer draw X: right-aligned within the textbox, accounting for right margin and potential slider width
        int footerDrawX = drawX + m_nWidth - m_nMarginX - footerWidth;
        if (m_sliderMode == VERTICAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
            footerDrawX -= m_nSliderStripWidth;
        }
        
        // Footer draw Y: at the bottom of the textbox, accounting for bottom margin.
        int footerDrawY = drawY + m_nHeight - m_nMarginY - footerHeight;

        RECT footerSrcRect = {0, 0, footerWidth, footerHeight};
        RECT footerClipRect = {
            drawX + m_nMarginX,
            drawY + m_nHeight - m_nMarginY - footerHeight, // Clip from its start Y position
            drawX + m_nWidth - m_nMarginX,
            drawY + m_nHeight - m_nMarginY // To the bottom margin
        };
        // Adjust clip rect if vertical slider is present
        if (m_sliderMode == VERTICAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
            footerClipRect.right -= m_nSliderStripWidth;
        }
        // Ensure valid clipping rect
        if (footerClipRect.left > footerClipRect.right) footerClipRect.right = footerClipRect.left;
        if (footerClipRect.top > footerClipRect.bottom) footerClipRect.bottom = footerClipRect.top;


        lp->BltNatural(m_vFooterTextPlane.get(),
                         footerDrawX,
                         footerDrawY,
                         NULL,
                         &footerSrcRect,
                         &footerClipRect,
                         0);
    }

    // Draw the scroll buttons and slider only if they are needed
    int effectiveMainTextDisplayHeightForSlider = m_nHeight - (m_nMarginY * 2) - m_nFooterHeight;
    effectiveMainTextDisplayHeightForSlider = max(0, effectiveMainTextDisplayHeightForSlider); // Ensure non-negative

    bool needsVerticalScroll = m_nContentHeight > effectiveMainTextDisplayHeightForSlider;
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
    // Reset base IGUIParts state
    IGUIParts::Reset();

    // Reset textbox specific state
    m_strCurrentText = "";
    m_strTitleText = ""; // Reset title text
    m_strFooterText = ""; // Reset footer text
    m_nScrollX = 0;
    m_nScrollY = 0;
    m_nContentWidth = 0;
    m_nContentHeight = 0;
    m_nMarginX = 0; // Reset margins to default
    m_nMarginY = 0; // Reset margins to default
    m_nFooterHeight = 0; // Reset footer height

    // Reset internal text plane and slider
    if (m_vTextFastPlane != NULL) {
        m_vTextFastPlane->GetFont()->SetText("");
        m_vTextFastPlane->UpdateTextAA();
    }
    if (m_vTitleTextFastPlane != NULL) {
        m_vTitleTextFastPlane->GetFont()->SetText("");
        m_vTitleTextFastPlane->UpdateText();
    }
    if (m_vFooterTextFastPlane != NULL) {
        m_vFooterTextFastPlane->GetFont()->SetText("");
        m_vFooterTextFastPlane->UpdateText();
    }

    if (m_vSlider.get()) {
        m_vSlider->Reset();
    }

	// Reset scroll buttons
    if (m_vScrollUpButton.get()) {
        m_vScrollUpButton->Reset();
    }
    if (m_vScrollDownButton.get()) {
        m_vScrollDownButton->Reset();
    }
}

void CGUITextBox::SetMouse(YTL::smart_ptr<CFixMouse> pv) {
    IGUIParts::SetMouse(pv); // Pass mouse to base
    if (m_vSlider.get()) {
        m_vSlider->SetMouse(pv); // Also pass mouse to slider
    }

	// Pass mouse to scroll buttons
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

    // Clamp scroll positions using min/max macros (or std:: equivalents if resolved)
    // Ensure scroll position does not go beyond the content boundaries.
    // The maximum scroll is (content_size - effective_textbox_display_size for main text).
    int effectiveDisplayWidth = m_nWidth - (m_nMarginX * 2);
    if (m_sliderMode == VERTICAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
        effectiveDisplayWidth -= m_nSliderStripWidth; // If vertical slider present
    }
    effectiveDisplayWidth = max(0, effectiveDisplayWidth);

    int titleRenderedHeight = 0;
    if (!m_strTitleText.empty() && m_vTitleTextFastPlane) {
        int w, h;
        m_vTitleTextFastPlane->GetSize(w, h);
        titleRenderedHeight = h;
    }
    int effectiveMainTextDisplayHeight = m_nHeight - (m_nMarginY * 2) - titleRenderedHeight - m_nFooterHeight;
    effectiveMainTextDisplayHeight = max(0, effectiveMainTextDisplayHeight);

    m_nScrollX = max(0, min(m_nScrollX, max(0, m_nContentWidth - effectiveDisplayWidth)));
    m_nScrollY = max(0, min(m_nScrollY, max(0, m_nContentHeight - effectiveMainTextDisplayHeight)));

    // Update slider position if scroll changed
    if (oldScrollX != m_nScrollX || oldScrollY != m_nScrollY) {
        if (m_vSlider.get()) {
            // Map current scroll position back to slider item
            // Since SetItemNum now sets items to pixel count for fine control,
            // we can directly set selected item to current scroll position.
			m_vSlider->SetSelectedItem(m_nScrollX, m_nScrollY);
        }
    }
}

std::string CGUITextBox::TrimLeadingSpaces(const std::string& s) {
	size_t first = s.find_first_not_of(" \t\n\r\f\v");
	if (std::string::npos == first) {
		return s; // Or "" if you want empty string for all spaces
	}
	return s.substr(first);
}

std::string yaneuraoGameSDK3rd::Draw::CGUITextBox::WrapText(const std::string& rawText, int availableWidth) {
    if (rawText.empty() || availableWidth <= 0) {
        return "";
    }

    std::string finalWrappedText; // This will accumulate all wrapped lines, separated by '\n'
    CFont* font = m_vTextFastPlane->GetFont(); // Get the font object for measurement

    // Save current font text, as SetText will modify internal m_String.
    // This is crucial if m_Font is a shared resource or used for other text simultaneously.
    std::string originalFontText = font->GetText(); 

    // --- Pass 1: Split rawText by existing newlines ('\n') ---
    size_t lineStart = 0;
    
    while (lineStart < rawText.length()) {
        size_t lineEnd = rawText.find('\n', lineStart);
        std::string segmentToWrap;
        bool endsWithExplicitNewline = false;

        if (lineEnd == std::string::npos) {
            // No more explicit newlines, this is the last "paragraph"
            segmentToWrap = rawText.substr(lineStart);
            lineStart = rawText.length(); // Advance to end to exit loop
        } else {
            // Found an explicit newline. Process the segment up to it.
            segmentToWrap = rawText.substr(lineStart, lineEnd - lineStart);
            lineStart = lineEnd + 1; // Move cursor past the newline character
            endsWithExplicitNewline = true; // Mark that this segment was terminated by an explicit newline
        }

        // --- Pass 2: Apply word wrapping to the current 'segmentToWrap' ---
        std::string wrappedSegmentResult; // Result of wrapping the current paragraph/segment
        std::string currentLineInSegment; // Builds one line within the current paragraph
        size_t wordCurrentPos = 0;
        int tempWidth, tempHeight; // For GetSize measurements

        while (wordCurrentPos < segmentToWrap.length()) {
            // Find the start of the next word (skip leading spaces/tabs within this segment)
            size_t wordSegmentStart = segmentToWrap.find_first_not_of(" \t\r\f\v", wordCurrentPos);
            if (std::string::npos == wordSegmentStart) {
                break; // No more non-whitespace characters in this segment
            }

            // Find the end of the current word (first whitespace after wordSegmentStart)
            size_t wordSegmentEnd = segmentToWrap.find_first_of(" \t\r\f\v", wordSegmentStart);
            std::string word;
            if (std::string::npos == wordSegmentEnd) { // Last word in this segment
                word = segmentToWrap.substr(wordSegmentStart);
                wordCurrentPos = segmentToWrap.length(); // Advance to end of segment
            } else {
                word = segmentToWrap.substr(wordSegmentStart, wordSegmentEnd - wordSegmentStart);
                wordCurrentPos = wordSegmentEnd; // Advance to the space after the word
            }
            
            // Build the test line to measure
            std::string testLine;
            if (!currentLineInSegment.empty()) {
                testLine = currentLineInSegment + " " + word; // Add word with a space
            } else {
                testLine = word; // First word on a new line in this segment
            }

            // Measure the potential new line's width
            font->SetText(testLine);
            font->GetSize(tempWidth, tempHeight);

            // Check if adding this word would exceed the available width
            if (tempWidth > availableWidth && !currentLineInSegment.empty()) {
                // The current line (currentLineInSegment) is full, add it to the result
                wrappedSegmentResult += currentLineInSegment + "\n";
                // Start a new line with the current word
                currentLineInSegment = word; 
            } else {
                // The word fits, append it to the current line in this segment
                currentLineInSegment = testLine;
            }
        }

        // Add the very last line of the current segment (if any text remains)
        if (!currentLineInSegment.empty()) {
            wrappedSegmentResult += currentLineInSegment;
        }

        // Append the wrapped segment to the final result
        finalWrappedText += wrappedSegmentResult;
        
        // Preserve the original explicit newline
        if (endsWithExplicitNewline) {
            finalWrappedText += "\n"; 
        }
    }

    // Restore original font text (important if font is shared or used elsewhere)
    font->SetText(originalFontText); 

    // Optional: Trim a trailing newline if the original rawText didn't end with one,
    // but the wrapping logic might have added one after the last line.
    if (!rawText.empty() && rawText[rawText.length()-1] != '\n' &&
        !finalWrappedText.empty() && finalWrappedText[finalWrappedText.length()-1] == '\n') {
        finalWrappedText.erase(finalWrappedText.length()-1);
    }

    return finalWrappedText;
}

} // namespace Draw
} // namespace yaneuraoGameSDK3rd