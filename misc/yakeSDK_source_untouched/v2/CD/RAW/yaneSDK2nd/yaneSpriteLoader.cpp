
#include "stdafx.h"
#include "yaneSpriteLoader.h"
#include "yaneSpriteChara.h"

//////////////////////////////////////////////////////////////////////////////

CSpriteLoader::CSpriteLoader(void){
}

CSpriteLoader::~CSpriteLoader(){
	InnerDelete();	//	これは、こちらのデストラクタで行なう必要あり
}

void		CSpriteLoader::InnerCreate(void){
	m_lpChara.resize(m_nMax);
}

void		CSpriteLoader::InnerDelete(void){
	m_lpChara.clear();
}

//////////////////////////////////////////////////////////////////////////////

CSpriteBase*	CSpriteLoader::GetSprite(int nNo){
	WARNING(nNo >= m_nMax,"CSpriteLoader::GetSpriteで範囲外");

	LRESULT hr;
	hr = Load(nNo);			//	読み込んでいなければここで読み込む必要あり
	if (hr) {
		WARNING(true,"CSpriteLoader::GetSpriteでファイル読み込みに失敗");
		return NULL;	//	だめだこりゃ＾＾
	}

	return m_lpChara[nNo].GetSprite();
}

//////////////////////////////////////////////////////////////////////////////

LRESULT		CSpriteLoader::InnerLoad(int nNo){
	return m_lpChara[nNo].Load(m_lpSLOAD_CACHE[nNo].lpszFilename);
}

LRESULT		CSpriteLoader::InnerRelease(int nNo){
	m_lpChara[nNo].Release();
	return 0;
}

//////////////////////////////////////////////////////////////////////////////
