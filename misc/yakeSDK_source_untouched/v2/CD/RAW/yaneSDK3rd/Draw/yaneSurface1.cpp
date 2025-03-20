#include "stdafx.h"
#include "yaneSurface.h"
#include "yaneGTL.h"

////////////////////////////////////////////////////////////////////
ISurface::ISurface(){
	m_vSurfaceInfo.Add(new CSurfaceInfo);
}
/////////////////////////////////////////////////////////////////////

RECT	CSurfaceInfo::GetClipRect(const LPRECT lpRect) const{
	RECT r;
	if(lpRect == NULL){
		::SetRect(&r,0,0,GetSize().cx,GetSize().cy);
	} else {
		r = *lpRect;
	}

	// クリッピングする
	RECT lpClip = { 0,0,m_size.cx,m_size.cy };

	if (lpClip.left	 > r.left)	{ r.left   = lpClip.left;	 }
	if (lpClip.right < r.right) { r.right  = lpClip.right;	 }
	if (lpClip.top	 > r.top)	{ r.top	   = lpClip.top;	 }
	if (lpClip.bottom<r.bottom) { r.bottom = lpClip.bottom; }

	//	invalid rect,but..
	//	if (r.left >= r.right || r.top	>= r.bottom) return 1;

	return r;
}

LRESULT CSurfaceInfo::Lock(){
	if (!IsInit()) return -1;	//	初期化自体がされていない
	if (IsLocked()) {
		#ifdef USE_EXCEPTION
			throw CRuntimeException("２重Lock(CSurfaceInfo::Lock)");
		#else
			return 1;
		#endif
	}
	SetLock(true);
	return GetLocker()->Lock();
}

LRESULT CSurfaceInfo::Unlock(){
	if (!IsInit()) return -1;	//	初期化自体がされていない
	if (!IsLocked()) {
		#ifdef USE_EXCEPTION
			throw CRuntimeException("２重UnlockCSurfaceInfo::Unlock");
		#else
			return 1;
		#endif
	}
	SetLock(false);
	return GetLocker()->Unlock();
}

/////////////////////////////////////////////////////////////////////

LRESULT CSurfaceInfo::GetPixel(int x,int y,ISurfaceRGB&rgb) const{
	CSurfaceInfo* pThis = const_cast<CSurfaceInfo*>(this);
	if (pThis->Lock()!=0) return 0;
	CSurfaceLockerGuard guard(pThis->GetLocker());
	//	Unlockは抜けるときに勝手に行なってくれる

	//	範囲外？
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
	default:	//	なんや、、不明サーフェースか．．
		rgb = 0; return -2;
	}
	return 0;
}

LRESULT CSurfaceInfo::GetMatchColor(ISurfaceRGB rgb,DWORD&dw) const {

	int nType = GetSurfaceType();
	switch (nType) {
	case 2:
		//	こんなん無理なんやよねー
		break;
	case 3: {
		CFastPlaneRGB565 p;
		p.SetRGB(rgb & 0xff,(rgb>>8) & 0xff,(rgb>>16)& 0xff);
		dw = p.GetRGB(); break; }
	case 4: {
		CFastPlaneRGB555 p;
		p.SetRGB(rgb & 0xff,(rgb>>8) & 0xff,(rgb>>16)& 0xff);
		dw = p.GetRGB(); break; }
	case 5: {
		CFastPlaneRGB888 p;
		p.SetRGB(rgb & 0xff,(rgb>>8) & 0xff,(rgb>>16)& 0xff);
		dw = p.GetRGB(); break; }
	case 6: {
		CFastPlaneBGR888 p;
		p.SetRGB(rgb & 0xff,(rgb>>8) & 0xff,(rgb>>16)& 0xff);
		dw = p.GetRGB(); break; }
	case 7: {
		CFastPlaneXRGB8888 p;
		p.SetRGB(rgb & 0xff,(rgb>>8) & 0xff,(rgb>>16)& 0xff);
		dw = p.GetRGB(); break; }
	case 8: {
		CFastPlaneXBGR8888 p;
		p.SetRGB(rgb & 0xff,(rgb>>8) & 0xff,(rgb>>16)& 0xff);
		dw = p.GetRGB(); break; }
	case 10: {
		CFastPlaneARGB4565 p;
		p.SetRGB(rgb & 0xff,(rgb>>8) & 0xff,(rgb>>16)& 0xff);
		p.SetA((rgb>>24)&0xff);
		dw = p.GetRGBA(); break; }
	case 11: {
		CFastPlaneARGB4555 p;
		p.SetRGB(rgb & 0xff,(rgb>>8) & 0xff,(rgb>>16)& 0xff);
		p.SetA((rgb>>24)&0xff);
		dw = p.GetRGBA(); break; }
	case 12: {
		CFastPlaneARGB8888 p;
		p.SetRGB(rgb & 0xff,(rgb>>8) & 0xff,(rgb>>16)& 0xff);
		p.SetA((rgb>>24)&0xff);
		dw = p.GetRGBA(); break; }
	case 13: {
		CFastPlaneABGR8888 p;
		p.SetRGB(rgb & 0xff,(rgb>>8) & 0xff,(rgb>>16)& 0xff);
		p.SetA((rgb>>24)&0xff);
		dw = p.GetRGBA(); break; }
	default:	//	なんや、、不明サーフェースか．．
		return -1; break;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////
