//
//	yaneTextDIB32.h :
//
#ifdef USE_DIB32

#ifndef __yaneTextDIB32_h__
#define __yaneTextDIB32_h__

#include "yaneFont.h"
#include "yaneDIB32.h"

class CTextDIB32 : public CDIB32 {
public:

	CFont*	GetFont(void) { return &m_Font; }
	void	SetTextPos(int x,int y){ m_nTextX=x; m_nTextY=y; }
	void	GetTextPos(int&x,int&y){ x=m_nTextX; y=m_nTextY; }
	LRESULT	UpdateText(void);
	LRESULT	UpdateTextA(void);	//	アンチェリかけるバージョン
										//	→α値を持つようになるので描画はBlendBltFastAlphaで行なう
	LRESULT	UpdateTextAA(void); //	精細なアンチェリをかけるバージョン
										//	UpdateTextAAよりずいぶん遅い＾＾；；

	CTextDIB32();
	virtual ~CTextDIB32() {};	//	plane holder

protected:
	CFont		m_Font;				//	こいつで描画
	int			m_nTextX,m_nTextY;	//	テキスト描画位置
};

/*	//	こういうコーディングもありかな？
#ifdef USE_YGA
class CTextDIB32Alpha : public CTextDIB32 {
public:
	LRESULT	UpdateText(void){ return CTextDIB32::UpdateTextA; }
};
class CTextDIB32Alpha2 : public CTextDIB32 {
public:
	LRESULT	UpdateText(void){ return CTextDIB32::UpdateTextAA; }
};
#endif	//	ifdef USE_YGA
*/

#endif	//	ifdef USE_DIB32

#endif // USE_DIB32
