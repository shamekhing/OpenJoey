
#include "stdafx.h"
#include "yaneSELoader.h"
#include "yaneSound.h"

//////////////////////////////////////////////////////////////////////////////

CSELoader::CSELoader(void){
	m_bCancel			= false;
	m_nLockInterval		= CSELoaderCount;

	// デフォルトのPCMReaderFactoryを生成
	m_lpReaderFactory.Add();
}

CSELoader::~CSELoader(){
	InnerDelete();	//	これは、こちらのデストラクタで行なう必要あり
}

void		CSELoader::InnerCreate(void){
	m_lpSound.resize(m_nMax);
	m_anPlay.resize(m_nMax);
	m_anInterval.resize(m_nMax);
	Reset();
}

void		CSELoader::InnerDelete(void){
	ReleaseAll();	//	こいつで全解放する
	m_lpSound.clear();
	m_anPlay.clear();
	m_anInterval.clear();
}

//////////////////////////////////////////////////////////////////////////////

CSound*	CSELoader::GetSound(int nNo){

	WARNING(nNo >= m_nMax,"CSELoader::GetSoundで範囲外");

	LRESULT hr;
	hr = Load(nNo);			//	読み込んでいなければここで読み込む必要あり
	if (hr) {
//		WARNING(true,"CSpriteSound::GetSoundでファイル読み込みに失敗");
//		return NULL;	//	だめだこりゃ＾＾
	//	失敗しても正規のポインタを返しておかないと不正な呼び出しになる...
	}

	m_lpnStaleTime[nNo] = 0;	//	使用したでフラグをＯｎ＾＾；
	return m_lpSound[nNo];
}

//////////////////////////////////////////////////////////////////////////////

LRESULT		CSELoader::InnerLoad(int nNo){
	WARNING(m_lpSound[nNo]!=NULL,"CSELoader::InnerLoadでm_lpSound[nNo]!=NULL");
	m_lpSound[nNo].Add();		//	CSound作成して読み込み

	// Factoryをセットする。
	m_lpSound[nNo]->SetReaderFactory(m_lpReaderFactory);

	return m_lpSound[nNo]->Load(m_lpSLOAD_CACHE[nNo].lpszFilename);
}

LRESULT		CSELoader::InnerRelease(int nNo){
	m_lpSound[nNo].Delete();	//	解放しちまう
	return 0;
}

//////////////////////////////////////////////////////////////////////////////

//	フレームが経過した場合
void		CSELoader::OnPlay(void){
	if (m_bCancel) return ;
	for(int i=0;i<m_nMax;i++){
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

void		CSELoader::OnPlayAndReset(void){
	if (m_bCancel) return ;
	for(int i=0;i<m_nMax;i++){
		if (m_anPlay[i] == m_nLockInterval){
			GetSound(i)->Play();
		}
		m_anPlay[i] = 0;
	}
}

void		CSELoader::PlayN(int nNo){
	if (m_bCancel) return ;
	//	いますぐ再生する
	WARNING(nNo>=m_nMax,"CSELoader::PlayNでnNo>=nMax");
	if (m_anPlay[nNo] == 0) {
		m_anPlay[nNo] = m_nLockInterval;
	}
	GetSound(nNo)->SetLoopMode(false);
}

void		CSELoader::Play(int nNo){
	if (m_bCancel) return ;
	//	鳴っていなければ再生する
	WARNING(nNo>=m_nMax,"CSELoader::PlayでnNo>=nMax");
	if (m_anPlay[nNo] == 0) {
		if (!GetSound(nNo)->IsPlay()) {
			m_anPlay[nNo] = m_nLockInterval;
		}
	}
	GetSound(nNo)->SetLoopMode(false);
}

void		CSELoader::PlayLN(int nNo){
	if (m_bCancel) return ;
	//	いますぐ再生する
	WARNING(nNo>=m_nMax,"CSELoader::PlayLNでnNo>=nMax");
	if (m_anPlay[nNo] == 0) {
		m_anPlay[nNo] = m_nLockInterval;
	}
	GetSound(nNo)->SetLoopMode(true);
}

void		CSELoader::PlayL(int nNo){
	if (m_bCancel) return ;
	//	鳴っていなければ再生する
	WARNING(nNo>=m_nMax,"CSELoader::PlayLでnNo>=nMax");
	if (m_anPlay[nNo] == 0) {
		if (!GetSound(nNo)->IsPlay()) {
			m_anPlay[nNo] = m_nLockInterval;
		}
	}
	GetSound(nNo)->SetLoopMode(true);
}

void		CSELoader::PlayT(int nNo,int nTimes,int nInterval){
	if (m_bCancel) return ;
	//	再生回数指定再生
	//	鳴っていなければ再生する
	WARNING(nNo>=m_nMax,"CSELoader::PlayLでnNo>=nMax");
	m_anPlay[nNo] = -nTimes;			//	これマイナスならば再生回数なのら＾＾；
	m_anInterval[nNo].Set(0,nInterval);	//	このときだけインターバルを設定するのだ
	m_anInterval[nNo] = nInterval;		//  １回目は速攻なるべし！
	GetSound(nNo)->SetLoopMode(false);
}

void		CSELoader::Stop(int nNo){
	GetSound(nNo)->Stop();
}

void		CSELoader::Reset(int nLevel){
	for(int i=0;i<m_nMax;i++){
		//	自分よりレベルの低いところに対してのみ有効
		if (m_lpnReleaseLevel[i]<=nLevel){
			m_anPlay[i] = 0;
		}
	}
}

void	CSELoader::IncStaleTime(void){
	for(int i=0;i<m_nMax;++i){
		if (!IsPlay(i)) m_lpnStaleTime[i]++;
	}
}

bool	CSELoader::IsPlay(int nNo){
	//	サウンドは再生中なのか？
	WARNING(nNo >= m_nMax,"CSELoader::IsPlayで範囲外");

	if (!m_lpbLoad[nNo]) {	return false; }
	return m_lpSound[nNo]->IsPlay();
}
