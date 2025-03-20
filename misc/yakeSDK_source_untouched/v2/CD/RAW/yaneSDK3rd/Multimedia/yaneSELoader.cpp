
#include "stdafx.h"
#include "yaneSELoader.h"
#include "yaneSound.h"

//////////////////////////////////////////////////////////////////////////////

CSELoader::CSELoader(){
	m_bCancel			= false;
	m_nLockInterval		= nSELoaderCount;
}

CSELoader::~CSELoader(){
	InnerDelete();	//	これは、こちらのデストラクタで行なう必要あり
}

void		CSELoader::InnerCreate(int nMax){
	m_anPlay.resize(nMax);
	m_anInterval.resize(nMax);
	Reset();
	CSoundLoader::InnerCreate(nMax);	//	superクラスを呼び出す
}

void		CSELoader::InnerDelete(){
	m_anPlay.clear();
	m_anInterval.clear();
	CSoundLoader::InnerDelete();	//	superクラスを呼び出す
}

//////////////////////////////////////////////////////////////////////////////

//	フレームが経過した場合
void		CSELoader::OnPlay(){
	if (m_bCancel) return ;
	for(int i=0;i<GetSize();i++){
		int& n = m_anPlay[i];
		if (n == m_nLockInterval){
			GetSound(i)->Play();
		}
		if (n > 0) {
			n--;
		} ef (n < 0) {
			//	回数指定再生モード
			if (!GetSound(i)->IsPlay()) {
				//	m_anInterval[i]分だけ経過したか？
				m_anInterval[i]++;
				if (m_anInterval[i].IsEnd()){
					GetSound(i)->Play();
					n++;
					m_anInterval[i].Reset();
				}
			}
		}
	}
}

void		CSELoader::OnPlayAndReset(){
	if (m_bCancel) return ;
	for(int i=0;i<GetSize();i++){
		if (m_anPlay[i] == m_nLockInterval){
			GetSound(i)->Play();
		}
		m_anPlay[i] = 0;
	}
}

void		CSELoader::PlayN(int nNo){
	if (m_bCancel) return ;
	//	いますぐ再生する
	if (m_anPlay[nNo] == 0) {
		m_anPlay[nNo] = m_nLockInterval;
	}
	GetSound(nNo)->SetLoopPlay(false);
}

void		CSELoader::Play(int nNo){
	if (m_bCancel) return ;
	//	鳴っていなければ再生する
	if (m_anPlay[nNo] == 0) {
		if (!GetSound(nNo)->IsPlay()) {
			m_anPlay[nNo] = m_nLockInterval;
		}
	}
	GetSound(nNo)->SetLoopPlay(false);
}

void		CSELoader::PlayLN(int nNo){
	if (m_bCancel) return ;
	//	いますぐ再生する
	if (m_anPlay[nNo] == 0) {
		m_anPlay[nNo] = m_nLockInterval;
	}
	GetSound(nNo)->SetLoopPlay(true);
}

void		CSELoader::PlayL(int nNo){
	if (m_bCancel) return ;
	//	鳴っていなければ再生する
	if (m_anPlay[nNo] == 0) {
		if (!GetSound(nNo)->IsPlay()) {
			m_anPlay[nNo] = m_nLockInterval;
		}
	}
	GetSound(nNo)->SetLoopPlay(true);
}

void		CSELoader::PlayT(int nNo,int nTimes,int nInterval){
	if (m_bCancel) return ;
	//	再生回数指定再生
	//	鳴っていなければ再生する
	m_anPlay[nNo] = -nTimes;			//	これマイナスならば再生回数なのら＾＾；
	m_anInterval[nNo].Set(0,nInterval);	//	このときだけインターバルを設定するのだ
	m_anInterval[nNo] = nInterval;		//	１回目は速攻なるべし！
	GetSound(nNo)->SetLoopPlay(false);
}

void		CSELoader::Stop(int nNo){
	GetSound(nNo)->Stop();
}

void		CSELoader::Reset(){
	for(int i=0;i<GetSize();i++){
		//	自分よりレベルの低いところに対してのみ有効
		m_anPlay[i] = 0;
	}
}

bool	CSELoader::IsPlay(int nNo){
	//	サウンドは再生中なのか？
	return GetSound(nNo)->IsPlay();
}
