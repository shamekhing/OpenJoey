#include "stdafx.h"
#include "yaneSurface.h"
#include "../Draw/yaneGTL.h"			//	CFastPlane???????g??
#include "../Draw/yaneGraphicLoader.h"	//	Load??g??
#include "../Draw/yaneSurfaceBltter.h"	//	Clear??g??
#include "../Draw/yaneDIBitmap.h"		//	Load??HDC?o?R???????????K?v
#include "../Auxiliary/yaneLZSS.h"		//	YGA??????g??

////////////////////////////////////////////////////////////////////

namespace yaneuraoGameSDK3rd {
namespace Draw {

ISurface::ISurface():
	m_vSurfaceInfo(new CSurfaceInfo)
{
}

/////////////////////////////////////////////////////////////////////
//	fill color??A

void		ISurface::SetFillColor(ISurfaceRGB c){
		///	Clear????F???w????(Default==RGB(0,0,0))
	CSurfaceInfo* pInfo = m_vSurfaceInfo.get();
	if (pInfo) pInfo->SetFillColor(c);
}

ISurfaceRGB	ISurface::GetFillColor() const{
				///	Clear??F?? ? use m_vSurfaceInfo directly to avoid recursion if derived GetSurfaceInfo() returns an object that dispatches back to this
	const CSurfaceInfo* pInfo = m_vSurfaceInfo.get();
	return pInfo ? pInfo->GetFillColor() : 0;
}

LRESULT		ISurface::Clear(LPCRECT lpRect/*=NULL*/){
			///	??`?N???A
	return GeneralEffect(CSurfaceInfo::eSurfaceFill,lpRect);
}

/////////////////////////////////////////////////////////////////////
///	????L?[??A????

LRESULT ISurface::SetColorKey(ISurfaceRGB rgb){
	m_bUsePosColorKey = false;
	m_rgbColorKey = rgb;
	return UpdateColorKey();
}

LRESULT ISurface::SetColorKeyPos(int x,int y){
	m_bUsePosColorKey = true;
	m_nColorKeyX = x; m_nColorKeyY = y;
	//	????|?C???g??F????????
	return UpdateColorKey();
}

ISurfaceRGB ISurface::GetColorKey() const{
	return m_rgbColorKey;
}

void	ISurface::ResetColorKey(){
	m_bUsePosColorKey	= true;
	m_nColorKeyX = m_nColorKeyY = 0;
	m_rgbColorKey =	0;
}

LRESULT ISurface::UpdateColorKey(){
	LRESULT lr;
	if (m_bUsePosColorKey) {	// ??u?w??^????L?[
		lr =GetSurfaceInfo()->GetPixel(m_nColorKeyX,m_nColorKeyY,m_rgbColorKey);
	} else {	// ?F?w??^????L?[
		//	?J???[?L?[??ISurfaceRGB?????????????????
		//	??????????????K?v??????????
		lr = 0;
	}
	//	?T?[?t?F?[?X?\?????????A????????f??????
	GetSurfaceInfo()->SetColorKey(GetColorKey());
	return lr;
}

/////////////////////////////////////////////////////////////////////
//	????nbltter

/*
LRESULT ISurface::BltNaturalPos(const ISurface* lpSrc,int x,int y,int nMode,BYTE nFade,
	LPCSIZE pDstSize,LPCRECT pSrcRect,LPCRECT	pDstClip,int nBasePoint)
{
	//	nMode == ?x?[?X??u(0:?????S 1:???? 2:?E?? 3:???? 4:?E??)
	if (lpSrc==NULL) return -1;

	//	?T?C?Y??
	int sx,sy;
	if (pDstSize==NULL) {
		//	?]????T?C?Y???w???????????]????T?C?Y??]?????T?[?t?F?[?X?????????????
		lpSrc->GetSize(sx,sy);
	} else {
		//	?]????T?C?Y???w???????????]????T?C?Y??A??????p
		sx = pDstSize->cx;
		sy = pDstSize->cy;
	}

	switch (nMode){	//	?t?F?[?h???x??
	case 0: return BltNatural(lpSrc,x-(sx>>1),y-(sy>>1)	,nFade,pDstSize,pSrcRect,pDstClip,nBasePoint);
	case 1: return BltNatural(lpSrc,x	,y				,nFade,pDstSize,pSrcRect,pDstClip,nBasePoint);
	case 2: return BltNatural(lpSrc,x-sx,y				,nFade,pDstSize,pSrcRect,pDstClip,nBasePoint);
	case 3: return BltNatural(lpSrc,x	,y-sy			,nFade,pDstSize,pSrcRect,pDstClip,nBasePoint);
	case 4: return BltNatural(lpSrc,x-sx,y-sy			,nFade,pDstSize,pSrcRect,pDstClip,nBasePoint);
	}
	return -2;	//	????HnMode???O
}
*/

/////////////////////////////////////////////////////////////////////

RECT	CSurfaceInfo::GetClipRect(LPCRECT lpRect) const{
	RECT r;
	if(lpRect == NULL){
		::SetRect(&r,0,0,GetSize().cx,GetSize().cy);
	} else {
		r = *lpRect;
	}

	// ?N???b?s???O????
	RECT lpClip = { 0,0,m_size.cx,m_size.cy };

	if (lpClip.left	 > r.left)	{ r.left   = lpClip.left;	 }
	if (lpClip.right < r.right) { r.right  = lpClip.right;	 }
	if (lpClip.top	 > r.top)	{ r.top	   = lpClip.top;	 }
	if (lpClip.bottom<r.bottom) { r.bottom = lpClip.bottom; }

	//	invalid rect,but..
	//	if (r.left >= r.right || r.top	>= r.bottom) return 1;

	return r;
}

LRESULT CSurfaceInfo::Lock() const
#ifdef USE_EXCEPTION
	throw(CRuntimeException)
#endif
{
	if (!IsInit()) return -1;	//	????????????????????
	if (IsLocked()) {
		#ifdef USE_EXCEPTION
			throw CRuntimeException("?Q?dLock(CSurfaceInfo::Lock)");
		#else
			return 1;
		#endif
	}
	SetLock(true);
	return GetLocker()->Lock();
}

LRESULT CSurfaceInfo::Unlock() const
#ifdef USE_EXCEPTION
	throw(CRuntimeException)
#endif
{
	if (!IsInit()) return -1;	//	????????????????????
	if (!IsLocked()) {
		#ifdef USE_EXCEPTION
			throw CRuntimeException("?Q?dUnlockCSurfaceInfo::Unlock");
		#else
			return 1;
		#endif
	}
	SetLock(false);
	return GetLocker()->Unlock();
}

/////////////////////////////////////////////////////////////////////

///	?T?[?t?F?[?X??P?s?N?Z????o?C?g??
int		CSurfaceInfo::GetPixelSize() const {
	int nSize;
	switch (GetSurfaceType()){
	case 2: nSize = sizeof(CFastPlaneBytePal); break;
	case 3: nSize = sizeof(CFastPlaneRGB565); break;
	case 4: nSize = sizeof(CFastPlaneRGB555); break;
	case 5: nSize = sizeof(CFastPlaneRGB888); break;
	case 6: nSize = sizeof(CFastPlaneBGR888); break;
	case 7: nSize = sizeof(CFastPlaneXRGB8888); break;
	case 8: nSize = sizeof(CFastPlaneXBGR8888); break;
	case 10: nSize = sizeof(CFastPlaneARGB4565); break;
	case 11: nSize = sizeof(CFastPlaneARGB4555); break;
	case 12: nSize = sizeof(CFastPlaneARGB8888); break;
	case 13: nSize = sizeof(CFastPlaneABGR8888); break;
	default: nSize = 0; break;
	}
	return nSize;
}

/////////////////////////////////////////////////////////////////////

void	CSurfaceInfo::iterator::SetRGB(BYTE r,BYTE g,BYTE b,BYTE a){

//	ISurfaceRGB??ARGB8888
	int n = GetSurfaceInfo()->GetSurfaceType();
	BYTE *p = (BYTE*)GetSurfaceInfo()->GetPtr() +
				GetSurfaceInfo()->GetPitch() * m_nY;
	switch(n){
	case 3:
		p += sizeof(CFastPlaneRGB565)*m_nX;
		((CFastPlaneRGB565*)p) ->SetRGB(r,g,b);
		break;
	case 4:
		p += sizeof(CFastPlaneRGB555)*m_nX;
		((CFastPlaneRGB555*)p) ->SetRGB(r,g,b);
		break;
	case 5:
		p += sizeof(CFastPlaneRGB888)*m_nX;
		((CFastPlaneRGB888*)p) ->SetRGB(r,g,b);
		break;
	case 6:
		p += sizeof(CFastPlaneBGR888)*m_nX;
		((CFastPlaneBGR888*)p) ->SetRGB(r,g,b);
		break;
	case 7:
		p += sizeof(CFastPlaneXRGB8888)*m_nX;
		((CFastPlaneXRGB8888*)p) ->SetRGB(r,g,b);
		break;
	case 8:
		p += sizeof(CFastPlaneXBGR8888)*m_nX;
		((CFastPlaneXBGR8888*)p) ->SetRGB(r,g,b);
		break;
	case 10:
		p += sizeof(CFastPlaneARGB4565)*m_nX;
		((CFastPlaneARGB4565*)p) ->SetRGB(r,g,b);
		((CFastPlaneARGB4565*)p) ->SetA(a);
		break;
	case 11:
		p += sizeof(CFastPlaneARGB4555)*m_nX;
		((CFastPlaneARGB4555*)p) ->SetRGB(r,g,b);
		((CFastPlaneARGB4555*)p) ->SetA(a);
		break;
	case 12:
		p += sizeof(CFastPlaneARGB8888)*m_nX;
		((CFastPlaneARGB8888*)p) ->SetRGB(r,g,b);
		((CFastPlaneARGB8888*)p) ->SetA(a);
		break;
	case 13:
		p += sizeof(CFastPlaneABGR8888)*m_nX;
		((CFastPlaneABGR8888*)p) ->SetRGB(r,g,b);
		((CFastPlaneABGR8888*)p) ->SetA(a);
		break;
	}
}

ISurfaceRGB	CSurfaceInfo::iterator::GetRGB() const{

//	ISurfaceRGB??ARGB8888
	int n = GetSurfaceInfo()->GetSurfaceType();
	int r,g,b,a;
	BYTE *p = (BYTE*)GetSurfaceInfo()->GetPtr() +
				GetSurfaceInfo()->GetPitch() * m_nY;
	switch(n){
	case 3:
		p += sizeof(CFastPlaneRGB565)*m_nX;
		r = ((CFastPlaneRGB565*)p) ->GetR();
		g = ((CFastPlaneRGB565*)p) ->GetG();
		b = ((CFastPlaneRGB565*)p) ->GetB();
		a = 0;
		break;
	case 4:
		p += sizeof(CFastPlaneRGB555)*m_nX;
		r = ((CFastPlaneRGB555*)p) ->GetR();
		g = ((CFastPlaneRGB555*)p) ->GetG();
		b = ((CFastPlaneRGB555*)p) ->GetB();
		a = 0;
		break;
	case 5:
		p += sizeof(CFastPlaneRGB888)*m_nX;
		r = ((CFastPlaneRGB888*)p) ->GetR();
		g = ((CFastPlaneRGB888*)p) ->GetG();
		b = ((CFastPlaneRGB888*)p) ->GetB();
		a = 0;
		break;
	case 6:
		p += sizeof(CFastPlaneBGR888)*m_nX;
		r = ((CFastPlaneBGR888*)p) ->GetR();
		g = ((CFastPlaneBGR888*)p) ->GetG();
		b = ((CFastPlaneBGR888*)p) ->GetB();
		a = 0;
		break;
	case 7:
		p += sizeof(CFastPlaneXRGB8888)*m_nX;
		r = ((CFastPlaneXRGB8888*)p) ->GetR();
		g = ((CFastPlaneXRGB8888*)p) ->GetG();
		b = ((CFastPlaneXRGB8888*)p) ->GetB();
		a = 0;
		break;
	case 8:
		p += sizeof(CFastPlaneXBGR8888)*m_nX;
		r = ((CFastPlaneXBGR8888*)p) ->GetR();
		g = ((CFastPlaneXBGR8888*)p) ->GetG();
		b = ((CFastPlaneXBGR8888*)p) ->GetB();
		a = 0;
		break;
	case 10:
		p += sizeof(CFastPlaneARGB4565)*m_nX;
		r = ((CFastPlaneARGB4565*)p) ->GetR();
		g = ((CFastPlaneARGB4565*)p) ->GetG();
		b = ((CFastPlaneARGB4565*)p) ->GetB();
		a = ((CFastPlaneARGB4565*)p) ->GetA();
		break;
	case 11:
		p += sizeof(CFastPlaneARGB4555)*m_nX;
		r = ((CFastPlaneARGB4555*)p) ->GetR();
		g = ((CFastPlaneARGB4555*)p) ->GetG();
		b = ((CFastPlaneARGB4555*)p) ->GetB();
		a = ((CFastPlaneARGB4555*)p) ->GetA();
		break;
	case 12:
		p += sizeof(CFastPlaneARGB8888)*m_nX;
		r = ((CFastPlaneARGB8888*)p) ->GetR();
		g = ((CFastPlaneARGB8888*)p) ->GetG();
		b = ((CFastPlaneARGB8888*)p) ->GetB();
		a = ((CFastPlaneARGB8888*)p) ->GetA();
		break;
	case 13:
		p += sizeof(CFastPlaneABGR8888)*m_nX;
		r = ((CFastPlaneABGR8888*)p) ->GetR();
		g = ((CFastPlaneABGR8888*)p) ->GetG();
		b = ((CFastPlaneABGR8888*)p) ->GetB();
		a = ((CFastPlaneABGR8888*)p) ->GetA();
		break;
	default:
		r = g = b = a = 0;
	}
	return ISurface::makeRGB(r,g,b,a);
}

LRESULT CSurfaceInfo::GetPixel(int x,int y,ISurfaceRGB&rgb) const{
	CSurfaceInfo* pThis = const_cast<CSurfaceInfo*>(this);
	if (pThis->Lock()!=0) return 0;
	CSurfaceLockerGuard guard(pThis /* ->GetLocker()*/ );
	//	Unlock???????????????s?????????

	//	???O?H
	if (x<0 || y<0 || x>=GetSize().cx || y>=GetSize().cy) {
		return -1;
	}

	int nType = GetSurfaceType();
	switch (nType) {
	case 2: {
		CFastPlaneBytePal* p = (CFastPlaneBytePal*)((BYTE*)GetPtr() + GetPitch() * y + x * sizeof(CFastPlaneBytePal));
		rgb = ISurface::makeRGB(p->GetR(),p->GetG(),p->GetB()); break; }
	case 3: {
		CFastPlaneRGB565* p = (CFastPlaneRGB565*)((BYTE*)GetPtr() + GetPitch() * y + x * sizeof(CFastPlaneRGB565));
		rgb = ISurface::makeRGB(p->GetR(),p->GetG(),p->GetB()); break; }
	case 4: {
		CFastPlaneRGB555* p = (CFastPlaneRGB555*)((BYTE*)GetPtr() + GetPitch() * y + x * sizeof(CFastPlaneRGB555));
		rgb = ISurface::makeRGB(p->GetR(),p->GetG(),p->GetB()); break; }
	case 5: {
		CFastPlaneRGB888* p = (CFastPlaneRGB888*)((BYTE*)GetPtr() + GetPitch() * y + x * sizeof(CFastPlaneRGB888));
		rgb = ISurface::makeRGB(p->GetR(),p->GetG(),p->GetB()); break; }
	case 6: {
		CFastPlaneBGR888* p = (CFastPlaneBGR888*)((BYTE*)GetPtr() + GetPitch() * y + x * sizeof(CFastPlaneBGR888));
		rgb = ISurface::makeRGB(p->GetR(),p->GetG(),p->GetB()); break; }
	case 7: {
		CFastPlaneXRGB8888* p = (CFastPlaneXRGB8888*)((BYTE*)GetPtr() + GetPitch() * y + x * sizeof(CFastPlaneXRGB8888));
		rgb = ISurface::makeRGB(p->GetR(),p->GetG(),p->GetB()); break; }
	case 8: {
		CFastPlaneXBGR8888* p = (CFastPlaneXBGR8888*)((BYTE*)GetPtr() + GetPitch() * y + x * sizeof(CFastPlaneXBGR8888));
		rgb = ISurface::makeRGB(p->GetR(),p->GetG(),p->GetB()); break; }
	case 10: {
		CFastPlaneARGB4565* p = (CFastPlaneARGB4565*)((BYTE*)GetPtr() + GetPitch() * y + x * sizeof(CFastPlaneARGB4565));
		rgb = ISurface::makeRGB(p->GetR(),p->GetG(),p->GetB(),p->GetA()); break; }
	case 11: {
		CFastPlaneARGB4555* p = (CFastPlaneARGB4555*)((BYTE*)GetPtr() + GetPitch() * y + x * sizeof(CFastPlaneARGB4555));
		rgb = ISurface::makeRGB(p->GetR(),p->GetG(),p->GetB(),p->GetA()); break; }
	case 12: {
		CFastPlaneARGB8888* p = (CFastPlaneARGB8888*)((BYTE*)GetPtr() + GetPitch() * y + x * sizeof(CFastPlaneARGB8888));
		rgb = ISurface::makeRGB(p->GetR(),p->GetG(),p->GetB(),p->GetA()); break; }
	case 13: {
		CFastPlaneABGR8888* p = (CFastPlaneABGR8888*)((BYTE*)GetPtr() + GetPitch() * y + x * sizeof(CFastPlaneARGB8888));
		rgb = ISurface::makeRGB(p->GetR(),p->GetG(),p->GetB(),p->GetA()); break; }
	default:	//	????A?A?s???T?[?t?F?[?X???D?D
		rgb = 0; return -2;
	}
	return 0;
}

LRESULT CSurfaceInfo::GetMatchColor(ISurfaceRGB rgb,DWORD&dw) const {

	int nType = GetSurfaceType();
	switch (nType) {
	case 2:
		//	??????????????[
		break;
	case 3: {
		CFastPlaneRGB565 p;
		p.SetRGB((BYTE)(rgb & 0xff),(BYTE)((rgb>>8) & 0xff),(BYTE)((rgb>>16)& 0xff));
		dw = p.GetRGB(); break; }
	case 4: {
		CFastPlaneRGB555 p;
		p.SetRGB((BYTE)(rgb & 0xff),(BYTE)((rgb>>8) & 0xff),(BYTE)((rgb>>16)& 0xff));
		dw = p.GetRGB(); break; }
	case 5: {
		CFastPlaneRGB888 p;
		p.SetRGB((BYTE)(rgb & 0xff),(BYTE)((rgb>>8) & 0xff),(BYTE)((rgb>>16)& 0xff));
		dw = p.GetRGB(); break; }
	case 6: {
		CFastPlaneBGR888 p;
		p.SetRGB((BYTE)(rgb & 0xff),(BYTE)((rgb>>8) & 0xff),(BYTE)((rgb>>16)& 0xff));
		dw = p.GetRGB(); break; }
	case 7: {
		CFastPlaneXRGB8888 p;
		p.SetRGB((BYTE)(rgb & 0xff),(BYTE)((rgb>>8) & 0xff),(BYTE)((rgb>>16)& 0xff));
		dw = p.GetRGB(); break; }
	case 8: {
		CFastPlaneXBGR8888 p;
		p.SetRGB((BYTE)(rgb & 0xff),(BYTE)((rgb>>8) & 0xff),(BYTE)((rgb>>16)& 0xff));
		dw = p.GetRGB(); break; }
	case 10: {
		CFastPlaneARGB4565 p;
		p.SetRGB((BYTE)(rgb & 0xff),(BYTE)((rgb>>8) & 0xff),(BYTE)((rgb>>16)& 0xff));
		p.SetA((BYTE)((rgb>>24)&0xff));
		dw = p.GetRGBA(); break; }
	case 11: {
		CFastPlaneARGB4555 p;
		p.SetRGB((BYTE)(rgb & 0xff),(BYTE)((rgb>>8) & 0xff),(BYTE)((rgb>>16)& 0xff));
		p.SetA((BYTE)((rgb>>24)&0xff));
		dw = p.GetRGBA(); break; }
	case 12: {
		CFastPlaneARGB8888 p;
		p.SetRGB((BYTE)(rgb & 0xff),(BYTE)((rgb>>8) & 0xff),(BYTE)((rgb>>16)& 0xff));
		p.SetA((BYTE)((rgb>>24)&0xff));
		dw = p.GetRGBA(); break; }
	case 13: {
		CFastPlaneABGR8888 p;
		p.SetRGB((BYTE)(rgb & 0xff),(BYTE)((rgb>>8) & 0xff),(BYTE)((rgb>>16)& 0xff));
		p.SetA((BYTE)((rgb>>24)&0xff));
		dw = p.GetRGBA(); break; }
	default:	//	????A?A?s???T?[?t?F?[?X???D?D
		return -1; break;
	}

	return 0;
}

/*
void	ISurface::ClearMSB(){
	//	?r?f?I?J?[?h??o?O???????o?C?g????

	int nType = GetSurfaceType();
	if (nType == 7 || nType == 8){
		if (GetSurfaceInfo()->Lock()!=0) return ;
		CFastPlaneEffect::Effect(
			CFastPlaneARGB8888(),GetSurfaceInfo(),
			CFastPlaneClearAlpha(),
			NULL);
		GetSurfaceInfo()->Unlock();
	}
}
*/

int	ISurface::GetYGASurfaceType(int nSurfaceType) const{
	switch(nSurfaceType){
	case 2: nSurfaceType = 11; break;
	case 3: nSurfaceType = 10; break;
	case 4: nSurfaceType = 11; break;
	case 5: nSurfaceType = 12; break;
	case 6: nSurfaceType = 13; break;
	case 7: nSurfaceType = 12; break;
	case 8: nSurfaceType = 13; break;
	//	???T?[?t?F?[?X?????????A?????????????B
	}
	return nSurfaceType;
}

LRESULT	ISurface::LoadByType(const string&strBitmapName,int nSurfaceType){
/**
	??????\?b?h??ACSurfaceInfo?????????????????????????????A
	???????????A???T?C?Y???????????????????
	IGraphicLoader????????A?????�????????????????
	????????????B???????????A????T?C?Y??T?[?t?F?[?X??
	??????????AIGraphicLoader??n????A?????rendering?????\?b?h??
	CSurfaceInfo????????????o?????A?????????\?b?h??
	???????????????B
*/
	//	?P?D???t?@?C?????????
	smart_ptr<IGraphicLoader> loader(IGraphicLoader::GetGraphicFactory()->CreateInstance());
	CFile file;
	if (file.Read(strBitmapName)!=0) return -1;
	//	????????s??????[

	if (loader->LoadPicture(file)!=0) return -2;

	if (loader->IsYGA()) {
		nSurfaceType = GetYGASurfaceType(nSurfaceType);
		//	???T?[?t?F?[?X???????I
	}

	//	?Q?D?T?[?t?F?[?X???
	LONG sx,sy;
	{
		loader->GetSize(sx,sy);
		if (CreateSurfaceByType(sx,sy,nSurfaceType)!=0) return -3;
	}

	//	?R?D?t?@?C??????e???A?T?[?t?F?[?X????f??????
	{
		CSurfaceLockerGuard locker(GetSurfaceInfo());
		GetSurfaceInfo()->Lock();
		if (loader->Render(GetSurfaceInfo())==0) return 0;
	}

	//	????rendering????s????????????????D?D
	//	HDC?o?R??`?????H

	//	CDIBitmap?????A?????o?R??`?????D?D?H
	//	GetDC()

	CDIBitmap dib;
	if (dib.CreateSurface(sx,sy)!=0) return -4;
	LRESULT lr;
	{
		HDC hdc = dib.GetDC();
		if (loader->Render(hdc)!=0) {
			// ?G???[???^?[??????????T?[?t?F?[?X??N???A???????????
			Clear();	return -5;
		}

		//	lr = dib.GetSurfaceInfo()->ClearMSB();
		//	DIB??24bpp surface???????S?~???????????

		#pragma warning(disable:4238)
		lr = GetSurfaceInfo()->GeneralBlt(
			CSurfaceInfo::eSurfaceBltFast,
			dib.GetSurfaceInfo(),
			&CSurfaceInfo::CBltInfo()
			//	???e???|?????I?u?W?F?N?g????n????A
			//	????????????Q??????????Z?[?t
		);
		#pragma warning(default:4238)
	}
	return lr;
}

//////////////////////////////////////////////////////////////////////////////

LRESULT CSurfaceInfo::Save(const string& strBitmapFileName,LPCRECT lpRect/*=NULL*/){
	///	????t?@?C?????????o??(lpRect??????o?????BNULL????S??)
	RECT	rc = GetClipRect(lpRect);

	int sx = rc.right-rc.left;
	int sy = rc.bottom-rc.top;
	if (sx<=0 || sy<=0) return -1; // ???l?n???????R(`?D?L)?m

	//	?r?b?g?}?b?v???????m??
	long lDstPitch = ((rc.right - rc.left)*3 + 3) & ~3;

	// ???C?????????o?C?g??
	long size = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)
		+lDstPitch * (rc.bottom - rc.top);
		//	???????t?@?C???T?C?Y?i?????j

	smart_ptr<BYTE> bitmap;
	bitmap.AddArray(size);

	//	?r?b?g?}?b?v?w?b?_?[???`
	BITMAPFILEHEADER &BF = *(BITMAPFILEHEADER*)bitmap.get();
	BF.bfType			= 0x4D42;
	BF.bfSize			= size;
	BF.bfReserved1		= 0;
	BF.bfReserved2		= 0;
	BF.bfOffBits		= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	
	BITMAPINFOHEADER &BI = *(BITMAPINFOHEADER*)(bitmap.get() + sizeof(BITMAPFILEHEADER));
	BI.biSize			= sizeof(BITMAPINFOHEADER);
	BI.biWidth			= sx;	// size to be saved...
	BI.biHeight			= sy;
	BI.biPlanes			= 1;
	BI.biBitCount		= 24;		//	?t???J???[
	BI.biCompression	= 0;		//	???k
	BI.biSizeImage		= 0;		//	???k??????0
	BI.biXPelsPerMeter	= 3780;	//	96dpi(??????????????\?t?g????????????)
	BI.biYPelsPerMeter	= 3780;
	BI.biClrUsed		= 0;
	BI.biClrImportant	= 0;

	//	?C???[?W?]??????I
	BYTE* pSrcPtr	= (BYTE*)GetPtr();
	LONG lSrcPitch	= GetPitch();
	int nSrcSurfaceType = GetSurfaceType();
	BYTE *pDstPtr = bitmap.get() + sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);

	for(int y=rc.bottom-1,y0=0;y>=rc.top;y--,y0++){
	//	??????]??????K?v??????
		void* pSrc = (void*)((BYTE*)(pSrcPtr+	y	* lSrcPitch));
		void* pDst = (void*)((BYTE*)(pDstPtr+	y0	* lDstPitch));
		CSurfaceBltter::Blt1Line(pSrc,nSrcSurfaceType,pDst,5,sx);
			//	src Surface -> RGB888
		//	???????ADWORD??A???C???????????o????E??]????
		//	?O??????K?v?????????C??????
		//	???????C?????K?v??????D?D???H
	}
	return CFile().Write(strBitmapFileName,bitmap.get(),size);	//	??????:p
}

LRESULT CSurfaceInfo::SaveYGA(const string& strBitmapFileName,LPCRECT lpRect/*=NULL*/,bool bCompress/*=true*/){
	///	YGA???`?????t?@?C???????o??(lpRect??????o?????BNULL????S??)
	///	bCompress==true????CLZSS????k???????B?f?B?t?H???g??true

	///	????t?@?C?????????o??(lpRect??????o?????BNULL????S??)
	RECT	rc = GetClipRect(lpRect);

	int sx = rc.right-rc.left;
	int sy = rc.bottom-rc.top;
	if (sx<=0 || sy<=0) return -1; // ???l?n???????

	int nSrcSurfaceType = GetSurfaceType();
	smart_ptr<BYTE> pSrcCopyed;	//	?R?s?[?????\?[?X???
	{
		bool	bAllScreen	//	?S?????????????H
			= (GetSize().cx==sx && GetSize().cy==sy);
		if (bAllScreen && GetSurfaceType()==12){
			//	?S???????�AARGB8888???!?
			pSrcCopyed.Set((BYTE*)GetPtr());
		} else {
			pSrcCopyed.AddArray(4*sx*sy);
			//	??????ARGB8888?T?[?t?F?[?X??

			BYTE* pSrcPtr	= (BYTE*)GetPtr() + GetPixelSize()*rc.left;
			LONG  lSrcPitch = GetPixelSize()*sx;
			BYTE* pDstPtr	= pSrcCopyed.get();
			LONG  lDstPitch = 4*sx;
			for(int y=rc.top,y0=0;y<rc.bottom;y++,y0++){
				void* pSrc = (void*)((BYTE*)(pSrcPtr+	y	* lSrcPitch));
				void* pDst = (void*)((BYTE*)(pDstPtr+	y0	* lDstPitch));
				CSurfaceBltter::Blt1Line(pSrc,nSrcSurfaceType,pDst,12,sx);
			}
		}
	}

	//	???k???|????A?w?b?_?[????t?^????
	CLZSS lzss;
	BYTE* lpDst;
	DWORD dwDstSize;
	bool bDelete;
	DWORD nSize = sx * sy * 4;
	if (! bCompress || lzss.Encode(pSrcCopyed.get(),lpDst,nSize,dwDstSize)!=0 ) {
		// ???k????? or ???k????????[??
		lpDst = (BYTE*)GetPtr();
		dwDstSize = nSize;
		bDelete = false;
	} else {
		bDelete = true;
	}

	//	?w?b?_?[????
	CYGAHeader header;
	header.dwSizeX = sx;
	header.dwSizeY = sy;
	header.bCompress = (DWORD) bDelete;
	header.dwOriginalSize  = nSize;
	header.dwCompressSize = dwDstSize;

	nSize = sizeof(CYGAHeader) + dwDstSize;
	smart_ptr<BYTE> writebuf;

	writebuf.AddArray(nSize);
	::CopyMemory(writebuf.get(),&header,sizeof(CYGAHeader));
	::CopyMemory(writebuf.get()+sizeof(CYGAHeader),lpDst,dwDstSize);	
	if (bDelete) DELETEPTR_SAFE(lpDst);

	return CFile().Write(strBitmapFileName,writebuf.get(),nSize);
}

//	????g???????\????
namespace {
	struct GeneralBltTmp {
		CSurfaceInfo::CBltInfo bltinfo;
		//	deep copy???????????????
		DWORD dw[2];
		POINT	dstPoint;
		SIZE	dstSize;
		RECT	srcRect;
		RECT	dstClip;
	};
}
smart_ptr<void> CSurfaceInfo::getWrappedPtr(
	CSurfaceInfo::EBltType type,CSurfaceInfo::CBltInfo*&pInfo,DWORD*&pAdditionalParameter)
{
	//	??????n?????p?????[?^??A
	//	callback??????valid???????????R?s?[??p?????K?v??????
	GeneralBltTmp* tmp = new GeneralBltTmp;
	tmp->bltinfo = *pInfo;
	//	pSrc??lpDst???????w??????q???????
	pInfo = &tmp->bltinfo;

	//	deep copy
	if (tmp->bltinfo.pDstPoint != NULL){
		tmp->dstPoint = *tmp->bltinfo.pDstPoint;
		tmp->bltinfo.pDstPoint = &tmp->dstPoint;
	}
	if (tmp->bltinfo.pDstSize != NULL){
		tmp->dstSize = *tmp->bltinfo.pDstSize;
		tmp->bltinfo.pDstSize = &tmp->dstSize;
	}
	if (tmp->bltinfo.pSrcRect != NULL){
		tmp->srcRect = *tmp->bltinfo.pSrcRect;
		tmp->bltinfo.pSrcRect = &tmp->srcRect;
	}
	if (tmp->bltinfo.pDstClip != NULL){
		tmp->dstClip = *tmp->bltinfo.pDstClip;
		tmp->bltinfo.pDstClip = &tmp->dstClip;
	}

	switch (type){
		//	1?p?????[?^
	case eSurfaceBltAddColorFastFade:
	case eSurfaceBltSubColorFastFade:
	case eSurfaceBltAddColorFade:
	case eSurfaceBltSubColorFade:
	case eSurfaceBlendBltFast:
	case eSurfaceBlendBlt:
		tmp->dw[0] = pAdditionalParameter[0];
		pAdditionalParameter = &tmp->dw[0];
		break;

		//	2?p?????[?^
	case eSurfaceBlendConstBltFast:
	case eSurfaceBlendConstBlt:
		tmp->dw[0] = pAdditionalParameter[0];
		tmp->dw[1] = pAdditionalParameter[1];
		pAdditionalParameter = &tmp->dw[0];
		break;
	}

	return smart_ptr<void>(tmp);
}

namespace {
	struct GeneralMorphTmp {
		CSurfaceInfo::CMorphInfo morphinfo;
		//	deep copy???????????????
		DWORD dw[2];
		POINT srcPoint;	///	?]???????W??
		POINT dstPoint;	///	?]??????W??
		RECT clipRect;		///	?]????N???b?v??`
	};
}

smart_ptr<void> CSurfaceInfo::getWrappedPtr(CSurfaceInfo::EBltType type,
											CSurfaceInfo::CMorphInfo*& pInfo,
											DWORD*& pAdditionalParameter)
{
	//	??????n?????p?????[?^??A
	//	callback??????valid???????????R?s?[??p?????K?v??????
	GeneralMorphTmp* tmp = new GeneralMorphTmp;
	tmp->morphinfo = *pInfo;
	//	pSrc??lpDst???????w??????q???????
	pInfo = &tmp->morphinfo;

	//	deep copy
	if (tmp->morphinfo.lpSrcPoint != NULL){
		tmp->srcPoint = *tmp->morphinfo.lpSrcPoint;
		tmp->morphinfo.lpSrcPoint = &tmp->srcPoint;
	}
	if (tmp->morphinfo.lpDstPoint != NULL){
		tmp->dstPoint = *tmp->morphinfo.lpDstPoint;
		tmp->morphinfo.lpDstPoint = &tmp->dstPoint;
	}
	if (tmp->morphinfo.lpClipRect != NULL){
		tmp->clipRect = *tmp->morphinfo.lpClipRect;
		tmp->morphinfo.lpClipRect = &tmp->clipRect;
	}

	switch (type){
		//	1?p?????[?^
	case eSurfaceBltAddColorFastFade:
	case eSurfaceBltSubColorFastFade:
	case eSurfaceBltAddColorFade:
	case eSurfaceBltSubColorFade:
	case eSurfaceBlendBltFast:
	case eSurfaceBlendBlt:
		tmp->dw[0] = pAdditionalParameter[0];
		pAdditionalParameter = &tmp->dw[0];
		break;

		//	2?p?????[?^
	case eSurfaceBlendConstBltFast:
	case eSurfaceBlendConstBlt:
		tmp->dw[0] = pAdditionalParameter[0];
		tmp->dw[1] = pAdditionalParameter[1];
		pAdditionalParameter = &tmp->dw[0];
		break;
	}

	return smart_ptr<void>(tmp);
}

namespace {
	struct GeneralEffectTmp {
		//	deep copy???????????????
		DWORD dw[2];	
		RECT rect;
	};
}

smart_ptr<void> CSurfaceInfo::getWrappedPtr(CSurfaceInfo::EEffectType type,
											LPCRECT& prc,
											DWORD*& pAdditionalParameter)
{
	GeneralEffectTmp* tmp = new GeneralEffectTmp;
	//	rect???w?????????A?????R?s?[??p????????????w?????????
	if (prc!=NULL) {
		tmp->rect = *prc;
		prc = &tmp->rect;
	}

	switch (type){
		//	1?p?????[?^
	case eSurfaceFade:
	case eSurfaceAddColor:
	case eSurfaceSubColor:
	case eSurfaceMosaic:
		tmp->dw[0] = pAdditionalParameter[0];
		pAdditionalParameter = &tmp->dw[0];
		break;
	}

	return smart_ptr<void>(tmp);
}


} // end of namespace Draw
} // end of namespace yaneuraoGameSDK3rd
