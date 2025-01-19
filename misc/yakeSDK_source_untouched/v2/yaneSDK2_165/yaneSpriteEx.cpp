#include "stdafx.h"
#include "yaneSpriteEx.h"

//////////////////////////////////////////////////////////////////////////////

void	CSpriteEx::Blt(CMapLayer*lpMap){
	lpMap->Blt(this);
}

void	CSpriteEx::Blt(CMapLayer*lpMap,int x,int y){
	lpMap->Blt(this,x,y);
}

void	CSpriteEx::BltFix(CMapLayer*lpMap){
	lpMap->BltFix(this);
}

void	CSpriteEx::BltFix(CMapLayer*lpMap,int x,int y){
	lpMap->BltFix(this,x,y);
}

void	CSpriteEx::BltOnce(CMapLayer*lpMap,int x,int y){
	lpMap->BltOnce(this,x,y);
}

//////////////////////////////////////////////////////////////////////////////

#ifdef USE_DirectDraw
void	CSpriteEx::Blt(CDirectDraw*lpDraw,LPRECT lpClip){
	lpDraw->Blt(this,lpClip);
}

void	CSpriteEx::Blt(CDirectDraw*lpDraw,int x,int y,LPRECT lpClip){
	lpDraw->Blt(this,x,y,lpClip);
}

void	CSpriteEx::BltFix(CDirectDraw*lpDraw,LPRECT lpClip){
	lpDraw->BltFix(this,lpClip);
}

void	CSpriteEx::BltFix(CDirectDraw*lpDraw,int x,int y,LPRECT lpClip){
	lpDraw->BltFix(this,x,y,lpClip);
}

void	CSpriteEx::BltOnce(CDirectDraw*lpDraw,int x,int y,LPRECT lpClip){
	lpDraw->BltFix(this,x,y,lpClip);
}
#endif

//////////////////////////////////////////////////////////////////////////////
#ifdef USE_DIB32
void	CSpriteEx::Blt(CDIBDraw*lpDraw,LPRECT lpClip){
	lpDraw->Blt(this,lpClip);
}

void	CSpriteEx::Blt(CDIBDraw*lpDraw,int x,int y,LPRECT lpClip){
	lpDraw->Blt(this,x,y,lpClip);
}

void	CSpriteEx::BltFix(CDIBDraw*lpDraw,LPRECT lpClip){
	lpDraw->BltFix(this,lpClip);
}

void	CSpriteEx::BltFix(CDIBDraw*lpDraw,int x,int y,LPRECT lpClip){
	lpDraw->BltFix(this,x,y,lpClip);
}
void	CSpriteEx::BltOnce(CDIBDraw*lpDraw,int x,int y,LPRECT lpClip){
	lpDraw->BltFix(this,x,y,lpClip);
}
#endif
//////////////////////////////////////////////////////////////////////////////

void	CSpriteEx::Blt(CPlaneBase*lpPlane,LPRECT lpClip){
	int x,y;
	GetPos(x,y);
	Blt(lpPlane,x,y,lpClip);
}

void	CSpriteEx::Blt(CPlaneBase*lpPlane,int x,int y,LPRECT lpClip){
	BltFix(lpPlane,x,y,lpClip);
	IncMotion();
}

void	CSpriteEx::BltFix(CPlaneBase*lpPlane,LPRECT lpClip){
	int x,y;
	GetPos(x,y);
	BltFix(lpPlane,x,y,lpClip);
}

void	CSpriteEx::BltFix(CPlaneBase*lpPlane,int x,int y,LPRECT lpClip){
	//	有効か？
	if (!IsEnable()) return ;

	//	CDirectDrawで描画する場合Layer無視
	SSprite* lpSS = GetSSprite();

	int ox,oy;
	GetOffset(ox,oy);
	ox+=x+lpSS->nOx;
	oy+=y+lpSS->nOy;

	//	そのまま委譲してまうとすっか～
	if (!lpSS->bFast) {
		lpPlane->Blt(lpSS->lpPlane,ox,oy,&lpSS->rcRect,NULL,lpClip);
	} else {
		lpPlane->BltFast(lpSS->lpPlane,ox,oy,&lpSS->rcRect,NULL,lpClip);
	}
}

void	CSpriteEx::BltOnce(CPlaneBase*lpPlane,int x,int y,LPRECT lpClip){
	BltFix(lpPlane,x,y,lpClip);
	
	//	ケツになっていたら、それ以上は加算しない
	int n=GetMotion();
	IncMotion();
	if (GetMotion()==0) {
		SetMotion(n);
	}
}
//////////////////////////////////////////////////////////////////////////////
