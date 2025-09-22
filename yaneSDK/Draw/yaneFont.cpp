#include "stdafx.h"

#include "yaneFont.h"

//////////////////////////////////////////////////////////////////////////////

namespace yaneuraoGameSDK3rd {
namespace Draw {

CFont::CFont(){
	SetSize(16);
	SetHeight(20);
	SetFont(0);	// MS ???? (MS Gothic) - Japanese font names in comments
//	SetColor(RGB(128,192,128));
//	SetBackColor(RGB(64,128,64));
	SetColor(RGB(255,255,255));
	SetBackColor(RGB(128,128,128));
	SetBGColor(CLR_INVALID);
	SetQuality(2);
	SetVerticalFont(false);
	SetWeight(FW_LIGHT);
	SetItalic(false);
	SetUnderLine(false);
	SetStrikeOut(false);
	SetShadowOffset(2,2);
#ifdef OPENJOEY_ENGINE_FIXES
    m_nLetterSpacing = 0; // Initialize member directly in constructor
	m_nCharSet = SHIFTJIS_CHARSET; //ANSI_CHARSET
#endif
}

CFont::~CFont(){
}

//////////////////////////////////////////////////////////////////////////////

void	CFont::SetQuality(int nQuality){
	switch(nQuality) {
	case 0: m_nQuality = DEFAULT_QUALITY; break;
	case 1: m_nQuality = DRAFT_QUALITY; break;
	case 2: m_nQuality = PROOF_QUALITY; break;
	case 3: m_nQuality = ANTIALIASED_QUALITY; break;
	case 4: m_nQuality = NONANTIALIASED_QUALITY; break;
	default: WARNING(true,"CFont::SetQuality ãKíËäOÇÃêîíl");
	}
}

void	CFont::SetSize(int nSize){
	m_nSize = nSize;
	m_nHeight = nSize;	// This value may also need to be updated.
}

void	CFont::SetColor(COLORREF rgb){
	m_nRgb = rgb;
}

void	CFont::SetBackColor(COLORREF rgb){
	m_nBkRgb	= rgb;
}

void	CFont::SetBGColor(COLORREF rgb){
	m_nBGRgb	= rgb;
}

void	CFont::SetHeight(int nHeight){
	m_nHeight = nHeight;
}

#ifdef OPENJOEY_ENGINE_FIXES
void	CFont::SetLetterSpacing(int nSpacing) {
    m_nLetterSpacing = nSpacing;
}
#endif

void	CFont::SetWeight(int nWeight){
	m_nWeight = nWeight;
}

////////////////////////////////////////////////////////////////////
//	font settings / get

void	CFont::SetFont(int nFontNo){
	string name;
	switch (nFontNo) {
#ifdef OPENJOEY_ENGINE_FIXES
	case 0: name ="Arial"; break;
	case 1: name ="Comic Sans MS"; break;
	case 2: name ="Times New Roman"; break;
	case 3: name ="Tahoma"; break;
#else
	case 0: name ="ÇlÇr ÉSÉVÉbÉN"; break;
	case 1: name ="ÇlÇr ÇoÉSÉVÉbÉN"; break;
	case 2: name ="ÇlÇr ñæí©"; break;
	case 3: name ="ÇlÇr Çoñæí©"; break;
#endif
	default: name="";
	}
	if (m_FontName==name) return ;
	m_FontName=name;
}
void	CFont::SetFont(const string& fontname){
	if (m_FontName==fontname) return ;
	m_FontName=fontname;
}

string	CFont::GetFont() const {
	if (!IsVerticalFont()){
	} else {
		if (!m_FontName.empty() && m_FontName[0]=='@'){
			// Already a vertical font, do nothing
		} else {
			return '@' + m_FontName; // Add '@' prefix for vertical font
		}
	}
	return m_FontName;
}

////////////////////////////////////////////////////////////////////

void	CFont::SetText(const string& s){
	m_String = s;
}

void	__cdecl CFont::SetText(LPSTR fmt, ... ){
	CHAR buf[512];
	va_list args; // Declare a va_list variable
    va_start(args, fmt); // Initialize it
	wvsprintf(buf,fmt,args); // Use wvsprintf with va_list
    va_end(args); // Clean up
	SetText((string)buf);
}

void CFont::SetText(int i){
	CHAR buf[16];
	wsprintf(buf,"%d",i);
	SetText(buf);
}

void	CFont::SetItalic(bool b){
	m_bItalic = b;
}

void	CFont::SetUnderLine(bool b){
	m_bUnderLine = b;
}

void	CFont::SetStrikeOut(bool b){
	m_bStrikeOut = b;
}

void	CFont::SetShadowOffset(int nOx,int nOy){
	m_nShadowOffsetX = nOx;
	m_nShadowOffsetY = nOy;
}

/////////////////////////////////////////////////////////////////////////////////////////

//	drawing

void	CFont::OnDraw(HDC hdc,int x,int y){

	// If text string is not set, return
	if (m_String.empty()) return ;

	string strFontName = GetFont();
	HFONT hFont = ::CreateFont(m_nSize,0,0,0,m_nWeight,m_bItalic,m_bUnderLine,m_bStrikeOut,m_nCharSet,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, m_nQuality,FF_MODERN,strFontName.c_str());

	if (hFont==NULL) return ; // Memory allocation failure

	HFONT hFontLast = (HFONT)::SelectObject(hdc,hFont);

#ifdef OPENJOEY_ENGINE_FIXES
    int iOldCharExtra = 0;
    if (m_nLetterSpacing != 0) {
        iOldCharExtra = ::SetTextCharacterExtra(hdc, m_nLetterSpacing);
    }
#endif

	// If drawing with background color, first draw opaque, then transparent
	bool bFirst = true;

	if (m_nBkRgb!=CLR_INVALID){
		// This does not seem to overlap (shadow effect?)
		POINT point[5] = {
			{m_nShadowOffsetX,m_nShadowOffsetY},		// 1
			{0,1},
			{2,1},		// 2
			{1,2},
			{2,2}		// 3
		};

		::SetTextColor(hdc,m_nBkRgb); // Set shadow/background color

		// Set background mode
		if (m_nBGRgb==CLR_INVALID) {
			::SetBkMode(hdc, TRANSPARENT);
		} else {
			::SetBkMode(hdc, OPAQUE);
			::SetBkColor(hdc,m_nBGRgb);
		}
		bFirst = false;

		// The loop for shadow, currently only runs once (i=0)
		for(int i=0;i<1 /* 5 */;i++){
			TextOut(hdc,x + point[i].x,y + point[i].y,m_String);
		}
	}

	if (m_nRgb!=CLR_INVALID){
		// Set background mode for foreground text
		if (bFirst){
			if (m_nBGRgb==CLR_INVALID) {
				::SetBkMode(hdc, TRANSPARENT);
			} else {
				::SetBkMode(hdc, OPAQUE);
				::SetBkColor(hdc,m_nBGRgb);
			}
		} else if (m_nBGRgb!=CLR_INVALID) {
			// If it's a second drawing pass (for foreground), revert to transparent if background was opaque
			::SetBkMode(hdc, TRANSPARENT);
		}

		::SetTextColor(hdc,m_nRgb); // Set foreground text color
		TextOut(hdc,x,y,m_String);	// Display text
	}

#ifdef OPENJOEY_ENGINE_FIXES
    if (m_nLetterSpacing != 0) {
        ::SetTextCharacterExtra(hdc, iOldCharExtra); // Reset to original value
    }
#endif

	::SelectObject(hdc,hFontLast); // Select back old font
	::DeleteObject(hFont);		// Delete created font
}

LRESULT	CFont::GetSize(int& sx,int& sy){
	sx = 0; sy = 0;	// fail safe

	// If text string is not set, return
	if (m_String.empty()) return 1;

	string strFontName = GetFont();
	HFONT hFont = ::CreateFont(m_nSize,0,0,0,m_nWeight,m_bItalic,m_bUnderLine,m_bStrikeOut,m_nCharSet,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, m_nQuality,FF_MODERN,strFontName.c_str());
	if (hFont==NULL) return 1;

	SIZE size;
	HDC hdc = ::CreateCompatibleDC(NULL);
	HFONT hFontLast = (HFONT)::SelectObject(hdc,hFont);

#ifdef OPENJOEY_ENGINE_FIXES
    int iOldCharExtra = 0;
    if (m_nLetterSpacing != 0) {
        iOldCharExtra = ::SetTextCharacterExtra(hdc, m_nLetterSpacing);
    }
#endif

	LPCSTR p = m_String.c_str();
	CHAR buf[256];	// Limited by this buffer size for text measurement
	LPSTR q=buf;
	size.cx=0; size.cy=0;
	SIZE size2;
	for(;;){
		*q = *(p++);
		if (*q=='\n') {
			*q=0;	// Null-terminate line
			::GetTextExtentPoint32(hdc,buf,::lstrlen(buf),&size2);
            // This GetTextExtentPoint32 call WILL reflect SetTextCharacterExtra from above.
            
            // For italic fonts, API can miscalculate width (bug workaround)
			if (m_bItalic) size2.cx+=m_nSize/4;

			size.cy+=m_nHeight;	// Fixed by akira (line height)
			if (size.cx<size2.cx) size.cx=size2.cx; // Update max width
			q=buf;
			continue;
		} else if (*q=='\0') {
			::GetTextExtentPoint32(hdc,buf,::lstrlen(buf),&size2);
            // This GetTextExtentPoint32 call WILL reflect SetTextCharacterExtra from above.

            // For italic fonts, API can miscalculate width (bug workaround)
			if (m_bItalic) size2.cx+=m_nSize/4;
			size.cy+=size2.cy; // Add height of the last line
			if (size.cx<size2.cx) size.cx=size2.cx; // Update max width
			break;
		}
		q++;
	}

#ifdef OPENJOEY_ENGINE_FIXES
    if (m_nLetterSpacing != 0) {
        ::SetTextCharacterExtra(hdc, iOldCharExtra); // Reset to original value
    }
#endif

	sx = size.cx + 3;	// Add padding/shadow allowance
	sy = size.cy + 3;	// Add padding/shadow allowance

	::SelectObject(hdc,hFontLast);
	::DeleteObject(hFont);

	::DeleteDC(hdc); // Delete the DC

	return 0;
}

// NOTE: This TextOut function (CFont::TextOut) is an internal helper.
// It relies on the HDC having the character extra already set by CFont::OnDraw.
void	CFont::TextOut(HDC hdc,int x,int y,const string& s){

	// If text string is not set, return
	if (s.empty()) return ; // Changed from m_String.empty() to s.empty() as this draws 's'

	CHAR buf[256];
	LPCSTR p = s.c_str();
	LPSTR q=buf;
	for(;;){
		*q = *(p++);
		if (*q=='\n') {
			*q=0;	// Null-terminate line
			::TextOut(hdc,x,y,buf,::lstrlen(buf)); // Draw the line
			y+=m_nHeight; // Advance Y for next line
			q=buf;
			continue;
		} else if (*q=='\0') {
			::TextOut(hdc,x,y,buf,::lstrlen(buf)); // Draw the last line
			break;
		}
		q++;
	}
}

} // end of namespace Draw
} // end of namespace yaneuraoGameSDK3rd
