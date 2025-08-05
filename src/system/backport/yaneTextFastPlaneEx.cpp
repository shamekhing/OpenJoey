//
// yaneTextFastPlaneEx.cpp : Extended text plane for rich text formatting.
//
// This file implements CTextFastPlaneEx, which inherits from CTextFastPlane
// to provide rich text layout and rendering.
//

#include "yaneTextFastPlaneEx.h"

namespace yaneuraoGameSDK3rd {
namespace Draw {

// =======================================================================================
// CTextFastPlaneEx Class Implementation
// =======================================================================================

// Constructor: Initializes the base class and other members.
CTextFastPlaneEx::CTextFastPlaneEx(CFastDraw* pFastDraw)
    : CFastPlane(pFastDraw), m_nWrapWidth(0) {
    // The rich text parser and other members are constructed automatically.
    // Base class members (m_Font, m_nTextX, m_nTextY) are also initialized.
}

// Destructor.
CTextFastPlaneEx::~CTextFastPlaneEx() {
    // Clean up resources if necessary.
}

// Sets the rich text string to be parsed and rendered.
void CTextFastPlaneEx::SetTextRich(const std::string& text) {
    m_strRichText = text;
}

// Sets the base font size for the parser.
void CTextFastPlaneEx::SetBaseFontSize(int nFontSize) {
    m_parser.SetBaseFontSize(nFontSize);
}

// Update the text and redraw the surface (standard drawing).
LRESULT CTextFastPlaneEx::UpdateText() {
    // We pass false for both antialias and blend flags.
    return UpdateRichText(false, false);
}

// Update the text and redraw with basic antialiasing.
LRESULT CTextFastPlaneEx::UpdateTextA() {
    // We pass true for antialias and false for blend.
    return UpdateRichText(true, false);
}

// Update the text and redraw with full antialiasing and blending.
LRESULT CTextFastPlaneEx::UpdateTextAA() {
    // We pass true for both antialias and blend flags.
    return UpdateRichText(true, true);
}

// The OnDraw method for the base class is not used in this rich text implementation.
LRESULT CTextFastPlaneEx::OnDraw() {
    return 0;
}

// This is the core logic that handles rich text parsing, layout, and drawing.
LRESULT CTextFastPlaneEx::UpdateRichText(bool bAntialias, bool bBlend) {
    // We must have a valid wrapping width.
    if (m_nWrapWidth <= 0) {
        return 1;
    }

    // Clear previous layout data before generating a new layout.
    m_vLines.clear();
    m_parser.SetText(m_strRichText);
    m_parser.SetBaseFontSize(m_parser.GetBaseFontSize());
        
    CRichTextLine currentLine;
    CRichTextSegment segment;
    int currentLineHeight = 0;
    int totalLayoutHeight = 0;
    
    // Loop through all text segments to measure and lay them out.
    while (m_parser.GetNextSegment(segment) == 0) {
        // Handle explicit line breaks.
        if (segment.text == "<BR>") {
            currentLine.alignment = m_parser.GetContext().m_nAlign;
            currentLine.totalHeight = currentLineHeight;
            m_vLines.push_back(currentLine);
            totalLayoutHeight += currentLineHeight;
            currentLine = CRichTextLine();
            currentLineHeight = 0;
            continue;
        }

        // Handle horizontal rule.
        if (segment.text == "<HR>") {
            currentLine.alignment = m_parser.GetContext().m_nAlign;
            currentLine.totalHeight = currentLineHeight;
            m_vLines.push_back(currentLine);
            totalLayoutHeight += currentLineHeight;
            currentLine = CRichTextLine();
            currentLineHeight = 0;
            
            CRichTextLine hrLine;
            hrLine.totalHeight = 5; // A simple height for the rule.
            m_vLines.push_back(hrLine);
            totalLayoutHeight += 5;
            continue;
        }
        
        // Create a temporary CFont object to measure the segment's size.
        CFont tempFont;
        tempFont.SetSize(segment.context.m_nFontSize);
        tempFont.SetColor(segment.context.m_rgbColor);
        tempFont.SetWeight(segment.context.m_bBold ? 700 : 300);
        tempFont.SetItalic(segment.context.m_bItalic);
        tempFont.SetUnderLine(segment.context.m_bUnderLine);
        tempFont.SetStrikeOut(segment.context.m_bStrikeOut);
        tempFont.SetText(segment.text);
        
        int segmentWidth, segmentHeight;
        tempFont.GetSize(segmentWidth, segmentHeight);
		//Optional trailing space adjustment (i disabled this because its buggy atm, fix in future)
		//if (segment.trailingSpace) {
		//	segmentWidth += 1;
		//}

        // Check if the segment fits on the current line. If not, start a new line.
        if (currentLine.totalWidth + segmentWidth > m_nWrapWidth && !currentLine.segments.empty()) {
            currentLine.alignment = m_parser.GetContext().m_nAlign;
            currentLine.totalHeight = currentLineHeight;
            m_vLines.push_back(currentLine);
            totalLayoutHeight += currentLineHeight;
            currentLine = CRichTextLine();
            currentLineHeight = 0;
        }
        
        // Add the segment to the current line.
        segment.width = segmentWidth;
        segment.height = segmentHeight;
        currentLine.segments.push_back(segment);
        currentLine.totalWidth += segment.width;
        
        // Update the line's height to be the height of the tallest segment.
        if (segment.height > currentLineHeight) {
            currentLineHeight = segment.height;
        }
    }
    
    // Push the final line if it's not empty.
    if (!currentLine.segments.empty()) {
        currentLine.alignment = m_parser.GetContext().m_nAlign;
        currentLine.totalHeight = currentLineHeight;
        m_vLines.push_back(currentLine);
        totalLayoutHeight += currentLineHeight;
    }

    // Now that we know the final size, we can resize the actual surface.
    SetSize(m_nWrapWidth, totalLayoutHeight);
	bool createAlphaSurface = bAntialias || bBlend;
	CreateSurface(m_nWrapWidth,totalLayoutHeight, createAlphaSurface);

    // Clear the surface with the background color from the parser context.
    SetFillColor(m_parser.GetContext().m_rgbColorBk);
    Clear();

    // Call the final drawing function.
    DrawLayout(bAntialias, bBlend);
    
    return 0;
}

// Draws the laid-out lines onto the surface.
void CTextFastPlaneEx::DrawLayout(bool antialias, bool blend) {
    int currentY = 0; //m_nTextY

    for (size_t i = 0; i < m_vLines.size(); ++i) {
        const CRichTextLine& line = m_vLines[i];
		int currentX = 0; //m_nTextX

        // Adjust horizontal position based on alignment.
        if (line.alignment == 1) { // Center
            currentX += (m_nWrapWidth - line.totalWidth) / 2;
        } else if (line.alignment == 2) { // Right
            currentX += m_nWrapWidth - line.totalWidth;
        }
        
        // Draw each segment of the line.
        for (size_t j = 0; j < line.segments.size(); ++j) {
            const CRichTextSegment& segment = line.segments[j];
            
			CTextFastPlane* pTextPtr = new CTextFastPlane;
			CFont* tempFont = pTextPtr->GetFont();
			tempFont->SetSize(segment.context.m_nFontSize);
			tempFont->SetColor(segment.context.m_rgbColor);
			tempFont->SetWeight(segment.context.m_bBold ? 700 : 300);
			tempFont->SetItalic(segment.context.m_bItalic);
			tempFont->SetUnderLine(segment.context.m_bUnderLine);
			tempFont->SetStrikeOut(segment.context.m_bStrikeOut);
			tempFont->SetText(segment.text);
			tempFont->SetShadowOffset(segment.context.m_nShadowOffset.cx, segment.context.m_nShadowOffset.cy);

			if(antialias == false && blend == false)
				pTextPtr->UpdateText();
			if(antialias && blend == false)
				pTextPtr->UpdateTextA();
			if(antialias && blend)
				pTextPtr->UpdateTextAA();
			if(antialias == false && blend)
				printf("wtf");

			BltFast(pTextPtr, currentX, currentY);

            currentX += segment.width;
        }
        currentY += line.totalHeight;
        if (i < m_vLines.size() - 1) {
            currentY += m_parser.GetContext().m_nBlankHeight;
        }
    }
}


// =======================================================================================
// CRichTextParser Class Implementation
// =======================================================================================

// Constructor: Initializes the parser's state.
CRichTextParser::CRichTextParser()
    : m_lpStr(NULL), m_lpTextAdr(NULL), m_nBaseSize(12) {
    m_contextStack.push(CRichTextContext());
    m_context = m_contextStack.top();
}

// Sets the rich text string and resets the parser state.
void CRichTextParser::SetText(const std::string& text) {
    m_text = text;
    m_lpStr = m_text.c_str();
    m_lpTextAdr = m_text.c_str();
    
    // Clear the context stack and push the default context.
    while (!m_contextStack.empty()) m_contextStack.pop();
    m_contextStack.push(CRichTextContext());
    m_context = m_contextStack.top();
}

// Parses and returns the next text segment or tag.
LRESULT CRichTextParser::GetNextSegment(CRichTextSegment& segment) {
    segment.text.clear();
    segment.context = m_context;
    
    if (*m_lpStr == '\0') {
        return 3; // End of text.
    }
    
    // Check for a tag.
    if (*m_lpStr == '<') {
        LPCSTR start = m_lpStr + 1;
        
        // Handle closing tags (e.g., </FONT>).
        if (*start == '/') {
            start++;
            if (IsToken(start, "FONT>")) {
                m_lpStr = start;
                if (m_contextStack.size() > 1) { m_contextStack.pop(); m_context = m_contextStack.top(); }
                return 0;
            }
            if (IsToken(start, "BOLD>")) {
                m_lpStr = start;
                if (m_contextStack.size() > 1) { m_contextStack.pop(); m_context = m_contextStack.top(); }
                return 0;
            }
            if (IsToken(start, "ITALIC>")) {
                m_lpStr = start;
                if (m_contextStack.size() > 1) { m_contextStack.pop(); m_context = m_contextStack.top(); }
                return 0;
            }
            if (IsToken(start, "U>")) {
                m_lpStr = start;
                if (m_contextStack.size() > 1) { m_contextStack.pop(); m_context = m_contextStack.top(); }
                return 0;
            }
            if (IsToken(start, "S>")) {
                m_lpStr = start;
                if (m_contextStack.size() > 1) { m_contextStack.pop(); m_context = m_contextStack.top(); }
                return 0;
            }
            if (IsToken(start, "CENTER>")) {
                m_lpStr = start;
                if (m_contextStack.size() > 1) { m_contextStack.pop(); m_context = m_contextStack.top(); }
                return 0;
            }
            if (IsToken(start, "RIGHT>")) {
                m_lpStr = start;
                if (m_contextStack.size() > 1) { m_contextStack.pop(); m_context = m_contextStack.top(); }
                return 0;
            }
        } else {
            // Handle opening tags (e.g., <FONT>, <BR>, <HR>).
            if (IsToken(start, "FONT")) {
                m_lpStr = start;
                m_contextStack.push(m_context);
                
                int fontSizeRate = 0;
                if (IsToken(m_lpStr, "SIZE=")) {
                    GetStrNum(m_lpStr, fontSizeRate);
                    m_context.m_nFontSize = m_nBaseSize + fontSizeRate * 4;
                }
                
                if (IsToken(m_lpStr, "COLOR=")) {
                    GetStrColor(m_lpStr, m_context.m_rgbColor);
                }

                SkipTo(m_lpStr, ">");
                m_lpStr++;
                
                return 0;
            } else if (IsToken(start, "BR>")) {
                m_lpStr = start;
                segment.text = "<BR>";
                return 0;
            } else if (IsToken(start, "HR>")) {
                m_lpStr = start;
                segment.text = "<HR>";
                return 0;
            } else if (IsToken(start, "BOLD>")) {
                m_lpStr = start;
                m_contextStack.push(m_context);
                m_context.m_bBold = true;
                return 0;
            } else if (IsToken(start, "ITALIC>")) {
                m_lpStr = start;
                m_contextStack.push(m_context);
                m_context.m_bItalic = true;
                return 0;
            } else if (IsToken(start, "U>")) {
                m_lpStr = start;
                m_contextStack.push(m_context);
                m_context.m_bUnderLine = true;
                return 0;
            } else if (IsToken(start, "S>")) {
                m_lpStr = start;
                m_contextStack.push(m_context);
                m_context.m_bStrikeOut = true;
                return 0;
            } else if (IsToken(start, "CENTER>")) {
                m_lpStr = start;
                m_contextStack.push(m_context);
                m_context.m_nAlign = 1; // Center alignment
                return 0;
            } else if (IsToken(start, "RIGHT>")) {
                m_lpStr = start;
                m_contextStack.push(m_context);
                m_context.m_nAlign = 2; // Right alignment
                return 0;
            }
        }
        
        SkipTo(m_lpStr, ">");
        m_lpStr++;
        return 0;
    }
    
    // Parse normal text segments (words).
    std::string word;
    while (*m_lpStr != '\0' && *m_lpStr != '<' && *m_lpStr != ' ' && *m_lpStr != '\n') {
        word += *m_lpStr;
        m_lpStr++;
    }
    
    // Handle spaces after words.
    if (*m_lpStr == ' ') {
        //word += *m_lpStr; // this causes double spaces for whatever reasons
        m_lpStr++;
		segment.trailingSpace = true;
	} else{
		segment.trailingSpace = false;
	}

    if (!word.empty()) {
        segment.text = word;
        return 0;
    }
    
    return 1; // Error or unexpected state.
}

// CStringScanner helper function implementations
bool CRichTextParser::IsToken(LPCSTR& lp, LPCSTR lp2) {
    LPCSTR temp = lp;
    while (*lp2 != '\0' && toupper(*temp) == toupper(*lp2)) {
        temp++;
        lp2++;
    }
    if (*lp2 == '\0') {
        lp = temp;
        return true;
    }
    return false;
}

LRESULT CRichTextParser::SkipTo(LPCSTR& lp, LPCSTR lp2) {
    LPCSTR temp = lp;
    while (*temp != '\0' && !IsToken(temp, lp2)) {
        temp++;
    }
    if (*temp == '\0') {
        lp = temp;
        return 1;
    }
    lp = temp;
    return 0;
}

LRESULT CRichTextParser::SkipTo2(LPCSTR& lp, LPCSTR lp2, char* lp3, size_t buf_size) {
    LPCSTR start = lp;
    if (SkipTo(lp, lp2) != 0) {
        return 1;
    }
    
    size_t len = lp - start;
    if (len >= buf_size) {
        len = buf_size - 1;
    }
    
    memcpy(lp3, start, len);
    lp3[len] = '\0';
    
    return 0;
}

LRESULT CRichTextParser::SkipSpace(LPCSTR& lp) {
    while (*lp == ' ' || *lp == '\t' || *lp == '\n' || *lp == '\r') {
        lp++;
    }
    return (*lp == '\0') ? 1 : 0;
}

LRESULT CRichTextParser::GetStrNum(LPCSTR& lp, int& nRate) {
    SkipSpace(lp);
    bool bNegative = false;
    if (*lp == '-') {
        bNegative = true;
        lp++;
    } else if (*lp == '+') {
        lp++;
    }
    if (!isdigit(*lp)) return 1;
    nRate = 0;
    while (isdigit(*lp)) {
        nRate = nRate * 10 + (*lp - '0');
        lp++;
    }
    if (bNegative) nRate = -nRate;
    return 0;
}

LRESULT CRichTextParser::GetStrColor(LPCSTR& lp, COLORREF& nFontColor) {
    SkipSpace(lp);
    if (*lp != '#') return 1;
    lp++;
    
    char hex[7];
    for (int i = 0; i < 6; ++i) {
        if (!isxdigit(*lp)) return 1;
        hex[i] = *lp;
        lp++;
    }
    hex[6] = '\0';
    
    unsigned long value = strtoul(hex, NULL, 16);
    nFontColor = RGB((value >> 16) & 0xFF, (value >> 8) & 0xFF, value & 0xFF);
    
    return 0;
}

LRESULT CRichTextParser::GetNum(LPCSTR& lp, int& nVal) {
    SkipSpace(lp);
    nVal = 0;
    while (isdigit(*lp)) {
        nVal = nVal * 10 + (*lp - '0');
        lp++;
    }
    return 0;
}

} // end of namespace Draw
} // end of namespace yaneuraoGameSDK3rd
