#ifndef __yaneGUITextBox_h__
#define __yaneGUITextBox_h__

#include "../../stdafx.h" // Corrected path
#include "yaneGUIParts.h"     // For IGUIParts base class
#include "yaneGUIButton.h"
#include "yaneGUISlider.h"    // For CGUISlider and its base classes
// CTextFastPlane is already part of the SDK via stdafx.h, so no direct include needed here.

namespace yaneuraoGameSDK3rd {
namespace Draw {

// Forward declaration
class CGUITextBox;

// Custom slider listener for the textbox
// Derives from CGUISliderEventListener which is a base of CGUINormalSliderListener
class CGUITextBoxSliderListener : public CGUINormalSliderListener {
public:
    CGUITextBoxSliderListener();
    void SetTextBox(smart_ptr<CGUITextBox> pv) { m_vTextBox = pv; }

    void SetMinSizeFromGraphic(ISurface* graphic) {
		int sx, sy;
		graphic->GetSize(sx, sy); // Get the actual dimensions of the graphic
		SetMinSize(sx, sy); // Call the base class method to set m_nMinX and m_nMinY
	}

    virtual void OnPageUp();
    virtual void OnPageDown();
    virtual void OnPageLeft();
    virtual void OnPageRight();

protected:
    smart_ptr<CGUITextBox> m_vTextBox;
};

// Define an enum to distinguish the up and down buttons for the listener
enum ScrollDirection {
    SCROLL_UP,
    SCROLL_DOWN
};

// Custom button listener for the textbox's scroll buttons
class CGUITextBoxArrowButtonListener : public CGUINormalButtonListener {
public:
    CGUITextBoxArrowButtonListener(ScrollDirection direction);
    void SetTextBox(YTL::smart_ptr<CGUITextBox> textBox);
    virtual void OnLBClick(void);

private:
    ScrollDirection m_direction;
    YTL::smart_ptr<CGUITextBox> m_vTextBox;
};

// Main Textbox class
class CGUITextBox : public IGUIParts {
public:
    enum SliderMode {
        NO_SLIDER = -1, // Custom value for no slider
        VERTICAL_SLIDER = 0,
        HORIZONTAL_SLIDER = 1,
        BOTH_SLIDERS = 2
    };

    CGUITextBox();
    virtual ~CGUITextBox();

    // Textbox initialization and setup
    void Create(int x, int y, int width, int height, SliderMode mode = NO_SLIDER);
	void SetSliderGFX(smart_ptr<ISurface> sliderThumbGraphic);
	void SetArrowGFX(smart_ptr<CPlaneLoader> pv, int upIndex, int downIndex);
	void SetSliderLoader(string data, string path);

    // Text content management
    void SetText(const string& text);
    string GetText() const;
    void SetFont(smart_ptr<CFont> font); // Set custom font
    CFont* GetFont(); // Get font for direct manipulation

    // --- NEW: Title and Footer Text methods ---
    void SetTextTitle(const string& text);
    void SetTextFooter(const string& text);
    // --- END NEW ---

    // Text drawing properties
    void SetTextColor(ISurfaceRGB color);
    void SetTextOffset(int x, int y); // Offset for text within the textbox content plane

    // Margins for text within the textbox
    void SetMargins(int x, int y);
    void GetMargins(int& x, int& y) const;

    // Background plane for the textbox
    void SetBackgroundPlane(smart_ptr<ISurface> pv);

    // IGUIParts overrides
    virtual LRESULT OnSimpleMove(ISurface* lp);
    virtual LRESULT OnSimpleDraw(ISurface* lp);
    virtual void Reset();
    virtual void SetMouse(smart_ptr<CFixMouse> pv);

    // Slider specific access for listener
    void ScrollContent(int dx, int dy); // Method to be called by slider listener
	smart_ptr<CGUISlider> GetSlider(){ return m_vSlider; }

    // Get current scroll positions
    void GetScrollPos(int& x, int& y) const { x = m_nScrollX; y = m_nScrollY; }

	void UpdateTextPlane(); // Renders the text onto m_vTextPlane
	std::string WrapText(const std::string& rawText, int availableWidth);
	std::string TrimLeadingSpaces(const std::string& s);

protected:
    void CalculateVisibleContentSize(); // Calculates the actual content size after text rendering
    void UpdateTitleAndFooterPlanes(); // NEW: Renders title and footer

    CTextFastPlane* m_vTextFastPlane; // Plane for rendered main text
    CPlane m_vTextPlane; // Wrapper for m_vTextFastPlane

    // --- NEW: Title and Footer members ---
    std::string m_strTitleText;
    CTextFastPlane* m_vTitleTextFastPlane; // Plane for rendered title text
    CPlane m_vTitleTextPlane; // Wrapper for m_vTitleTextFastPlane

    std::string m_strFooterText;
    CTextFastPlane* m_vFooterTextFastPlane; // Plane for rendered footer text
    CPlane m_vFooterTextPlane; // Wrapper for m_vFooterTextFastPlane
    int m_nFooterHeight; // Height of the rendered footer, 0 if not shown
    // --- END NEW ---

    string m_strCurrentText;
    ISurfaceRGB m_textColor;
    int m_nTextOffsetX;
    int m_nTextOffsetY;

    int m_nMarginX;
    int m_nMarginY;

    smart_ptr<ISurface> m_vBackgroundPlane; // Background for the textbox

    SliderMode m_sliderMode;
    smart_ptr<CGUISlider> m_vSlider;
    smart_ptr<CGUITextBoxSliderListener> m_vSliderListener;
	smart_ptr<ISurface> m_vSliderThumbGraphic;

    // Scroll button members
    smart_ptr<CGUIButton> m_vScrollUpButton;
    smart_ptr<CGUIButton> m_vScrollDownButton;
    smart_ptr<CGUITextBoxArrowButtonListener> m_vScrollUpButtonListener;
    smart_ptr<CGUITextBoxArrowButtonListener> m_vScrollDownButtonListener;

    int m_nWidth;  // Width of the textbox display area
    int m_nHeight; // Height of the textbox display area
    int m_nSliderStripWidth;  // Stores the width of the scrollbar graphic (thumb)
    int m_nSliderStripHeight; // Stores the height of the scrollbar graphic (thumb)
    int m_nContentWidth;  // Actual width of the rendered text content (main text + title)
    int m_nContentHeight; // Actual height of the rendered text content (main text + title)

    int m_nScrollX; // Current X scroll position of the text content
    int m_nScrollY; // Current Y scroll position of the text content

    // Internal helper for clipping rectangle
    RECT m_rcTextBoxClip; // Used for drawing/clipping within the textbox
};

} // namespace Draw
} // namespace yaneuraoGameSDK3rd

#endif // __yaneGUITextBox_h__