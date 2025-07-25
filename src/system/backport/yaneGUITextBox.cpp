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
// CGUITextBoxSliderListener Implementation
///////////////////////////////////////////////////////////////////////////////

CGUITextBoxSliderListener::CGUITextBoxSliderListener() {
    // Default min slider size, can be overridden if needed
    // Assuming SetMinSize is part of CGUINormalSliderListener or CGUISliderEventListener
    SetMinSize(5, 5);
}

void CGUITextBoxSliderListener::OnPageUp() {
    if (m_vTextBox.get()) {
        m_vTextBox->ScrollContent(0, -m_nMinY); // Scroll up by min slider height
        ResetEventFlag(); // Reset event flag after handling
    }
}

void CGUITextBoxSliderListener::OnPageDown() {
    if (m_vTextBox.get()) {
        m_vTextBox->ScrollContent(0, m_nMinY); // Scroll down by min slider height
        ResetEventFlag(); // Reset event flag after handling
    }
}

void CGUITextBoxSliderListener::OnPageLeft() {
    if (m_vTextBox.get()) {
        m_vTextBox->ScrollContent(-m_nMinX, 0); // Scroll left by min slider width
        ResetEventFlag(); // Reset event flag after handling
    }
}

void CGUITextBoxSliderListener::OnPageRight() {
    if (m_vTextBox.get()) {
        m_vTextBox->ScrollContent(m_nMinX, 0); // Scroll right by min slider width
        ResetEventFlag(); // Reset event flag after handling
    }
}

///////////////////////////////////////////////////////////////////////////////
// CGUITextBox Implementation
///////////////////////////////////////////////////////////////////////////////

CGUITextBox::CGUITextBox()
    : m_nWidth(0), m_nHeight(0), m_nContentWidth(0), m_nContentHeight(0),
      m_nScrollX(0), m_nScrollY(0), m_sliderMode(NO_SLIDER),
      m_nTextOffsetX(0), m_nTextOffsetY(0)
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

void CGUITextBox::SetSliderGFX(smart_ptr<ISurface> sliderThumbGraphic){
    m_vSliderThumbGraphic = sliderThumbGraphic;
    if (m_vSlider.get() && m_vSliderListener.get() && m_vSliderThumbGraphic.get()) {
        CGUINormalSliderListener* pSliderListener = static_cast<CGUINormalSliderListener*>(m_vSliderListener.get());
        pSliderListener->SetPlane(m_vSliderThumbGraphic);
    }
}

void CGUITextBox::Create(int x, int y, int width, int height, SliderMode mode) {
    // Set base IGUIParts coordinates. This also updates the base class m_rcRect.
    SetXY(x, y);

    m_nWidth = width;
    m_nHeight = height;
    m_sliderMode = mode;

    // Update the clipping rectangle for text rendering, relative to screen coordinates
    ::SetRect(&m_rcTextBoxClip, x, y, x + width, y + height);

    // Initialize text plane
    m_vTextFastPlane = new CTextFastPlane;
    m_vTextPlane = CPlane(m_vTextFastPlane);

    // Default font setup (can be overridden by SetFont)
    if (m_vTextFastPlane != NULL) {
        m_vTextFastPlane->GetFont()->SetColor(m_textColor);
        m_vTextFastPlane->GetFont()->SetSize(16); // Default font size
    }

    // Initialize slider if mode requires it
    if (m_sliderMode != NO_SLIDER) {
        m_vSlider = YTL::smart_ptr<CGUISlider>(new CGUISlider());
        m_vSliderListener = YTL::smart_ptr<CGUITextBoxSliderListener>(new CGUITextBoxSliderListener());

        // Use smart_ptr_static_cast for safe upcasting from CGUITextBox* to smart_ptr<CGUITextBox>
        // and from CGUITextBoxSliderListener* to smart_ptr<CGUISliderEventListener>.
        // Create a temporary smart_ptr for 'this' to perform the cast.
        YTL::smart_ptr<CGUITextBox> this_ptr(this, false); // 'this' is not owned by this temp smart_ptr
        m_vSliderListener->SetTextBox(YTL::smart_ptr_static_cast<CGUITextBox>(this_ptr));

        m_vSlider->SetEvent(YTL::smart_ptr_static_cast<CGUISliderEventListener>(m_vSliderListener));
        m_vSlider->SetType(m_sliderMode);

        // Set slider's drawing coordinates to align with textbox
        // Slider's internal logic expects its SetXY to be its top-left screen coord
        m_vSlider->SetXY(m_nX, m_nY);

        // Adjust slider's movement rectangle based on textbox dimensions
        // This rect defines the *relative* area within the slider's own (m_nX, m_nY) for its thumb movement.
        RECT sliderMovementRect;
        ::SetRect(&sliderMovementRect, 0, 0, m_nWidth, m_nHeight); // Default to full textbox area

        // Define standard padding for where the actual slider button moves, typically narrower/shorter than full textbox
        const int sliderVisualWidth = 15;
        const int sliderVisualHeight = 15;

        if (m_sliderMode == VERTICAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
            // Vertical slider movement area (right edge of textbox)
            sliderMovementRect.left = m_nWidth - sliderVisualWidth;
            sliderMovementRect.right = m_nWidth;
            if (m_sliderMode == BOTH_SLIDERS) {
                sliderMovementRect.bottom -= sliderVisualHeight; // Shrink for horizontal slider corner
            }
        }
        if (m_sliderMode == HORIZONTAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
            // Horizontal slider movement area (bottom edge of textbox)
            sliderMovementRect.top = m_nHeight - sliderVisualHeight;
            sliderMovementRect.bottom = m_nHeight;
            if (m_sliderMode == BOTH_SLIDERS) {
                sliderMovementRect.right -= sliderVisualWidth; // Shrink for vertical slider corner
            }
        }
        m_vSlider->SetRect(&sliderMovementRect);
    }

    //UpdateTextPlane(); // Render initial text (if any)
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
        m_vTextFastPlane->GetFont()->SetText(m_strCurrentText);
        m_vTextFastPlane->SetTextPos(0, 0); // Text position relative to its own plane
        m_vTextFastPlane->UpdateTextAA(); // Render with anti-aliasing

        CalculateVisibleContentSize(); // Recalculate content size after text update
    }

    if (m_vSlider.get()) {
        // Adjust slider's total scrollable range based on content size vs textbox size
        int itemsX = 1; // Default to 1 if no scroll needed or horizontal not enabled
        int itemsY = 1; // Default to 1 if no scroll needed or vertical not enabled

        // Set item numbers for the slider. If content fits, 1 item. Otherwise,
        // use the difference in pixels as the number of items for fine-grained control.
        if (m_sliderMode == HORIZONTAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
            itemsX = max(1, m_nContentWidth - m_nWidth + 1); // Each pixel as an item for fine control
        }
        if (m_sliderMode == VERTICAL_SLIDER || m_sliderMode == BOTH_SLIDERS) {
            itemsY = max(1, m_nContentHeight - m_nHeight + 1); // Each pixel as an item for fine control
        }
        m_vSlider->SetItemNum(itemsX, itemsY);

        // Reset scroll position if content shrinks, to avoid out-of-bounds scrolling
        m_nScrollX = max(0, min(m_nScrollX, max(0, m_nContentWidth - m_nWidth)));
        m_nScrollY = max(0, min(m_nScrollY, max(0, m_nContentHeight - m_nHeight)));

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

    // First, process mouse input for the slider
    if (m_vSlider.get() && m_pvMouse.get()) {
        m_vSlider->SetMouse(m_pvMouse); // Pass mouse state to slider
        result |= m_vSlider->OnSimpleMove(lp); // Process slider's own movement logic

        // Update textbox scroll position based on slider's selected item
        int sliderSelectedItemX, sliderSelectedItemY;
        // Correct usage of GetSelectedItem, it takes two references
        m_vSlider->GetSelectedItem(sliderSelectedItemX, sliderSelectedItemY);

        // The slider's selected item directly represents the pixel scroll position.
        int targetScrollX = sliderSelectedItemX;
        int targetScrollY = sliderSelectedItemY;

        // Clamp target scroll positions to valid range
        int maxScrollX = max(0, m_nContentWidth - m_nWidth);
        int maxScrollY = max(0, m_nContentHeight - m_nHeight);
        targetScrollX = max(0, min(targetScrollX, maxScrollX));
        targetScrollY = max(0, min(targetScrollY, maxScrollY));

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
                                m_nScrollX + m_nWidth, m_nScrollY + m_nHeight);

        // Clamp the source rectangle to the actual dimensions of the rendered text content
        // This prevents reading beyond the bounds of the text plane if content is smaller
        srcRect.right = min(srcRect.right, m_nContentWidth);
        srcRect.bottom = min(srcRect.bottom, m_nContentHeight);

        // Call BltNatural to draw the text content.
        // Parameters:
        // 1. Source Surface: The text content plane (raw pointer from smart_ptr)
        // 2. Destination X: Top-left X coordinate on 'lp' where text starts, with internal offsets
        // 3. Destination Y: Top-left Y coordinate on 'lp' where text starts, with internal offsets
        // 4. Destination Size: NULL, meaning draw at original size as defined by srcRect (no additional scaling)
        // 5. Source Rectangle: Pointer to the srcRect, defining the scrolled view of the text content
        // 6. Destination Clip: Pointer to the textbox's clipping rectangle (m_rcTextBoxClip)
        // 7. Base Point: 0 (top-left alignment)
        lp->BltNatural(m_vTextPlane.get(),
                       drawX + m_nTextOffsetX,
                       drawY + m_nTextOffsetY,
                       NULL,
                       &srcRect,
                       &m_rcTextBoxClip,
                       0);
    }

    // Draw the slider if it exists
    if (m_vSlider.get()) {
        // The slider's OnSimpleDraw handles its own positioning relative to its SetXY
        // (which was set to the textbox's m_nX, m_nY in Create).
        m_vSlider->OnSimpleDraw(lp);
    }

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

    // Reset internal text plane and slider
    if (m_vTextFastPlane != NULL) {
        m_vTextFastPlane->GetFont()->SetText("");
        m_vTextFastPlane->UpdateTextAA();
    }

    if (m_vSlider.get()) {
        m_vSlider->Reset();
    }
}

void CGUITextBox::SetMouse(YTL::smart_ptr<CFixMouse> pv) {
    IGUIParts::SetMouse(pv); // Pass mouse to base
    if (m_vSlider.get()) {
        m_vSlider->SetMouse(pv); // Also pass mouse to slider
    }
}

void CGUITextBox::ScrollContent(int dx, int dy) {
    int oldScrollX = m_nScrollX;
    int oldScrollY = m_nScrollY;

    m_nScrollX += dx;
    m_nScrollY += dy;

    // Clamp scroll positions using min/max macros (or std:: equivalents if resolved)
    // Ensure scroll position does not go beyond the content boundaries.
    // The maximum scroll is (content_size - textbox_display_size).
    m_nScrollX = max(0, min(m_nScrollX, max(0, m_nContentWidth - m_nWidth)));
    m_nScrollY = max(0, min(m_nScrollY, max(0, m_nContentHeight - m_nHeight)));

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

} // namespace Draw
} // namespace yaneuraoGameSDK3rd