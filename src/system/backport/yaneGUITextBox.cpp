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
      m_nMarginY(0)  // Initialize new member
{
    // Default text color. Use ISurface::makeRGB since ISurfaceRGB is a DWORD typedef.
    // Assuming 0 for alpha (fully opaque) for common text rendering, adjust if different blend mode needed.
    m_textColor = ISurface::makeRGB(255, 255, 255, 0); // White, opaque

    // Initialize m_rcTextBoxClip (local to CGUITextBox)
    ::SetRect(&m_rcTextBoxClip, 0, 0, 0, 0);
}

CGUITextBox::~CGUITextBox() {
    // Smart pointers handle deallocation
}

// --- New Margin Methods Implementation ---
void CGUITextBox::SetMargins(int x, int y) {
    m_nMarginX = max(0, x); // Margins shouldn't be negative
    m_nMarginY = max(0, y);
    UpdateTextPlane(); // Recalculate and redraw text with new margins
    // No explicit ResetScrollBars method in this version,
    // so `UpdateTextPlane` (which calls `SetItemNum` on slider) handles range updates.
}

void CGUITextBox::GetMargins(int& x, int& y) const {
    x = m_nMarginX;
    y = m_nMarginY;
}
// --- End New Margin Methods ---

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

        // Position arrow buttons considering margins
		m_vScrollUpButton->SetXY(m_nX + m_nWidth - m_nSliderStripWidth - m_nMarginX, m_nY + m_nMarginY);
        m_vScrollDownButton->SetXY(m_nX + m_nWidth - m_nSliderStripWidth - m_nMarginX, m_nY + m_nHeight - arrowButtonHeight - m_nMarginY);
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

    // Adjust clipping rectangle to account for margins
    ::SetRect(&m_rcTextBoxClip, x + m_nMarginX, y + m_nMarginY,
                             x + width - m_nMarginX, y + height - m_nMarginY);

    m_vTextFastPlane = new CTextFastPlane;
    m_vTextPlane = CPlane(m_vTextFastPlane);

    if (m_vTextFastPlane != NULL) {
        m_vTextFastPlane->GetFont()->SetColor(m_textColor);
        m_vTextFastPlane->GetFont()->SetSize(16);
    }

    if (m_sliderMode != NO_SLIDER) {
        m_vSlider = YTL::smart_ptr<CGUISlider>(new CGUISlider());
        m_vSliderListener = YTL::smart_ptr<CGUITextBoxSliderListener>(new CGUITextBoxSliderListener());

        YTL::smart_ptr<CGUITextBox> this_ptr(this, false);
        m_vSliderListener->SetTextBox(this_ptr);

        m_vSlider->SetEvent(YTL::smart_ptr_static_cast<CGUISliderEventListener>(m_vSliderListener));
        m_vSlider->SetType(m_sliderMode);
        
        // Position slider relative to textbox, adjusted for margins
        m_vSlider->SetXY(m_nX + m_nMarginX, m_nY + m_nMarginY);

        // ONLY initialize the CGUIButton objects here.
        // Their listeners and events will be set in SetArrowGFX.
        if (m_sliderMode == VERTICAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
            m_vScrollUpButton = YTL::smart_ptr<CGUIButton>(new CGUIButton());
            m_vScrollDownButton = YTL::smart_ptr<CGUIButton>(new CGUIButton());
        }

        RECT tempSliderRect;
        const int defaultSliderStripSize = 15;
        
        // Calculate slider bounds based on effective (margin-adjusted) textbox area
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

void CGUITextBox::UpdateTextPlane() {
    if (m_vTextFastPlane != NULL) {
        // Calculate available width for text, considering both textbox width and margins.
        int textAvailableWidth = m_nWidth - (m_nMarginX * 2);

        // If a vertical slider is present, reduce the width available for text further.
        if (m_sliderMode == VERTICAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
            textAvailableWidth -= m_nSliderStripWidth;
        }
        // Ensure minimum width for text if slider takes up too much space.
        if (textAvailableWidth < 0) textAvailableWidth = 0;

        std::string wrappedContent = WrapText(m_strCurrentText, textAvailableWidth);
        m_vTextFastPlane->GetFont()->SetText(wrappedContent);
        m_vTextFastPlane->SetTextPos(0, 0); // Text position relative to its own plane
        m_vTextFastPlane->UpdateText(); // Render without anti-aliasing

        CalculateVisibleContentSize(); // Recalculate content size after text update
    }

    if (m_vSlider.get()) {
        // Adjust slider's total scrollable range based on content size vs effective textbox display size.
        // The effective display size is the textbox size minus margins.
        int effectiveDisplayWidth = m_nWidth - (m_nMarginX * 2);
        int effectiveDisplayHeight = m_nHeight - (m_nMarginY * 2);

        int itemsX = 1; // Default to 1 if no scroll needed or horizontal not enabled
        int itemsY = 1; // Default to 1 if no scroll needed or vertical not enabled

        // Set item numbers for the slider. If content fits, 1 item. Otherwise,
        // use the difference in pixels as the number of items for fine-grained control.
        if (m_sliderMode == HORIZONTAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
            itemsX = max(1, m_nContentWidth - effectiveDisplayWidth + 1); // Each pixel as an item for fine control
        }
        if (m_sliderMode == VERTICAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
            itemsY = max(1, m_nContentHeight - effectiveDisplayHeight + 1); // Each pixel as an item for fine control
        }
        m_vSlider->SetItemNum(itemsX, itemsY);

        // Reset scroll position if content shrinks, to avoid out-of-bounds scrolling
        m_nScrollX = max(0, min(m_nScrollX, max(0, m_nContentWidth - effectiveDisplayWidth)));
        m_nScrollY = max(0, min(m_nScrollY, max(0, m_nContentHeight - effectiveDisplayHeight)));

        // Update slider's visual position to match the clamped scroll
        // Since SetItemNum now sets items to pixel count for fine control,
        // we can directly set selected item to current scroll position.
        m_vSlider->SetSelectedItem(m_nScrollX, m_nScrollY);
    }
}

void CGUITextBox::CalculateVisibleContentSize() {
    if (m_vTextFastPlane != NULL) {
        m_vTextFastPlane->GetSize(m_nContentWidth, m_nContentHeight);
    } else {
        m_nContentWidth = 0;
        m_nContentHeight = 0;
    }
}

LRESULT CGUITextBox::OnSimpleMove(ISurface* lp) {
    LRESULT result = 0;

    // Calculate effective height for content to determine if vertical scrolling is needed.
    int effectiveDisplayHeight = m_nHeight - (m_nMarginY * 2);
    bool needsVerticalScroll = m_nContentHeight > effectiveDisplayHeight;

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
        int maxScrollX = max(0, m_nContentWidth - effectiveDisplayWidth);
        int maxScrollY = max(0, m_nContentHeight - effectiveDisplayHeight);
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
    if (!m_vBackgroundPlane.isNull()) { // The crash point: ensure m_vBackgroundPlane is valid here.
        // Create a SIZE struct for the destination dimensions
        SIZE destSize = { m_nWidth, m_nHeight };
        
        // Call BltNatural to draw the background.
        // Parameters:
        // 1. Source Surface: The background plane itself (raw pointer from smart_ptr)
        // 2. Destination X: Top-left X coordinate on the target surface 'lp'
        // 3. Destination Y: Top-left Y coordinate on the target surface 'lp'
        // 4. Destination Size: Pointer to SIZE struct for stretching the source to this size
        // 5. Source Rectangle: NULL, meaning use the entire source surface
        // 6. Destination Clip: Pointer to the textbox's clipping rectangle (m_rcTextBoxClip)
        // 7. Base Point: 0 (typically top-left alignment)
        lp->BltNatural(m_vBackgroundPlane.get(), drawX, drawY, &destSize, NULL, &m_rcTextBoxClip, 0);
    } else {
        // If no background plane is set, you might want to draw a solid color background  or simply do nothing.
    }

    // Draw the text plane
    if (m_vTextPlane.get()) {
        RECT srcRect;
        // Define the "window" into the m_vTextPlane (the entire rendered text content)
        // that we want to display. This window is shifted by m_nScrollX and m_nScrollY
        // to implement scrolling.
        ::SetRect(&srcRect, m_nScrollX, m_nScrollY,
                                     m_nScrollX + m_nWidth - (m_nMarginX * 2), // Adjust source width by margins
                                     m_nScrollY + m_nHeight - (m_nMarginY * 2)); // Adjust source height by margins

        // Clamp the source rectangle to the actual dimensions of the rendered text content
        // This prevents reading beyond the bounds of the text plane if content is smaller
        srcRect.right = min(srcRect.right, m_nContentWidth);
        srcRect.bottom = min(srcRect.bottom, m_nContentHeight);
		
        RECT textClippingRect = m_rcTextBoxClip; // Start with the overall textbox clip, already adjusted for margins in Create()

        // Further adjust text clipping if sliders are present
        if (m_sliderMode == VERTICAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
            textClippingRect.right -= m_nSliderStripWidth;
        }
        if (m_sliderMode == HORIZONTAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
            textClippingRect.bottom -= m_nSliderStripHeight;
        }
        // Ensure valid clipping rect in case of very small textbox or large slider
        if (textClippingRect.left > textClippingRect.right) textClippingRect.right = textClippingRect.left;
        if (textClippingRect.top > textClippingRect.bottom) textClippingRect.bottom = textClippingRect.top;

        // Call BltNatural to draw the text content.
        // Parameters:
        // 1. Source Surface: The text content plane (raw pointer from smart_ptr)
        // 2. Destination X: Top-left X coordinate on 'lp' where text starts, with internal offsets and margins
        // 3. Destination Y: Top-left Y coordinate on 'lp' where text starts, with internal offsets and margins
        // 4. Destination Size: NULL, meaning draw at original size as defined by srcRect (no additional scaling)
        // 5. Source Rectangle: Pointer to the srcRect, defining the scrolled view of the text content
        // 6. Destination Clip: Pointer to the effective text clipping rectangle (textClippingRect)
        // 7. Base Point: 0 (top-left alignment)
        lp->BltNatural(m_vTextPlane.get(),
                         drawX + m_nTextOffsetX + m_nMarginX, // Add margin to draw position
                         drawY + m_nTextOffsetY + m_nMarginY, // Add margin to draw position
                         NULL,
                         &srcRect,
                         &textClippingRect,
                         0);
    }

    // Draw the scroll buttons and slider only if they are needed
    int effectiveDisplayHeight = m_nHeight - (m_nMarginY * 2);
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
    // Reset base IGUIParts state
    IGUIParts::Reset();

    // Reset textbox specific state
    m_strCurrentText = "";
    m_nScrollX = 0;
    m_nScrollY = 0;
    m_nContentWidth = 0;
    m_nContentHeight = 0;
    m_nMarginX = 0; // Reset margins to default
    m_nMarginY = 0; // Reset margins to default


    // Reset internal text plane and slider
    if (m_vTextFastPlane != NULL) {
        m_vTextFastPlane->GetFont()->SetText("");
        m_vTextFastPlane->UpdateTextAA();
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
    // The maximum scroll is (content_size - effective_textbox_display_size).
    int effectiveDisplayWidth = m_nWidth - (m_nMarginX * 2);
    int effectiveDisplayHeight = m_nHeight - (m_nMarginY * 2);

    m_nScrollX = max(0, min(m_nScrollX, max(0, m_nContentWidth - effectiveDisplayWidth)));
    m_nScrollY = max(0, min(m_nScrollY, max(0, m_nContentHeight - effectiveDisplayHeight)));

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