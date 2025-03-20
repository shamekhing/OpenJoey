#include "stdafx.h"
#include "yaneSoundFactory.h"
#include "yaneSound.h"
//	CSoundFactoryが必要とする
#include "../Multimedia/yaneSoundFactory.h"
#include "../Multimedia/yaneDirectMusic.h"
#include "../Multimedia/yaneDirectSound.h"
#include "../Multimedia/yaneWaveOutput.h"
#include "yaneSoundLoader.h"

//////////////////////////////////////////////////////////////////////////////

void		CSoundLoader::InnerCreate(int nMax){
	m_apSound.resize(nMax);
}

void		CSoundLoader::InnerDelete(){
	ReleaseAll();	//	こいつで全解放する
	m_apSound.clear();
}

//////////////////////////////////////////////////////////////////////////////

smart_ptr<ISound>	CSoundLoader::GetSound(int nNo){

	LRESULT hr;
	hr = Load(nNo);		//	読み込んでいなければここで読み込む必要あり
	if (hr) {
		return smart_ptr<ISound>();	//	だめだこりゃ＾＾
	}
	return m_apSound[nNo];
}

//////////////////////////////////////////////////////////////////////////////

LRESULT		CSoundLoader::InnerLoad(int nNo){
	if (nNo<0 || nNo>=GetSize()) return -1;	//	範囲外
	if (m_sf.isNull()) return -2;	//	factoryが設定されていない
	string strFileName(GetLoadCache(nNo)->strFilename);
	m_apSound[nNo] = m_sf->Create(strFileName);
	return m_apSound[nNo]->Open(strFileName);
}

LRESULT		CSoundLoader::InnerRelease(int nNo){
	if (nNo<0 || nNo>=GetSize()) return -1;	//	範囲外
	m_apSound[nNo].Delete();	//	解放しちまう
	return 0;
}

//////////////////////////////////////////////////////////////////////////////
