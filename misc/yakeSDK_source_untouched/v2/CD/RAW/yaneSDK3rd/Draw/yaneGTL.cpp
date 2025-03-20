#include "stdafx.h"
#include "yaneGTL.h"

BYTE	CFastPlaneBlendTable::abyMulTable[256*256];
BYTE	CFastPlaneBlendTable::abyMulTable2[256*256];
WORD	CFastPlaneBlendTable::awdMulTableRGB565[65536*16];
WORD	CFastPlaneBlendTable::awdMulTableRGB555[65536*16];
/*
WORD	CFastPlaneBlendTable::awdAddTableRGB565[65536*8];
WORD	CFastPlaneBlendTable::awdAddTableRGB555[65536*8];
WORD	CFastPlaneBlendTable::awdSubTableRGB565[65536*8];
WORD	CFastPlaneBlendTable::awdSubTableRGB555[65536*8];
*/
BYTE	CFastPlaneBlendTable::abyConvertTable555[32768];
	//	これで遅ければ、32bppに対しても、こういうのを用意するが．．．

void	CFastPlaneBlendTable::InitTable()
	{
		{
			for(int i=0;i<256;i++){
				for(int j=0;j<256;j++){
					abyMulTable[i + (j<<8)] = (BYTE)(i*j/255);
					//	このnarrowing変換がオーバーフローしないことは、
					//	255*255/255 == 255であることから明らか。
					abyMulTable2[i + (j<<8)] = (BYTE)(i*(255-j)/255);
				}
			}
		}
		{
			//	for rgb565 & argb3565
			for(int a=0;a<16;a++){
			 int a255 = a*255/7;
			 int a5 = a255 >> 3;
			 int a6 = a255 >> 2;
			 for(int r=0;r<32;r++){
			  for(int g=0;g<64;g++){
			   for(int b=0;b<32;b++){
					DWORD dw = b + (g<<5) + (r<<11) + (a<<16);
					WORD data;
					data= b*a/15 + (g*a/15 << 5) + (r*a/15 <<11);
					awdMulTableRGB565[dw] = data;

/*
	//	飽和加算および減算のテーブル化はやんぺー＾＾；

					//	こんなところにif書くなってか＾＾；
					if (a<8) {
						data = ((b+a5>31)?31:b+a5) + (((g+a6>63)?63:g+a6)<<5) + (((r+a5)>31?31:r+a5)<<11);
						awdAddTableRGB565[dw] = data;
						data = ((b<a5)?0:b-a5) + (((g<a6)?0:g-a6)<<5) + (((r<a5)?0:r-a5)<<11);
						awdSubTableRGB565[dw] = data;
					}
*/

			   }
			  }
			 }
			}
		}
		{
			//	for rgb555 & argb3555
			for(int a=0;a<16;a++){
			 int a255 = a*255/7;
			 int a5 = a255 >> 3;
			 int a6 = a255 >> 2;
			 for(int r=0;r<32;r++){
			  for(int g=0;g<32;g++){
			   for(int b=0;b<32;b++){
					DWORD dw =	b + (g<<5) + (r<<10) + (a<<16);
					WORD  data;
					data = b*a/15 + (g*a/15 << 5) + (r*a/15 <<10);
					awdMulTableRGB555[dw		  ] = data;
					awdMulTableRGB555[dw + (1<<15)] = data; // 不届きなビデオカード対策＾＾；
					//	↑こいつは、本当は、この半分で良いのだが、
					//	最上位が不定になっている可能性があるので、ビットマスクを
					//	とるのがだるいので、これでいってまう！＾＾；

/*
	//	飽和加算のテーブル化はやんぺー（笑）

					//	こんなところにif書くなってか＾＾；
					if (a<8) {
						data = ((b+a5>31)?31:b+a5) + (((g+a6>63)?63:g+a6)<<5) + (((r+a5)>31?31:r+a5)<<11);
						awdAddTableRGB555[dw] = data;
						awdAddTableRGB555[dw + (1<<15)] = data; // 不届きなビデオカード対策＾＾；
						data = ((b<a5)?0:b-a5) + (((g<a6)?0:g-a6)<<5) + (((r<a5)?0:r-a5)<<11);
						awdSubTableRGB555[dw] = data;
						awdSubTableRGB555[dw + (1<<15)] = data; // 不届きなビデオカード対策＾＾；
					}
*/

			   }
			  }
			 }
			}
		}
	}

void	CFastPlaneBlendTable::OnChangePalette(){
	HDC hdc = ::GetDC(NULL);
	if (hdc==NULL) return ;
	PALETTEENTRY pal[256];
	int nPal = GetSystemPaletteEntries(hdc,0,256,&pal[0]);
	//	パレットは256,16,4,2であることもありうる
	ReleaseDC(NULL,hdc);

	for(int r=0;r<32;r++){
	 for(int g=0;g<32;g++){
	  for(int b=0;b<32;b++){
		DWORD dw =	b + (g<<5) + (r<<10);
		BYTE byR=(r << 3),byG=(g << 3),byB=(b << 3);
		BYTE nNearest;
		int nMinDistance=INT_MAX;
		for(int i=0;i<nPal;i++){
			BYTE byR2 = pal[i].peRed;
			BYTE byG2 = pal[i].peGreen;
			BYTE byB2 = pal[i].peBlue;

			//	線形距離で考えると？
			int nDistance =
				abs(byR-byR2) + abs(byG-byG2) + abs(byB-byB2);

			if (nDistance < nMinDistance){
				nMinDistance = nDistance;
				nNearest = (BYTE) i;
			}
		}
		abyConvertTable555[dw] = nNearest;
	  }
	 }
	}
}

///////////////////////////////////////////////////////////////////////////////

//	クリッパー
LRESULT CFastPlaneEffect::Clipping(CSurfaceInfo*lpDstInfo,CSurfaceInfo*lpSrcInfo,const CSurfaceInfo::CBltInfo* pInfo,CFastPlaneEffectClipper* lpClipper){
	int		nSsizeX, nSsizeY, nDsizeX, nDsizeY;
	RECT rcSrcRect,rcDstRect,rcClipRect;
	int		nInitialX;	//	-DX　 :　εの初期値 = -DX
	int		nStepX;		//	 2*SX :　ε+=2*SX
	int		nCmpX;		//	 2*DX :　ε>0ならばε-=2*DXしてね
	int		nStepsX;	//	 SrcXの一回の加算量(整数部)
	int		nInitialY;
	int		nStepY;
	int		nCmpY;
	int		nStepsY;

	
	if (pInfo->pSrcRect==NULL) {
		::SetRect(&rcSrcRect,0,0,lpSrcInfo->GetSize().cx,lpSrcInfo->GetSize().cy);
	} else {
		rcSrcRect = *pInfo->pSrcRect;
	}
	nSsizeX = rcSrcRect.right - rcSrcRect.left;
	nSsizeY = rcSrcRect.bottom - rcSrcRect.top;

	int x,y;
	if (pInfo->pDstPoint==NULL){
		x = y = 0;
	} else {
		x = pInfo->pDstPoint->x;
		y = pInfo->pDstPoint->y;
	}
	if (pInfo->pDstSize==NULL) {
//		rcDstRect = *(lpDstInfo->GetRect());
		//	lpDstSize == NULLのときは、
		//	Srcからの等倍の転送
		::SetRect(&rcDstRect,x,y,x+nSsizeX,y+nSsizeY);
	} else {
		::SetRect(&rcDstRect,x,y,x+pInfo->pDstSize->cx,y+pInfo->pDstSize->cy);
	}
	nDsizeX = rcDstRect.right - rcDstRect.left;
	nDsizeY = rcDstRect.bottom - rcDstRect.top;

	//	クリップ領域
	if (pInfo->pDstClip == NULL){
		::SetRect(&rcClipRect,0,0,lpDstInfo->GetSize().cx,lpDstInfo->GetSize().cy);
	} else {
		rcClipRect = *pInfo->pDstClip;
	}
	LPRECT lpClip = &rcClipRect;

	//--- 追加 '02/03/04  by ENRA ---
	{	// クリップRectは、転送先Rectに内包されないといけない
		// しかしClipRectが的はずれな所にあった場合の処理はどうしよう…
		RECT rcDstRect2 = { 0,0,lpDstInfo->GetSize().cx,lpDstInfo->GetSize().cy };
		int t;
		//--- 修正 '02/04/08  by ENRA ---
		// 勝手に０にしたらあかんかった^^;;
		t = lpClip->left  - rcDstRect2.left;
		if (t<0)	{ lpClip->left	= rcDstRect2.left; }
		t = lpClip->top - rcDstRect2.top;
		if (t<0)	{ lpClip->top = rcDstRect2.top; }
		//-------------------------------
		t = lpClip->right  - rcDstRect2.right;
		if (t>0)	{ lpClip->right	 = rcDstRect2.right; }
		t = lpClip->bottom - rcDstRect2.bottom;
		if (t>0)	{ lpClip->bottom = rcDstRect2.bottom; }
	}
	//-------------------------------

	if ( (nSsizeX == nDsizeX) && (nSsizeY == nDsizeY)){
		lpClipper->bActualSize = true;
		// クリッピングする
		// this clipping algorithm is thought by yaneurao(M.Isozaki)
		int sx = rcSrcRect.right - rcSrcRect.left;
		int sy = rcSrcRect.bottom - rcSrcRect.top;
		::SetRect(&rcDstRect,x,y,x+sx,y+sy);

		int t;
		t = lpClip->left-x;
		if (t>0)	{ rcSrcRect.left += t;	rcDstRect.left	= lpClip->left; }
		t = lpClip->top -y;
		if (t>0)	{ rcSrcRect.top	 += t;	rcDstRect.top	= lpClip->top;	 }
		t = x+sx-lpClip->right;
		if (t>0)	{ rcSrcRect.right -= t;	rcDstRect.right	= lpClip->right; }
		t = y+sy-lpClip->bottom;
		if (t>0)	{ rcSrcRect.bottom -= t; rcDstRect.bottom = lpClip->bottom; }

		//	invalid rect ?
		if (rcSrcRect.left >= rcSrcRect.right ||
			rcSrcRect.top	 >= rcSrcRect.bottom) return 1;

	}else{ 
		//	非等倍の転送？
		lpClipper->bActualSize = false;
		int dx = rcDstRect.right - rcDstRect.left;
		int dy = rcDstRect.bottom - rcDstRect.top;
		::SetRect(&rcDstRect,x,y,x+dx,y+dy);

		//	ブレゼンハムの初期値を計算する

		//	Initial(x,y) = -Dst size(x,y)
		nInitialX = rcDstRect.left - rcDstRect.right;
		nInitialY = rcDstRect.top  - rcDstRect.bottom;

		nStepX	= (rcSrcRect.right	- rcSrcRect.left) ;
		nStepY	= (rcSrcRect.bottom - rcSrcRect.top ) ;

		nCmpX = - (nInitialX);
		nCmpY = - (nInitialY);

		// invalid rect
		if (nCmpX == 0 || nCmpY == 0) return 1;

		// クリッピングする
		// this clipping algorithm is thought by yaneurao(M.Isozaki)

		//	ミラー時と非ミラー時でブレゼンハムの初期値が違う
		int t;
		t = lpClip->left-rcDstRect.left;
		if (t>0)	{
			nInitialX+=t*nStepX;	//	実際にブレゼンハムしてみる
			if (nInitialX > 0){
				int s = nInitialX / nCmpX + 1;
				rcSrcRect.left += s;	//	not mirror!
				nInitialX		-= s*nCmpX;
			}
			rcDstRect.left	 = lpClip->left;
		}
		t = lpClip->top -rcDstRect.top;
		if (t>0)	{
			nInitialY+=t*nStepY;	//	実際にブレゼンハムしてみる
			if (nInitialY > 0){
				int s = nInitialY / nCmpY +1;
				rcSrcRect.top += s;
				nInitialY		-= s*nCmpY;
			}
			rcDstRect.top = lpClip->top;
		}
		t = rcDstRect.right-lpClip->right;
		if (t>0)	{
			rcDstRect.right	 = lpClip->right;
		}
		//	srcRectの算出。dstのはみ出し分だけ控えるのだが、t*m_nStepX/m_nCmpをroundupする必要あり。

		t = rcDstRect.bottom-lpClip->bottom;
		if (t>0)	{ /*rcSrcRect.bottom -= t;*/
			rcDstRect.bottom = lpClip->bottom;}

		//	invalid rect ?
		if (rcSrcRect.left >= rcSrcRect.right ||
			rcSrcRect.top	 >= rcSrcRect.bottom ||
			rcDstRect.left >= rcDstRect.right ||
			rcDstRect.top	 >= rcDstRect.bottom) return 1;

		//	m_nStepX < m_nCmpXを保証する。
		nStepsX = nStepX/nCmpX;
		nStepX -= nCmpX*nStepsX;

		nStepsY = nStepY/nCmpY;
		nStepY -= nCmpY*nStepsY;

		// どこで１を引くのか分からないので、ここで引くのだ☆ By Tia
		nInitialX--;
		nInitialY--;

		lpClipper->nCmpX = nCmpX;
		lpClipper->nCmpY = nCmpY;
		lpClipper->nInitialX = nInitialX;
		lpClipper->nInitialY = nInitialY;
		lpClipper->nStepsX = nStepsX;
		lpClipper->nStepsY = nStepsY;
		lpClipper->nStepX = nStepX;
		lpClipper->nStepY = nStepY;
	}
	lpClipper->rcDstRect = rcDstRect;
	lpClipper->rcSrcRect = rcSrcRect;
	lpClipper->rcClipRect = rcClipRect;
/*
	WARNING ( rcSrcRect.left	< p->m_rcRect.left	|| m_rcSrcRect.top	   < p->m_rcRect.top
		||	  rcSrcRect.right > p->m_rcRect.right || m_rcSrcRect.bottom	 > p->m_rcRect.bottom,
		"CFastPlaneEffect::Clippingで転送元サーフェースの指定がサーフェースからはみ出している");
*/
	// clip end

	return 0;
}
