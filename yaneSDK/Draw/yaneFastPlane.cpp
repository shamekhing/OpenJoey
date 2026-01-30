
#include "stdafx.h"

#ifdef USE_FastDraw

#include <stdexcept>
#include "yaneFastPlane.h"
#include "yaneFastDraw.h"
#include "yaneDirectDraw.h"
#include "yaneDIBitmap.h"
#include "yaneGraphicLoader.h"
#include "../Auxiliary/yaneFile.h"
#include "yaneGTL.h"

namespace yaneuraoGameSDK3rd {
namespace Draw {

//////////////////////////////////////////////////////
//	static members..

ThreadLocal<CFastDraw*> CFastPlane::m_pDefaultFastDraw;

//////////////////////////////////////////////////////

CFastPlane::CFastPlane(CFastDraw* pFastDraw)
{
	m_pFastDraw = NULL;
	SetFastDraw(pFastDraw);

	m_nSizeX	=	0;
	m_nSizeY	=	0;
	m_lpSurface	=	NULL;
	m_lpPalette	=	NULL;

	ResetColorKey();
	m_hDC		=	NULL;

	//	VXeãÉmÛ·éÌ¾I
	m_bUseSystemMemory	=	true;

	m_bYGA			= false;
	m_bYGAUse		= false;
	m_bMySurface	= false;
	m_bSecondary256 = false;
	//	256F[hÈçÎAÊíRGB555ÌT[tF[Xðì¬·éÌ¾ªA
	//	ZJ_¾¯Í256FT[tF[X
	m_bLoad256		= false;
	m_bSecondary256DIB=false;

	//	RGB555ÌMySurfaceÉÎµÄADIB Sectionðgp·éÌ©H
	m_hBitmap		= NULL;
	m_hDC			= NULL;

	//	©®CT[tF[X
	m_bAutoRestore = false;
	//	I[i[h[(RestoreªÄÑo³êÈ¢BPrimary,SecondaryÍ±ê)
	m_bOwnerDraw	= false;
	m_bPrimary		= false;

	m_bNowRestoring	= false;
}

CFastPlane::~CFastPlane(){
	Release();
}

smart_ptr<ISurface> CFastPlane::clone() {
	return smart_ptr<ISurface>(new CFastPlane(GetMyFastDraw()));
}

#ifdef OPENJOEY_ENGINE_FIXES
smart_ptr<ISurface> CFastPlane::cloneFull() {
    // Validate FastDraw instance
    CFastDraw* pFastDraw = GetMyFastDraw();
    if (!pFastDraw) {
        throw std::runtime_error("FastDraw instance is invalid.");
    }

    // Create a new CFastPlane instance
    CFastPlane* pClone = new CFastPlane(pFastDraw);

    // Create a new surface with identical dimensions and type
    pClone->CreateSurface(m_nSizeX, m_nSizeY, m_bYGA);

    if (m_lpSurface) {
        // Ensure surface is restored
        if (m_lpSurface->IsLost()) {
            m_lpSurface->Restore();
        }

        DDSURFACEDESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.dwSize = sizeof(desc);

        // Lock the source surface
        HRESULT hr = m_lpSurface->Lock(NULL, &desc, DDLOCK_WAIT | DDLOCK_READONLY, NULL);
        if (hr == DDERR_WASSTILLDRAWING) {
            while ((hr = m_lpSurface->Lock(NULL, &desc, DDLOCK_WAIT | DDLOCK_READONLY, NULL)) == DDERR_WASSTILLDRAWING)
                ; // Retry until the surface is ready
        }
        if (FAILED(hr)) {
            delete pClone;
            throw std::runtime_error("Failed to lock the source framebuffer surface.");
        }

        // Lock the cloned surface
        DDSURFACEDESC cloneDesc;
        ZeroMemory(&cloneDesc, sizeof(cloneDesc));
        cloneDesc.dwSize = sizeof(cloneDesc);
        hr = pClone->m_lpSurface->Lock(NULL, &cloneDesc, DDLOCK_WAIT, NULL);
        if (FAILED(hr)) {
            m_lpSurface->Unlock(NULL);
            delete pClone;
            throw std::runtime_error("Failed to lock the cloned surface.");
        }

        // Copy surface data row by row
        for (DWORD row = 0; row < desc.dwHeight; row++) {
            memcpy(
                static_cast<BYTE*>(cloneDesc.lpSurface) + row * cloneDesc.lPitch,
                static_cast<BYTE*>(desc.lpSurface) + row * desc.lPitch,
                desc.dwWidth * (desc.ddpfPixelFormat.dwRGBBitCount / 8)
            );
        }

        // Unlock both surfaces
        m_lpSurface->Unlock(NULL);
        pClone->m_lpSurface->Unlock(NULL);
    }

    // Update cloned surface properties
    pClone->UpdateColorKey();
    pClone->SetSystemMemoryUse(m_bUseSystemMemory);

    return smart_ptr<ISurface>(pClone);
}
#endif

LRESULT	CFastPlane::Release(){
	//	owner create surface
	//	256F[hÌ¼zZJ_ÍDIBÆµÄì¬µÄ½ñ©H
	if (m_hBitmap){
		m_bMySurface = false;
		if (m_hDC!=NULL){
			::DeleteDC(m_hDC);
			m_hDC = NULL;
		}
		if (m_hBitmap!=NULL){
			::DeleteObject(m_hBitmap);
			m_hBitmap = NULL;
		}
	}

	if (m_bMySurface) {
		switch (GetSurfaceType()){
		case 3: {
			CFastPlaneRGB565* pwd = (CFastPlaneRGB565*)GetSurfaceInfo()->GetPtr();
			delete [] pwd; } break;
		case 4: {
			CFastPlaneRGB555* pwd = (CFastPlaneRGB555*)GetSurfaceInfo()->GetPtr();
			delete [] pwd; } break;
		case 5: {
			CFastPlaneRGB888* pwd = (CFastPlaneRGB888*)GetSurfaceInfo()->GetPtr();
			delete [] pwd; } break;
		case 6: {
			CFastPlaneBGR888* pwd = (CFastPlaneBGR888*)GetSurfaceInfo()->GetPtr();
			delete [] pwd; } break;
		case 7: {
			CFastPlaneXRGB8888* pwd = (CFastPlaneXRGB8888*)GetSurfaceInfo()->GetPtr();
			delete [] pwd; } break;
		case 8: {
			CFastPlaneXBGR8888* pwd = (CFastPlaneXBGR8888*)GetSurfaceInfo()->GetPtr();
			delete [] pwd; } break;
		case 10: {
			CFastPlaneARGB4565* pwd = (CFastPlaneARGB4565*)GetSurfaceInfo()->GetPtr();
			delete [] pwd; } break;
		case 11: {
			CFastPlaneARGB4555* pwd = (CFastPlaneARGB4555*)GetSurfaceInfo()->GetPtr();
			delete [] pwd; } break;
		case 12: {
			CFastPlaneARGB8888* pdw = (CFastPlaneARGB8888*)GetSurfaceInfo()->GetPtr();
			delete [] pdw; } break;
		case 13: {
			CFastPlaneABGR8888* pdw = (CFastPlaneABGR8888*)GetSurfaceInfo()->GetPtr();
			delete [] pdw; } break;
		default:
			Err.Out("CFastPlane::ReleaseÅs¾T[tF[XÌ[X");
		}
		m_bMySurface = false;
	}

	RELEASE_SAFE(m_lpSurface);
	RELEASE_SAFE(m_lpPalette);
	m_strBitmapFile.erase();
	m_nSizeX	=	0;
	m_nSizeY	=	0;
	m_bLoad256	= false;
	GetSurfaceInfo()->SetInit(false);	//	T[tF[Xîñàú»

	if (!m_bNowRestoring && GetMyFastDraw()!=NULL){
		GetMyFastDraw()->GetFastPlaneList()->erase(this);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////

LRESULT	CFastPlane::GetSize(int &x,int &y) const {
	x = m_nSizeX;
	y = m_nSizeY;
	return const_cast<CFastPlane*>(this)->GetSurfaceInfo()->IsInit()?0:1;
}

//	T[tF[XÌXgÉÎ·éA
LRESULT	CFastPlane::Restore(){
	LRESULT lr = 0;
	m_bNowRestoring = true;	//	XgAÉÂ«AReleaseÅtbNð·éÌÖ~I
	int nType = GetSurfaceInfo()->GetSurfaceType();
	if (nType!=0){	//	Surfaceª¶Ý·é©Ç¤©ÍA±¢ÂÅ»è·éKvª é
		//	VXeãÌT[tF[XÍLostµÈ¢Ì¾ªOOG
		//	»ÝÌæÊbppÆÙÈéT[tF[XÈçÎAðÌµÄìèÈ¨·Kvª é
		bool bRestore = false;
		if (m_lpSurface!=NULL && m_lpSurface->IsLost()) {
			//	&& _ZqÍA¶©çEÖÌ]¿ðÛØ·éµA
			//	VCÌÅK»ÉæèA¶ª¬§ÉEª]¿³êÈ¢ÌàÛØ³êé
			bRestore = true;
		} else {
		//	»ÝÌæÊbppÆRp`Å³¢T[tF[XÈçÎXgA·é
			int nType2 = GetMyFastDraw()->GetPrimary()->GetSurfaceType();
			switch (nType2){
			case 2:	//	256FÌÆ«ÍARGB555ÆÝÈ·
				if (nType==2 && m_bSecondary256) break; // 256FpZJ_âñH
				if (nType==4 || nType == 11) break;
				bRestore = true; break;
			case 3:
				if (nType==3 || nType == 10) break;
				bRestore = true; break;
			case 4:
				//	256F[hÌÉÇÝñ¾rbg}bvÈñ©MpÅ«È¢
				if (m_bLoad256 && IsLoaded()) {
					bRestore = true; break;
				}
				if (nType==4 || nType == 11) break;
				bRestore = true; break;
			case 5:
			case 6:
			case 7:
			case 8:
				if (nType==nType2 || nType == 12) break;
				bRestore = true; break;
			}
		}

		if (bRestore){
			if (m_bAutoRestore){
			//	©®CT[tF[XÈçÎAT[tF[Xðì¬ãA»¢ÂðRs[
				LRESULT lrErr = 0;
				if (m_nSizeX!=0 && m_nSizeY!=0){
					CFastPlane plane;
					plane.CreateSurface(m_nSizeX,m_nSizeY,IsYGA());
					lrErr = plane.BltFast(this,0,0);
					// ±ÌÏ·Â\âÁ½©H
					if (lrErr==0){
						InnerCreateSurface(m_nSizeX,m_nSizeY,m_bYGA,m_bSecondary256);
						lrErr = BltFast(&plane,0,0);
					}
				}
				//	]É¸sµ½çAµá[È¢©ç©OÅXgAµÄñ©DD
				if (lrErr!=0) goto RestoreRetry;

				//	»ÝÌæÊ[hÉí¹ÄColorKeyðÝèµÈ¨·
				UpdateColorKey();
			} else {
RestoreRetry:;
				//	rbg}bvt@CÈçÎ»êð³·é
				if (IsLoaded()){
					lr = InnerLoad(m_strBitmapFile);
				} else {
					//	{ÍA±ÌÆ«AÏ·qðpÓµÄÏ·Ù¤ªÇ¢
					//	µ©µA³Ì·×ÄÌgÝí¹ÍcåÅ éDD
					//	êxAARGB8888ÉÏ·µÄA»±©ç^[Qbg^É
					//	ß¹ÎA»¤Åà³¢Ì¾ªDD

					if (m_nSizeX!=0 && m_nSizeY!=0){
						//	Æè ¦¸AT[tF[XìèÈ¨µÆ­©DD
						InnerCreateSurface(m_nSizeX,m_nSizeY,m_bYGA,m_bSecondary256);
					}
				}
				//	»ÝÌæÊ[hÉí¹ÄColorKeyðÝèµÈ¨·
				UpdateColorKey();
				//	I[i[h[©àmêÈ¢ÌÅA»êð³·é
				lr |= OnDraw();	//	Ï÷·é
			}
		}
	}
	m_bNowRestoring = false;	//	XgAI¹ÉÂ«AReleaseÅtbNð·éÌðI
	return lr;
}


LRESULT CFastPlane::Load(const string& strBitmapFileName){
	Release();
	ResetColorKey();
	//  ÆÅRestoreÅ«éæ¤Ét@C¼ði[µÄ¨­B
	LRESULT lr = InnerLoad(strBitmapFileName);
	m_strBitmapFile = strBitmapFileName;
	return lr;
}

//	t@C¼ðÔ·
string	CFastPlane::GetFileName() const{
	return m_strBitmapFile;
}

////////////////////////////////////////////////////////////////////////////////////

//	rbg}bvÌàIÈ[hBi[t@C¼ÉÍe¿µÈ¢
LRESULT	CFastPlane::InnerLoad(const string& strFileName){
/**
	ISurface::LoadªÀ³êÄ¢éÌÅAî{IÈÇÝÝÍA
	T[tF[Xì¬ãÉAISurface::LoadÉÏ÷·êÎÇ¢
*/

	/**
		InnerLoadÍA
		rbg}bvt@C¼A²«FÌwèðjóµÈ¢±ÆðÛØ·é
		»Ì½ßReleaseOÉ·×ÄÌÝèðÛ¶·é
	*/
	string strBitmap(m_strBitmapFile);
	//	colorkeyÌÝèàÛ¶

	bool bUsePosColorKey = m_bUsePosColorKey;
	int	nColorKeyX = m_nColorKeyX, nColorKeyY = m_nColorKeyY;
	ISurfaceRGB rgbColorKey = m_rgbColorKey;
	{
		Release();	//	ðÌ
	}
	m_strBitmapFile = strBitmap;
	m_bUsePosColorKey = bUsePosColorKey;
	m_nColorKeyX = nColorKeyX; m_nColorKeyY = nColorKeyY;
	m_rgbColorKey = rgbColorKey;


	//	#notdefined#ÈçÎA³íI¹µ½ÆµÄAé
	if (strFileName == "#notdefined#"){
		return 0;
	}

	//	nSurfaceTypeÌT[tF[Xðì¬µA»±ÉÇÝÞ

	int nSurfaceType = GetMyFastDraw()->GetPrimary()->GetSurfaceType();

	m_bLoad256 = false;
	if (m_bYGAUse){
	//	¿T[tF[Xðì¬·ñÌ©H
		nSurfaceType = GetYGASurfaceType(nSurfaceType);
		//	Î·é¿T[tF[XÌÔð¾é
	} else {
		switch(nSurfaceType){
		case 2: nSurfaceType = 4; m_bLoad256=true; break;
		// ---- 8bppÈçÎ RGB555Åì¬B
		//	±ÌÆ«ÇÝñ¾rbg}bvÍA
		//	¼ÌæÊ[hÉÚsµ½Æ«ÉXgA·×«
		}
	}

	LRESULT lr = ISurface::LoadByType(strFileName,nSurfaceType);
	if (lr!=0) return lr;

	// ²«FðZbgµÆ±©[
	// ResetColorKey();@©Ü¿ª¢('02/11/01)yane.
	// ²«FðXVµÆ±©[
	UpdateColorKey();

	return lr;
}

HDC CFastPlane::GetDC(){
	if (m_hBitmap!=NULL){
		return m_hDC;
	}
	
	if (m_hDC!=NULL) {
		Err.Out("CFastPlane::EndPaintªÄÑo³êÄ¢È¢ÌÉBeginPaintªÄÑo³ê½");
		return NULL;
	}
	if (m_lpSurface==NULL) return NULL;
	if (m_lpSurface->GetDC(&m_hDC)!=DD_OK) return NULL;
	return m_hDC;
}

void CFastPlane::ReleaseDC(){
	if (m_hBitmap!=NULL){
		return ;
	}

	if (m_hDC==NULL) {
		Err.Out("CFastPlane::BeginPaintªÄÑo³êÄ¢È¢ÌÉEndPaintªÄÑo³ê½");
		return ;
	}
	if (m_lpSurface==NULL) return ;
	if (m_lpSurface->ReleaseDC(m_hDC)!=DD_OK) return ;
	m_hDC = NULL;
}

LRESULT CFastPlane::InnerCreateMySurface(int sx,int sy,int nSurfaceType,bool bClear/*=true*/){

	m_nSizeX = sx; m_nSizeY = sy;
	SIZE rc = { m_nSizeX,m_nSizeY };

	switch (nSurfaceType){
	case 4:	{ //	RGB555ðì¬
		CFastPlaneRGB555* lpSurface;
		if (m_bSecondary256DIB){
			//	256FpÌ¼zZJ_Å éÈçÎADIBSectionÅìÁÄ¨­
			//	(HDCðæ¾Å«éæ¤É·é½ß)
			BITMAPINFO bmi;
			ZERO(bmi);
			bmi.bmiHeader.biSize		= sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biWidth		=  m_nSizeX;
			bmi.bmiHeader.biHeight		= -m_nSizeX;
			bmi.bmiHeader.biPlanes		= 1;
			bmi.bmiHeader.biBitCount	= 16;
			bmi.bmiHeader.biCompression	= BI_RGB;
			HDC hdc = ::GetDC(NULL); // hWndÌÙ¤ª¦¦ñ©H
			m_hBitmap = ::CreateDIBSection(hdc /* NULL*/, &bmi , DIB_RGB_COLORS, (void**)&lpSurface, NULL, 0 );
			//	ª±êÅìçêéDIBÍRGB555Ìæ¤¾ªH
			::ReleaseDC(NULL,hdc);
			if (m_hBitmap==NULL) return 1;
			m_hDC = ::CreateCompatibleDC(NULL);
//			m_hDC = ::CreateCompatibleDC(::GetDC(NULL));
			::SelectObject(m_hDC,m_hBitmap);
		} else {
			lpSurface = new CFastPlaneRGB555[m_nSizeX * m_nSizeY];
		}
		GetSurfaceInfo()->Init((void*)lpSurface,m_nSizeX*sizeof(CFastPlaneRGB555),rc);
		break;
				}
		//	±ÌÆ«AHDCÍæ¾Å«È¢BSl
	case 3:	{ //	RGB565ðì¬
		CFastPlaneRGB565* lpSurface = new CFastPlaneRGB565[m_nSizeX * m_nSizeY];
		GetSurfaceInfo()->Init((void*)lpSurface,m_nSizeX*sizeof(CFastPlaneRGB565),rc);
		break;
				}
	case 5: { //	RGB888ðì¬
		CFastPlaneRGB888* lpSurface = new CFastPlaneRGB888[m_nSizeX * m_nSizeY];
		GetSurfaceInfo()->Init((void*)lpSurface,m_nSizeX*sizeof(CFastPlaneRGB888),rc);
		break;
			}
	case 6: { //	BGR888ðì¬
		CFastPlaneBGR888* lpSurface = new CFastPlaneBGR888[m_nSizeX * m_nSizeY];
		GetSurfaceInfo()->Init((void*)lpSurface,m_nSizeX*sizeof(CFastPlaneBGR888),rc);
		break;
			}
	case 7: { //	XRGB8888ðì¬
		CFastPlaneXRGB8888* lpSurface = new CFastPlaneXRGB8888[m_nSizeX * m_nSizeY];
		GetSurfaceInfo()->Init((void*)lpSurface,m_nSizeX*sizeof(CFastPlaneXRGB8888),rc);
		break;
			}
	case 8: { //	XBGR8888ðì¬
		CFastPlaneXBGR8888* lpSurface = new CFastPlaneXBGR8888[m_nSizeX * m_nSizeY];
		GetSurfaceInfo()->Init((void*)lpSurface,m_nSizeX*sizeof(CFastPlaneXBGR8888),rc);
		break;
			}

	case 10:	{ //	ARGB4565ðì¬
		CFastPlaneARGB4565* lpSurface = new CFastPlaneARGB4565[m_nSizeX * m_nSizeY];
		GetSurfaceInfo()->Init((void*)lpSurface,m_nSizeX*sizeof(CFastPlaneARGB4565),rc);
		break;
				}
	case 11: { //	ARGB4555ðì¬
		CFastPlaneARGB4555* lpSurface = new CFastPlaneARGB4555[m_nSizeX * m_nSizeY];
		GetSurfaceInfo()->Init((void*)lpSurface,m_nSizeX*sizeof(CFastPlaneARGB4555),rc);
		break;
				}
	case 12:	//	ARGB8888
			{ //	ARGB8888ðì¬
		CFastPlaneARGB8888* lpSurface = new CFastPlaneARGB8888[m_nSizeX * m_nSizeY];
		GetSurfaceInfo()->Init((void*)lpSurface,m_nSizeX*sizeof(CFastPlaneARGB8888),rc);
		break;
				}
	case 13:	//	ABGR8888
			{ //	ABGR8888ðì¬
		CFastPlaneABGR8888* lpSurface = new CFastPlaneABGR8888[m_nSizeX * m_nSizeY];
		GetSurfaceInfo()->Init((void*)lpSurface,m_nSizeX*sizeof(CFastPlaneABGR8888),rc);
		break;
				}
	default:
		return 1;	//	T|[gµÄÖñÅOOG
	}

	m_bYGA = (nSurfaceType>=10);

	GetSurfaceInfo()->SetSurfaceType(nSurfaceType);
	m_bMySurface = true; // ©ªÅnewµ½T[tF[XÅ é±ÆðÓ¡·é}[J[

	//	NAtO§ÁÄÈ¢ÈçNAµÈ¢
	if (bClear) Clear();

	//	hook Jn·é
	if (!m_bNowRestoring && GetMyFastDraw()!=NULL){
		GetMyFastDraw()->GetFastPlaneList()->insert(this);
	}
	return 0;
}

LRESULT CFastPlane::CreateSurfaceByType(int sx,int sy,int nType){
	return InnerCreateMySurface(sx,sy,nType,false);
}

LRESULT CFastPlane::InnerCreateSurface(int sx,int sy,bool bYGA,bool bSecondary256){
	Release();

	if (sx==0 || sy==0) {
		return 1;	//	±ñÈT[tF[XA¨ÙµÄ­ê[OOG
	}

	//	»ÝÌæÊ[hÉ¶½àÌÉ·éKv è
	int nType = GetMyFastDraw()->GetPrimary()->GetSurfaceType();

	if (!bYGA) {
		//	YGAæÅÍ³¢ÌÅ³ðÅDirectDrawSurface
		//	½¾µA8bppÌÆ«ÍARGB555ÌT[tF[Xðìé
		if (nType == 2 && !bSecondary256) {
			return InnerCreateMySurface(sx,sy,4);
		} else {
			if (bSecondary256) m_bSecondary256 = true;
			//	±ÌT[tF[XÍA256FpZJ_
		}

		///	»ÝÌvC}æèå«ÈT[tF[XÍì¬É¸s·é
		///	(DirectX3Ì§À)
		int nPx,nPy;
		GetMyFastDraw()->GetPrimary()->GetSize(nPx,nPy);
		bool bCreateDirectDrawSurface = !(nPx < sx || nPx < sy);
		if (bCreateDirectDrawSurface) {

			DDSURFACEDESC ddsd;
			ZERO(ddsd);
			ddsd.dwSize = sizeof(ddsd);
			ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
			//	­§IÉVXeðg¤IvV
			if (m_bUseSystemMemory) {
				ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
			} else {
				ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
			}
			// TCYðÛ¶µÄ¨­
			m_nSizeX = ddsd.dwWidth	 = sx;
			m_nSizeY = ddsd.dwHeight = sy;

			//	RGB888ÅAïoCgÅIíÁÄ¢éÆâçµ¢ÌÅA
			//	]ªÉmÛ·éæ¤É·éBdWidthªïÈçÎôÅmÛ·éæ¤É·é
			if (GetMyFastDraw()->GetPrimary()->GetBpp()==24 && ((ddsd.dwWidth & 1)==1)){
				ddsd.dwWidth++;	//	­§IÉôoCgÉAC
			}
			LPDIRECTDRAW lpDraw = GetMyFastDraw()->GetDDraw();
			if (lpDraw==NULL) return 1;
			if (lpDraw->CreateSurface(&ddsd,&m_lpSurface,NULL)!=DD_OK){
				Err.Out("CFastPlane::InnerCreateSurfaceÌCreateSurfaceÉ¸s");
				return 2; //  ¶á[
			}
		} else {
			return InnerCreateMySurface(sx,sy,nType);
		}

		UpdateSurfaceInfo();

		//	m_nSurfaceRef = 1;	//	QÆJEgÌÝèOO
		Clear();	//	OÌ½ßNAµÄ¨­(ÅãÊð0É·é½ß)
	} else {
		//	xf`æÈÌÅA»ÝÌæÊ[hÉ¶½àÌÉ·éKv è
		int nType = GetMyFastDraw()->GetPrimary()->GetSurfaceType();
		switch (nType){
		case 3:	 //	ARGB4565ðì¬
			return InnerCreateMySurface(sx,sy,10);
			break;

		case 2:	 // Ë8bppÌÆ«ÍARGB555ÈÌÅA»êÉÎ·éYGAÍARGB4555
		case 4:	 //	ARGB4555ðì¬
			return InnerCreateMySurface(sx,sy,11);
			break;

		//	RGBÈçÎARGB8888Å¦¦ñ¿á¤H
		case 5:	//	RGB888
		case 7:	//	XRGB8888
			return InnerCreateMySurface(sx,sy,12);
			break;
		//	BGRÈçÎOaÁZÌ±ÆÆ©l¦ÄABGR8888ÉµÆ±©H
		case 6:	//	BGR888
		case 8:	//	XBGR8888
			return InnerCreateMySurface(sx,sy,13);
			break;
		}

		//	NAµÄ¨©È¢Æ¿ÉS~ªcÁ½ÜÜÉÈéDDOOG
		Clear();
	}

	m_bYGA = bYGA;

	//	hook Jn·é
	//	--- ±êçÈOÍAInnerCreateMySurfaceÌÈ©ÅhookµÄ¢éÌÅok
	if (GetMyFastDraw()!=NULL){
		GetMyFastDraw()->GetFastPlaneList()->insert(this);
	}
	return 0;
}

LRESULT CFastPlane::CreateSurface(int sx,int sy,bool bYGA){
	//	ÊÉCreateSurface·éÆA»êÍÔá¢È­I[i[h[v[Å é
//	m_bOwnerDraw	= false;
//	m_bBitmap		= false;
	ResetColorKey();

	LRESULT lr = InnerCreateSurface(sx,sy,bYGA || m_bYGAUse);
	if (lr) return lr;

	UpdateColorKey();

	//	CreateSurfaceµÄéñ¾©çAFillColorÍZbg·×«? '00/09/09
	//	Ëµ©µA±¤µÄµÜ¤ÆAæÊðØèÖ¦½ ÆAFillColorªÏíÁÄµÜ¤DD

//	m_bOwnerDraw	= true;

	return 0;
}

//	vC}T[tF[XÌ¶¬
LRESULT	CFastPlane::CreatePrimary(bool& bUseFlip,int nSx,int nSy){
	m_bPrimary = true;

	Release();
	ResetColorKey();
	LPDIRECTDRAW lpDraw = GetMyFastDraw()->GetDDraw();
	if (lpDraw==NULL) return 1;
	/* if (!IWindow::IsFullScreen()) */
	bUseFlip = false;

sur_retry: ;
	DDSURFACEDESC ddsd;
	ZERO(ddsd);
	ddsd.dwSize = sizeof(ddsd);

	if (bUseFlip) {
		ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE
			| DDSCAPS_FLIP
			| DDSCAPS_COMPLEX;
		ddsd.dwBackBufferCount = 1;
	} else {
		ddsd.dwFlags = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	}
	if (lpDraw->CreateSurface(&ddsd,&m_lpSurface,NULL)!=DD_OK){
		Err.Out("CFastPlane::CreatePrimaryÉ¸s");
		if (bUseFlip) {
			bUseFlip=false;
			// Flipping SurfaceÍrfIãÉzu·éKvª é½ßA
			// CreateÉ~Xµ½Æàl¦çêé
			goto sur_retry;
		}
		return 1; //  ¶á[
	}
	//	±±ÅRs[µÄ¨­
	if (nSx && nSy) {
		m_nSizeX = nSx; m_nSizeY = nSy;	//	TCYÍ±êðÌp
	} else {
		GetMyFastDraw()->GetSize(m_nSizeX,m_nSizeY);
	}

	UpdateSurfaceInfo();

	if (GetSurfaceType() == 2) {
		//	8bppâÁ½çA±êZJ_256¯lA
		//	256F[hâ¯ÇARGB555ÅÍ³­A8bppÆµÄpÓ³ê½
		//	ÁêÈT[tF[XÆµÄ\¿·é
		m_bSecondary256 = true;
	}

	//	±ÌT[tF[XÉÎµÄàAJ[L[ÍÝèµÆ±©DD
	UpdateColorKey();

//	m_nSurfaceRef = 1;		//	QÆJEg«µÄ¨©È¢Æ¤Ü­ðú³êÈ¢
	m_bOwnerDraw = true;	//	±êðOnÉµÈ¢ÆRestore³êÄµÜ¤
	return 0;
}

//	ZJ_T[tF[XÌ¶¬
LRESULT CFastPlane::CreateSecondary(CFastPlane*lpPrimary,bool& bUseFlip){
	Release();
	ResetColorKey();

	LPDIRECTDRAW lpDraw = GetMyFastDraw()->GetDDraw();
	if (lpDraw==NULL) return 1;

	//	«flip[`«©¯I¢T|[gI
sur_retry: ;
	if (bUseFlip) {
		// flipðg¤ÈãAVXeÉobt@ðmÛ·éÌÍÜ¸¢DDD
		DDSCAPS ddscaps;		
		ZERO(ddscaps);	//	¢çÈ¢¯ÇêË
		if (m_bUseSystemMemory) {
			ddscaps.dwCaps = DDSCAPS_BACKBUFFER | DDSCAPS_SYSTEMMEMORY;
		} else {
			ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
		}
		if (lpPrimary->GetSurface()->GetAttachedSurface(&ddscaps,&m_lpSurface)!=DD_OK){
			Err.Out("CFastPlane::CreateSecondaryÌGetAttachedSurfaceÉ¸s");
			// Ùâ©ç¾íñ±Á¿áÈ¢I
			bUseFlip = false;
			goto sur_retry;
		}
		lpPrimary->GetSize(m_nSizeX,m_nSizeY);
		//	êNAµÆ±©...
		Clear();
		//	T[tF[XÌ`FbNXV
		UpdateSurfaceInfo();
	} else {
		int nSizeX,nSizeY;
		lpPrimary->GetSize(nSizeX,nSizeY);
		if (InnerCreateSurface(nSizeX,nSizeY)!=0){
			Err.Out("CFastPlane::CreateSecondaryÉ¸s");
			return 0;
		}
		//	UpdateÆClearÍAInnerCreateSurfaceªÊ|ð©é
	}

	//	±ÌT[tF[XÉÎµÄàAJ[L[ÍÝèµÆ±©DD
	UpdateColorKey();

//	m_nSurfaceRef = 1;		//	QÆJEg«µÄ¨©È¢Æ¤Ü­ðú³êÈ¢
	m_bOwnerDraw = true;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//	Vµ­T[tF[XðìÁ½Æ«ÉÍA±ÌÖðÄÑo·±ÆI
LRESULT	CFastPlane::UpdateSurfaceInfo(){
	int nSurfaceType = CDirectDrawSurfaceManager().GetSurfaceType(GetSurface());

	//	ÀÛÉLockìÆªKvÅ é
	DDSURFACEDESC ddsd = { sizeof (ddsd) };
	LRESULT hres;
	ddsd.dwSize = sizeof(ddsd);
	while ((hres = GetSurface()->Lock(NULL, &ddsd, 0, NULL)) == DDERR_WASSTILLDRAWING)
		;
	SIZE rc = { m_nSizeX,m_nSizeY };

	if (hres != DD_OK) {
		if (IsPrimarySurface()){
			//	primary©àmêñ
			GetSurfaceInfo()->Init(NULL,0,rc,nSurfaceType);
			///	primaryÍALockÉ¸s·é±Æª é©çú»¾¯ÅàµÆ©Éá¢©ñ
			GetSurfaceInfo()->SetLocker(smart_ptr<ISurfaceLocker>(new IDisableSurfaceLocker));
			///	SurfaceÌLockÍK¸¸s·éæ¤ÉµÆ©È­ÄÍ¢©ñ
		} else {
			//	primaryÅÈ¢ÌÉlock¸sµ½ç_Û
			GetSurfaceInfo()->SetInit(false);
		}
		return 1;	//	LockÉ¸s
	}
	GetSurfaceInfo()->Init(ddsd.lpSurface,ddsd.lPitch,rc,nSurfaceType);
	//	ªAhXðÁè·é½ßÉA±êªKv

	//	unlockðYêéÆåÏÈ±ÆÉOOG
	GetSurface()->Unlock(NULL);

	return 0;
}

LPDIRECTDRAWSURFACE CFastPlane::GetSurface(){
	return m_lpSurface;
}

LPDIRECTDRAWPALETTE CFastPlane::GetPalette(){
	return m_lpPalette;
}


LRESULT CFastPlane::SetSystemMemoryUse(bool bEnable){
	m_bUseSystemMemory = bEnable;
	return 0;
}

//////////////////////////////////////////////////////////////////////////////

/*
LRESULT		CFastPlane::SetFillColor(ISurfaceRGB c){
	if (m_lpSurface==NULL) return -1;
	m_FillColor = c;
	return GetSurfaceInfo()->GetMatchColor(c,m_dwFillColor);
}

ISurfaceRGB		CFastPlane::GetFillColor() const {
	return m_FillColor;
}
*/

//////////////////////////////////////////////////////////////////////////////
// todo
// fBXvCÌFð²×éÌÉGetDisplayModeÍgÁÄÍ¢¯È¢
/*
int		CFastPlane::GetBpp(){
	return CBppManager::GetBpp();
}

void	CFastPlane::InnerGetBpp() {
	CBppManager::Reset();
}
*/

//////////////////////////////////////////////////////////////////////////////

} // end of namespace Draw
} // end of namespace yaneuraoGameSDK3rd

#endif // USE_DirectDraw
