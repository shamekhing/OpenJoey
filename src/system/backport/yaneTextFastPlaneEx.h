//
// yaneTextFastPlaneEx.h
//
// Blits multiple CTextFastPlane objects (that are stylized by HTML tags & text wrap) onto one big CFastPlane surface.
// based on the yaneTextDraw from yaneSDK2nd but with own flavour and heavily rewritten
//

#ifndef __yaneTextFastPlaneEx_h__
#define __yaneTextFastPlaneEx_h__

#include "../../stdafx.h"

namespace yaneuraoGameSDK3rd {
namespace Draw {

static SIZE MakeSize(LONG cx, LONG cy) {
    SIZE sz;
    sz.cx = cx;
    sz.cy = cy;
    return sz;
}

// CRichTextContext: Stores font and style settings for a segment of text.
struct CRichTextContext {
    int m_nFontNo;     // Font number (index)
    int m_nFontSize;   // Font size in points
    COLORREF m_rgbColor; // Font color
    COLORREF m_rgbColorBk; // Background color
    bool m_bBold;        // Bold text flag
    bool m_bItalic;      // Italic text flag
    bool m_bUnderLine;  // Underline flag
    bool m_bStrikeOut;  // Strikeout flag
    int m_nAlign;        // 0:left, 1:center, 2:right
    int m_nBlankHeight; // Height between lines
	SIZE m_nShadowOffset; // Shadow offset (X/Y)

    // Default constructor to set initial values
    CRichTextContext() : m_nFontNo(0), m_nFontSize(12), m_rgbColor(RGB(255, 255, 255)),
                         m_rgbColorBk(RGB(0, 0, 0)), m_bBold(false), m_bItalic(false),
                         m_bUnderLine(false), m_bStrikeOut(false), m_nAlign(0), m_nBlankHeight(0),
						 m_nShadowOffset(MakeSize(0, 0)) {}
};

// CRichTextSegment: Represents a single piece of text with its associated context.
struct CRichTextSegment {
    std::string text;
    CRichTextContext context;
    int width;
    int height;
	bool trailingSpace;
};

// CRichTextLine: Represents a full line of laid-out text.
struct CRichTextLine {
    std::vector<CRichTextSegment> segments;
    int totalWidth;
    int totalHeight;
    int alignment;

    CRichTextLine() : totalWidth(0), totalHeight(0), alignment(0) {}
};

// Rich text parser for tags like <FONT>, <COLOR>, <BOLD>, etc.
class CRichTextParser {
public:
	CRichTextParser();
	void SetText(const std::string& text);
	LRESULT GetNextSegment(CRichTextSegment& segment);
	void ParseTagAttributes(const std::string& tagContent, CRichTextContext& context);
	CRichTextContext GetContext() const { return m_context; }
	void SetBaseFontSize(int nSize) { m_nBaseSize = nSize; }
	int GetBaseFontSize() { return m_nBaseSize; }

private:
	std::string m_text;
	LPCSTR m_lpStr;
	LPCSTR m_lpTextAdr;
	std::stack<CRichTextContext> m_contextStack;
	CRichTextContext m_context;
	int m_nBaseSize;

	bool IsToken(LPCSTR& lp, LPCSTR lp2);
	LRESULT SkipTo(LPCSTR& lp, LPCSTR lp2);
	LRESULT SkipTo2(LPCSTR& lp, LPCSTR lp2, char* lp3, size_t buf_size);
	LRESULT SkipSpace(LPCSTR& lp);
	LRESULT GetStrNum(LPCSTR& lp, int& nRate);
	LRESULT GetStrColor(LPCSTR& lp, COLORREF& nFontColor);
	LRESULT GetNum(LPCSTR& lp, int& nVal);
};

class CTextFastPlaneEx : public CFastPlane {
public:
    CTextFastPlaneEx(CFastDraw* pFastDraw = NULL);
    virtual ~CTextFastPlaneEx();

    void SetTextRich(const std::string& text);
    void SetBaseFontSize(int nFontSize);
    void SetWrapWidth(int nWidth) { m_nWrapWidth = nWidth; }

    LRESULT UpdateText();    // Standard drawing
    LRESULT UpdateTextA();   // Antialias drawing
    LRESULT UpdateTextAA();  // Antialias and blend drawing

protected:
    virtual LRESULT OnDraw();

private:
    LRESULT UpdateRichText(bool bAntialias, bool bBlend);
    void DrawLayout(bool bAntialias, bool bBlend);

    CRichTextParser m_parser;
    std::string m_strRichText;
    int m_nWrapWidth;
    
    std::vector<CRichTextLine> m_vLines;
};

} // end of namespace Draw
} // end of namespace yaneuraoGameSDK3rd

#endif // __yaneTextFastPlaneEx_h__