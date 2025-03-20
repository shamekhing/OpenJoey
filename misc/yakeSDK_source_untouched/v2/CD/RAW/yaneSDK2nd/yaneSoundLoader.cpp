#include "stdafx.h"
#include "yaneSoundLoader.h"
#include "yaneSound.h"

//////////////////////////////////////////////////////////////////////////////

CSoundLoader::CSoundLoader(void){
}

CSoundLoader::~CSoundLoader(){
	InnerDelete();	//	これは、こちらのデストラクタで行なう必要あり
}

void		CSoundLoader::InnerCreate(void){
	m_lpSound.resize(m_nMax);
}

void		CSoundLoader::InnerDelete(void){
	ReleaseAll();	//	こいつで全解放する
	m_lpSound.clear();
}

//////////////////////////////////////////////////////////////////////////////

CSound*	CSoundLoader::GetSound(int nNo){
	WARNING(nNo >= m_nMax,"CSoundLoader::GetSoundで範囲外");

	LRESULT hr;
	hr = Load(nNo);			//	読み込んでいなければここで読み込む必要あり
	if (hr) {
		WARNING(true,"CSoundLoader::GetSoundでファイル読み込みに失敗");
		return NULL;	//	だめだこりゃ＾＾
	}

	m_lpnStaleTime[nNo] = 0;	//	使用したでフラグをＯｎ＾＾；
	return m_lpSound[nNo];
}

//////////////////////////////////////////////////////////////////////////////

LRESULT		CSoundLoader::InnerLoad(int nNo){
	WARNING(m_lpSound[nNo]!=NULL,"CSoundLoader::InnerLoadでm_lpSound[nNo]!=NULL");
	m_lpSound[nNo].Add();		//	CSound作成して読み込み
	return m_lpSound[nNo]->Load(m_lpSLOAD_CACHE[nNo].lpszFilename);
}

LRESULT		CSoundLoader::InnerRelease(int nNo){
	m_lpSound[nNo].Delete();	//	解放しちまう
	return 0;
}

//////////////////////////////////////////////////////////////////////////////
