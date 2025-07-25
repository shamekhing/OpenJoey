#ifndef __yaneGUITextBox_h__
#define __yaneGUITextBox_h__

#include "../../stdafx.h" // Corrected path
#include "yaneGUIParts.h"      // For IGUIParts base class
#include "yaneGUISlider.h"     // For CGUISlider and its base classes
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

    virtual void OnPageUp();
    virtual void OnPageDown();
    virtual void OnPageLeft();
    virtual void OnPageRight();

protected:
    smart_ptr<CGUITextBox> m_vTextBox;
};


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
    void Create(int x, int y, int width, int height, SliderMode mode = NO_SLIDER); //, smart_ptr<ISurface> sliderThumbGraphic = smart_ptr<ISurface>()
	void SetSliderGFX(smart_ptr<ISurface> sliderThumbGraphic);

    // Text content management
    void SetText(const string& text);
    string GetText() const;
    void SetFont(smart_ptr<CFont> font); // Set custom font
    CFont* GetFont(); // Get font for direct manipulation

    // Text drawing properties
    void SetTextColor(ISurfaceRGB color);
    void SetTextOffset(int x, int y); // Offset for text within the textbox content plane

    // Background plane for the textbox
    void SetBackgroundPlane(smart_ptr<ISurface> pv);

    // IGUIParts overrides
    virtual LRESULT OnSimpleMove(ISurface* lp);
    virtual LRESULT OnSimpleDraw(ISurface* lp);
    virtual void Reset();
    virtual void SetMouse(smart_ptr<CFixMouse> pv);

    // Slider specific access for listener
    void ScrollContent(int dx, int dy); // Method to be called by slider listener

    // Get current scroll positions
    void GetScrollPos(int& x, int& y) const { x = m_nScrollX; y = m_nScrollY; }

	void UpdateTextPlane(); // Renders the text onto m_vTextPlane

protected:
    void CalculateVisibleContentSize(); // Calculates the actual content size after text rendering

    CTextFastPlane* m_vTextFastPlane; // Plane for rendered text
    CPlane m_vTextPlane; // Wrapper for m_vTextFastPlane

    string m_strCurrentText;
    ISurfaceRGB m_textColor; // Changed to ISurfaceRGB type, assuming it's a struct with .r, .g, .b members
    int m_nTextOffsetX;
    int m_nTextOffsetY;

    smart_ptr<ISurface> m_vBackgroundPlane; // Background for the textbox

    SliderMode m_sliderMode;
    smart_ptr<CGUISlider> m_vSlider;
    smart_ptr<CGUITextBoxSliderListener> m_vSliderListener;

	smart_ptr<ISurface> m_vSliderThumbGraphic;

    int m_nWidth;  // Width of the textbox display area
    int m_nHeight; // Height of the textbox display area

    int m_nContentWidth;  // Actual width of the rendered text content
    int m_nContentHeight; // Actual height of the rendered text content

    int m_nScrollX; // Current X scroll position of the text content
    int m_nScrollY; // Current Y scroll position of the text content

    // Internal helper for clipping rectangle
    RECT m_rcTextBoxClip; // Used for drawing/clipping within the textbox
};

} // namespace Draw
} // namespace yaneuraoGameSDK3rd

#endif // __yaneGUITextBox_h__