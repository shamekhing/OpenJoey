
#include "stdafx.h"
#include "yaneSprite.h"

//////////////////////////////////////////////////////////////////////////////
//	CSprite

void	SSprite::SetPlane(CPlaneBase*lp){
	//	プレーン全域を一つのスプライトとする
	WARNING(lp==NULL,"CSprite::SetPlaneでlp==NULL");

	lpPlane = lp;
	int	sx,sy;
	lpPlane->GetSize(sx,sy);
	::SetRect(&rcRect,0,0,sx,sy);
	nOx = nOy = 0;
	nHeight	= sy;
	bFast	= true;
}

//////////////////////////////////////////////////////////////////////////////
//	CSprite

CSpriteBase::CSpriteBase(void){
	Clear();
}

CSpriteBase::~CSpriteBase(){
	Clear();
}

void	CSpriteBase::Clear(void){
	m_Animation.clear();
}

void	CSpriteBase::SetSprite(SSprite*lpSSprite){
	while (lpSSprite->lpPlane!=NULL) {
		m_Animation.push_back(*lpSSprite);
		lpSSprite++;
	}
}

void	CSpriteBase::SetSpriteAdd(SSprite*lpSSprite){
	m_Animation.push_back(*lpSSprite);
}

SSprite* CSpriteBase::GetSSprite(int nAnimation) {
	WARNING(m_Animation.size() < nAnimation,"CSpriteBase::GetSpriteでnAnimationが範囲外");
	return& m_Animation[nAnimation];
}

int		CSpriteBase::GetAnimationMax(void){
	return m_Animation.size();
}

//////////////////////////////////////////////////////////////////////////////
//	CSprite : CSpriteBaseのローカライズ版

CSprite::CSprite(void) {
	m_lpSpriteBase	= NULL;
	m_nAnimation	= 0;
	m_nOx			= 0;
	m_nOy			= 0;
	m_nLayer		= 8;	//	これがディフォルト（深い意味は無い）
	m_bVisible		= true;
	m_nHeight		= 64;
	m_nDirection	= 0;
}

CSprite::~CSprite(){
}

void	CSprite::SetSprite(CSpriteBase*lpSprite){
	m_lpSpriteBase	= lpSprite;
	m_lpSprite		= lpSprite;
	m_nDirection	= 0;
	m_nAnimation	= 0;
}

void	CSprite::SetOffset(int nOffsetX,int nOffsetY){
	m_nOx = nOffsetX;
	m_nOy = nOffsetY;
}

void	CSprite::GetOffset(int& nOffsetX,int& nOffsetY){
	nOffsetX = m_nOx;
	nOffsetY = m_nOy;
}

void	CSprite::IncMotion(void){
	m_nAnimation++;
	if (m_nAnimation >= m_lpSprite->GetAnimationMax()) {
		m_nAnimation = 0;
	}
}

SSprite* CSprite::GetSSprite(void){
	return m_lpSprite->GetSSprite(m_nAnimation);
}

void	CSprite::SetDirection(int nDirection){
	//	方向の変更が無い場合はアニメカウンタをリセットしない（仕様）
	if (m_nDirection != nDirection) {
		m_nAnimation = 0;
	}
	m_nDirection = nDirection;
	m_lpSprite	 = m_lpSpriteBase + nDirection;	//	これ
}

int		CSprite::GetDirection(void){
	return m_nDirection;
}

//////////////////////////////////////////////////////////////////////////////

