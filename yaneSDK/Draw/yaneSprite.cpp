
#include "stdafx.h"
#include "yaneSprite.h"
#include "yaneSurface.h"
#include "../Auxiliary/yaneFile.h"
#include "../Auxiliary/yaneStringScanner.h" // CLineParser
#include "yanePlane.h"

namespace yaneuraoGameSDK3rd {
namespace Draw {

//////////////////////////////////////////////////////////////////////////////
//	CSprite

void	CSimpleSprite::set(ISurface*lp){
	//	v[SæðêÂÌXvCgÆ·é
	WARNING(lp==NULL,"CSimpleSprite::SetSurfaceÅlp==NULL");

	pSurface = lp;
	int	sx,sy;
	pSurface->GetSize(sx,sy);
	::SetRect(&rcRect,0,0,sx,sy);
	nOx = nOy = 0;
	nHeight	= sy;
	bFast	= true;
}

void	CSimpleSprite::set(ISurface*lp,const RECT &rc){
	//	v[SæðêÂÌXvCgÆ·é
	WARNING(lp==NULL,"CSimpleSprite::SetSurfaceÅlp==NULL");

	pSurface = lp;
	rcRect = rc;
	nOx = nOy = 0;
	nHeight	= rc.bottom-rc.top;
	bFast	= true;
}

//////////////////////////////////////////////////////////////////////////////
//	CSprite

CSprite::CSprite() {
	m_nAnimation	= 0;
	m_nDirection	= 0;
	m_nOx			= 0;
	m_nOy			= 0;
	m_nPriority		= 0;	//	±êªfBtHgi[¢Ó¡Í³¢j
	m_bVisible		= true;
	m_nHeight		= 0;	//	[¢Ó¡ÍÈ¢
	m_vSprtieVector.Add(new spritevectorofvector);
}

CSprite::~CSprite(){
}

void	CSprite::setOffset(int nOffsetX,int nOffsetY){
	m_nOx = nOffsetX;
	m_nOy = nOffsetY;
}

void	CSprite::getOffset(int& nOffsetX,int& nOffsetY){
	nOffsetX = m_nOx;
	nOffsetY = m_nOy;
}

void	CSprite::incMotion(){
	m_nAnimation++;
	if (m_nAnimation >= (int)getDirectionalSprite()->size()) {
		m_nAnimation = 0;
	}
}

void	CSprite::setDirection(int nDirection){
	//	ûüÌÏXª³¢êÍAjJE^ðZbgµÈ¢idlj
	if (m_nDirection != nDirection) {
		m_nAnimation = 0;
	}
	m_nDirection = nDirection;
}

//////////////////////////////////////////////////////////////////////////////

void	CSpriteEx::Blt(ISurface*pSurface,LPRECT lpClip){
	Blt(pSurface,0,0,lpClip);
}

void	CSpriteEx::Blt(ISurface*pSurface,int x,int y,LPRECT lpClip){
	BltFix(pSurface,x,y,lpClip);
	incMotion();
}

void	CSpriteEx::BltFix(ISurface*pSurface,LPRECT lpClip){
	int x,y;
	getPos(x,y);
	BltFix(pSurface,x,y,lpClip);
}

void	CSpriteEx::BltFix(ISurface*pSurface,int x,int y,LPRECT lpClip){
	//	Lø©H
	if (!isEnable()) return ;

	//	CDirectDrawÅ`æ·éêLayer³
	CSimpleSprite &s = *getSprite();

	int ox,oy;
	getOffset(ox,oy);
	ox+=x+s.nOx;
	oy+=y+s.nOy;

	//	»ÌÜÜÏ÷µÄÜ¤Æ·Á©`
	if (!s.bFast) {
		pSurface->Blt(s.pSurface,ox,oy,NULL,&s.rcRect,lpClip);
	} else {
		pSurface->BltFast(s.pSurface,ox,oy,NULL,&s.rcRect,lpClip);
	}
}

void	CSpriteEx::BltOnce(ISurface*pSurface,int x,int y,LPRECT lpClip){
	BltFix(pSurface,x,y,lpClip);
	
	//	PcÉÈÁÄ¢½çA»êÈãÍÁZµÈ¢
	int n=getMotion();
	incMotion();
	if (getMotion()==0) {
		setMotion(n);
	}
}

//////////////////////////////////////////////////////////////////////////////
///	yaneSDK2nd©çÌÔÁ±²«B Üè¢¢ÀÅÍÈ¢ÌÅ}lµÈ¢Ål

void	CSpriteChara::Release()
{
	m_apSurface.clear();
	m_vSpriteVector.Delete();
}

LRESULT	CSpriteChara::Load(const string& strFileName)
{
	Release();
	m_vSpriteVector.Add(new CSprite::spritevectorofvector);
	CFile file;
	if (file.Read(strFileName)!=0) return 1;

	//	ReadLine()Åês¸ÂðÍµÄXvCgÝèðsÈ¤

	//	XvCgXCXÌpÓ
	CSimpleSprite ssprite[256];
	ZERO(ssprite);

	int		nLine = 0;
	CHAR	szBuf[256];
	while (true) {
		nLine++;

		CLineParser		lineParser;
		if( file.ReadLine( szBuf ) !=0 ) break;
		lineParser.SetLine( szBuf );					//	C p[T[É¶ñðZbg·é

		//////////////////////////////////////////////////
		// eR}hÊÉÀs³¹é

		// XvCgè`ÌÝè
		if ( lineParser.IsMatch( "#SpriteMax" ) ){
			int		n;
			if ( lineParser.GetNum( n ) != 0 ) { Err.Out("CSpriteChara::LoadÅ#SpriteMaxÉ¸s"); return 2; }
//			m_lpSprite.resize(n);
//			m_nSpriteMax = n;
			m_vSpriteVector->resize(n);
			continue;
		}
		////	Ååêv@ SpriteMax > SpriteF > Sprite
		//	XvCgxÌé`zÂÌÝè (½¾µ²«FÍ³ø)
		if ( lineParser.IsMatch( "#SpriteF" ) ){
			WARNING(m_vSpriteVector->empty(),"CSpriteChara::LoadÅSpriteMaxÈµÉ#SpriteªLé");
			int		n,m;
			lineParser.GetNum(n);
			while (true){
				if (lineParser.GetNum(m)) break;
				ssprite[m].bFast=true;

				//	³ÍA«³
				ssprite[m].nHeight = 32;
					//	¡ñÍ±êÅêµ½Ù¤ªæ³°...
					//	àµÏXªKvÈêÍAeLNXÅÏX·é±Æ

				(*m_vSpriteVector)[n].push_back(ssprite[m]);
			}
			continue;
		}
/*
	Ë@êè`
	#SpritePlane 0,0,34					//	XvCg0`34ÍAv[0©ç35ÂÚ(v[34)Æ·éB
	#SpritePlaneF 0,0,34					//	XvCg0`34ÍAv[0©ç35ÂÚ(v[34)Æ·éB½¾µ²«FÍ³ø
*/

		//	ñzÂXvCgxÌÝè
		if ( lineParser.IsMatch( "#SpritePlaneF" ) ){
			WARNING(m_vSpriteVector->empty(),"CSpriteChara::LoadÅSpriteMaxÈµÉ#SpriteªLé");
			int		n,m,l;
			lineParser.GetNum(n);
			if (lineParser.GetNum(m)) break;
			if (lineParser.GetNum(l)) break;

			CSimpleSprite s = {0};
			for(int i=n;i<n+l;i++){
				ssprite[i].bFast=true;
				//	³ÍA«³
				ssprite[i].nHeight = 32;
				//	¡ñÍ±êÅêµ½Ù¤ªæ³°...
				//	àµÏXªKvÈêÍAeLNXÅÏX·é±Æ
				s.pSurface = m_apSurface[m+i].get();
				{
					int sx,sy;
					s.pSurface->GetSize(sx,sy);
					::SetRect(&s.rcRect,0,0,sx,sy);
				}
//				m_lpSprite[i].SetSpriteAdd(&s);
				(*m_vSpriteVector)[n].push_back(s);
			}
			continue;
		}

		//	ñzÂXvCgxÌÝè
		if ( lineParser.IsMatch( "#SpritePlane" ) ){
			WARNING(m_vSpriteVector->empty(),"CSpriteChara::LoadÅSpriteMaxÈµÉ#SpriteªLé");
			int		n,m,l;
			lineParser.GetNum(n);
			if (lineParser.GetNum(m)) break;
			if (lineParser.GetNum(l)) break;

			CSimpleSprite s = {0};
			for(int i=n;i<n+l;i++){
				ssprite[i].bFast=false;
				//	³ÍA«³
				ssprite[i].nHeight = 32;
				//	¡ñÍ±êÅêµ½Ù¤ªæ³°...
				//	àµÏXªKvÈêÍAeLNXÅÏX·é±Æ
				s.pSurface = m_apSurface[m+i].get();
				{
					int sx,sy;
					s.pSurface->GetSize(sx,sy);
					::SetRect(&s.rcRect,0,0,sx,sy);
				}
//				m_lpSprite[i].SetSpriteAdd(&s);
				(*m_vSpriteVector)[n].push_back(s);
			}
			continue;
		}


		//	XvCgxÌé`zÂÌÝè
		if ( lineParser.IsMatch( "#Sprite" ) ){
			WARNING(m_vSpriteVector->empty(),"CSpriteChara::LoadÅSpriteMaxÈµÉ#SpriteªLé");
			int		n,m;
			lineParser.GetNum(n);
			while (true){
				if (lineParser.GetNum(m)) break;
				ssprite[m].bFast=false;

				//	³ÍA«³
				ssprite[m].nHeight = 32;
					//	¡ñÍ±êÅêµ½Ù¤ªæ³°...
					//	àµÏXªKvÈêÍAeLNXÅÏX·é±Æ
				
//				m_lpSprite[n].SetSpriteAdd(&ssprite[m]);
				(*m_vSpriteVector)[n].push_back(ssprite[m]);
			}
			continue;
		}

		// wèµ½v[pÉXvCgf[^ðÇÝÞ
		if ( lineParser.IsMatch( "#Plane" ) ){
			int		n;
			lineParser.GetNum( n );
			//	s«ªÍAnew·é
			while ((int)m_apSurface.size()<=n){
				m_apSurface.insert(CPlane::GetFactory()->CreateInstance());
			}
			string szFile;
			lineParser.GetStr(szFile);
			m_apSurface[n]->Load(szFile);
			continue;
		}

		//	wèµ½v[pÌ²«FðÀWÅwè
		if ( lineParser.IsMatch( "#ColorKeyA" ) ){
			int		n,m;
			lineParser.GetNum( n );
			lineParser.GetNum( m );
			//	s«ªÍAnew·é
			//	(½¾µA±±Ånewµ½àÌÉÎµÄSetColorKeyÍs³¾ªDD)
			while ((int)m_apSurface.size()<=m){
				m_apSurface.insert(CPlane::GetFactory()->CreateInstance());
			}
			int x,y;
			lineParser.GetNum( x );
			lineParser.GetNum( y );
			for(int i=n;i<=m;i++) {
				m_apSurface[i]->SetColorKeyPos(x,y);
			}
			continue;
		}

		//	wèµ½v[pÌ²«FðRGBÅwè
		if ( lineParser.IsMatch( "#ColorKeyB" ) ){
			int		n,m;
			lineParser.GetNum( n );
			lineParser.GetNum( m );
			//	s«ªÍAnew·é
			while ((int)m_apSurface.size()<=m){
				m_apSurface.insert(CPlane::GetFactory()->CreateInstance());
			}
			int r,g,b;
			lineParser.GetNum( r );
			lineParser.GetNum( g );
			lineParser.GetNum( b );
			for(int i=n;i<=m;i++) {
				m_apSurface[i]->SetColorKey(ISurface::makeRGB(r,g,b));
			}
			continue;
		}

		//	wèµ½v[pÌ²«FðÀWÅwè
		if ( lineParser.IsMatch( "#ColorKey" ) ){
			int		n;
			lineParser.GetNum( n );
			//	s«ªÍAnew·é
			while ((int)m_apSurface.size()<=n){
				m_apSurface.insert(CPlane::GetFactory()->CreateInstance());
			}
			int x,y;
			lineParser.GetNum( x );
			lineParser.GetNum( y );
			m_apSurface[n]->SetColorKeyPos(x,y);
			continue;
		}

		//	Ååêv@ (RectOffset > RectA > Rect)
		//	é`a©çbÜÅÉA(x,y)ÌItZbgð^¦é
		if ( lineParser.IsMatch( "#RectOffset" ) ){
			int		a,b;
			lineParser.GetNum( a );
			lineParser.GetNum( b );
			int		x,y;
			lineParser.GetNum(x);
			lineParser.GetNum(y);
			for(int i=a;i<=b;i++){
				ssprite[i].nOx = x;
				ssprite[i].nOy = y;
			}
			continue;
		}

		//	é`a©çAv[pÉÎµÄA(x,y)©çW~HÅ¡xñAcysÌJèÖ¦µè`ðs¤
		if ( lineParser.IsMatch( "#RectA" ) ){
			int		n;
			lineParser.GetNum( n );
			int		nPlane;
			lineParser.GetNum( nPlane );

			int		x,y,w,h,xt,yt;
			lineParser.GetNum(x);
			lineParser.GetNum(y);
			lineParser.GetNum(w);
			lineParser.GetNum(h);
			lineParser.GetNum(xt);
			lineParser.GetNum(yt);
			for(int j=0;j<yt;j++){
				for(int i=0;i<xt;i++){
					::SetRect( &ssprite[n+i+j*xt].rcRect,x+w*i,y+h*j,x+w*(i+1),y+h*(j+1));
					ssprite[n+i+j*xt].pSurface = m_apSurface[nPlane].get();
				}
			}
			continue;
		}

		//	é`aÆµÄv[pÌ(x,y)©çW~HÌé`Æ·é
		if ( lineParser.IsMatch( "#Rect" ) ){
			int		n;
			lineParser.GetNum( n );
			WARNING(n<0||n>255,"CSpriteChara::LoadÅn<0||n>255");
			int		nPlane;
			lineParser.GetNum( nPlane );
			int		x,y,sx,sy;
			lineParser.GetNum(x);
			lineParser.GetNum(y);
			lineParser.GetNum(sx);
			lineParser.GetNum(sy);
			::SetRect( &ssprite[n].rcRect,x,y,x+sx,y+sy);
			ssprite[n].pSurface = m_apSurface[nPlane].get();
			continue;
		}

		/*
		//	XvCgs1©çs2ÍC[LÆµÄè`ifBtHgÅ5j
		if ( lineParser.IsMatch( "#Layer" ) ){
			int		a,b;
			lineParser.GetNum( a );
			lineParser.GetNum( b );
			int		nLayer;
			lineParser.GetNum(nLayer);
			for(int i=a;i<=b;i++){
				m_lpSprite[i].SetLayer(nLayer);
			}
			continue;
		}
		*/
		if (lineParser.IsMatch("#")){
			Err.Out("CSpriteChara::LoadÅ%sÌ%dsÉs¾È½ß",strFileName.c_str(),nLine);
			return 1;
		}
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////

/*
	XvCgè`t@CÌtH[}bg
	Eå¶¬¶ÍæÊµÈ¢
	EPsÍQTU¶ÜÅ
	Eé`è`Í256ÜÅ
	EXvCgÆÍA éêAÌ®«iCSimpleSpritejðè`·é
	EXvCgè`ÍAé`ðÔÉwè·é±ÆÉæÁÄsÈ¤
	EesA//È~ÍRgÆÅ«é
	Ev[Í³ÀÉpÅ«é
	Eé`ÆÆàÉA«ÌÊuix[XCjðwè·é±Æª é
	Eé`ÆÆàÉAItZbgðwè·é±Æª é
	EXvCgEé`ÔÍO©çnÜé
	#SpriteMax	4			//	XvCgè`ÍSB±êÍæªÅwè·é
	#Plane 0,"test.bmp"		//	v[0ÆµÄtest.bmpðÇÝÞ
	#ColorKey 0,4,10		//	0ÔÌv[Ì²«FðÊuÅwèµA»êÍ(4,10)Æ·é(fBtHgÍ(0,0))
	#Plane 1,"test2.bmp"	//	v[1ÆµÄtest2.bmpðÇÝÞ
	#Rect 0,1,48,48,64,128	//	é`0ÆµÄv[1Ì(48,48)©çW64~H128Ìé`Æ·é
	#Rect 1,1,112,48,64,128	//	é`1ÆµÄv[1Ì(112,48)©çW64~H128Ìé`Æ·é
	#Sprite 1,0,1,1,0		//	XvCg1Íé`0¨é`1¨é`1¨é`0Æ¢¤zÂÆ·é
	#SpriteF 0,0,1,1,0		//	XvCg0Íé`0¨é`1¨é`1¨é`0Æ¢¤zÂÆ·éB½¾µ²«FÍ³ø


	Ë@êè`
	#SpritePlane 0,0,34					//	XvCg0`34ÍAv[0©ç35Â(v[0©çv[34)Æ·éB
	#SpritePlaneF 0,0,34				//	XvCg0`34ÍAv[0©ç35Â(v[0©çv[34)Æ·éB½¾µ²«FÍ³ø
	#ColorKeyA 0,34,4,10				//	0`34Ìv[ÌJ[L[ÍÊuÅwèµA»êÍ(4,10)Æ·é(fBtHgÍ(0,0))
	#ColorKeyB 0,107,0,255,0			
	//	0-107ÔÌv[Ì²«FðRGBÅwèµA»êÍ(0,255,0)Æ·é

//	#SpriteA 0,0,34						//	XvCg0`34ÍAé`0©ç35ÂÚÌé`ÜÅÆ·éBi¢Àj

	#RectA	12,0,10,15,32,64,4,3	//	é`12`ÍAv[0ÉÎ·éAW32~H64ÌTCYÅ èA
									//	»êÍ(10,15)ÉnÜèA¡4ñAc3sÌ12ñªJèÖ¦µ
									//	è`³êéBÂÜèé`12`23ÜÅªêCÉè`³êé
	#RectOffset 10,16,5,6	//	é`10©ç16ÜÅÉA(5,6)ÌItZbgð^¦é

//	#Layer 1,3,5			//	XvCg1©ç3ÍLayer5ÆµÄè`ifBtHgÅ5j
//

*/

}} // end of namespace
