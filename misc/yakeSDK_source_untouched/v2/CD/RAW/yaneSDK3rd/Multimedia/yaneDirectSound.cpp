// DirectSound Wrapper

	//　サウンドドライバーを複数サポートする必要があるのならば、
	//	サウンドがらみの関数は、すべて関数ポインタとして扱うべき
	//	だが、幸いにして、DirectSoundがあるかなしかだけなので、
	//	そこまでしない。

#include "stdafx.h"
#include "yaneDirectSound.h"
#include "../Auxiliary/yaneCOMBase.h"		//	COMの利用のため
#include "../AppFrame/yaneAppManager.h"
//////////////////////////////////////////////////////////////////////////////

// static member
bool CDirectSound::CanUseDirectSound(){
	//	DirectMusicが使える環境なのかどうかを調べて返す
	static bool bFirst = true;
	static bool bUse   = false;

	if (bFirst){	//	最初の１回のみ調べて返す
		bFirst = false;
		CCOMObject<LPDIRECTSOUND> obj;
		bUse = obj.CreateInstance(CLSID_DirectSound, IID_IDirectSound)==0;
	}
	return bUse;
}

//////////////////////////////////////////////////////////////////////////////

CDirectSound::CDirectSound()
{
	m_nStatus = 0;
	m_hWnd = CAppManager::GetHWnd();

	// Create the DirectSound object
	if (GetObject()->CreateInstance(CLSID_DirectSound, IID_IDirectSound)!=0) {
		Err.Out("CDirectSound objectの作成に失敗");
		// SoundCARDが存在しない場合は、このエラー
		m_nStatus = 1;
		return ;
	}

	// この第１パラメータはサウンドカードのGUID
	if (Get()->Initialize(NULL)!=DS_OK){
		Err.Out("CDirectSound::Initializeに失敗");
		// COMオブジェクトを解体する
		GetObject()->Release();
		m_nStatus = 2;
		return ;
	}

	// Set the cooperation level for the DirectSound object
	if (SetCooperativeLevel(DSSCL_NORMAL) != DS_OK) {
		Err.Out("CDirectSoundの初期化エラー。SetCooperativeLevelの失敗");
		// COMオブジェクトを解体する
		GetObject()->Release();
		m_nStatus = 3;
		return ;
	}
}

CDirectSound::~CDirectSound(){
	// 念のため、もとの協調レベルに戻す
	SetCooperativeLevel(DSSCL_NORMAL);
}

//////////////////////////////////////////////////////////////////////////////
LRESULT CDirectSound::SetCooperativeLevel(DWORD dwLevel)
{
	// 初期化に失敗してたらエラー
	if (m_nStatus!=0) {
		return 1;
	}

	if (DS_OK!=Get()->SetCooperativeLevel(m_hWnd, dwLevel)) return 2;
	
	return 0;
}
