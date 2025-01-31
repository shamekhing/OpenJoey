
#include "stdafx.h"
#include "yaneLoadCache.h"
#include "yaneFile.h"
#include "yaneStringScanner.h"

//////////////////////////////////////////////////////////////////////////////

CLoadCache::CLoadCache(void){
	m_nMax		= 0;
	m_bCanReloadAgain = true;
	m_vLoadCacheListener.Add(new CLoadCacheListener);
}

CLoadCache::~CLoadCache(){
	InnerRelease();
}

void		CLoadCache::InnerRelease(void){
	m_lpbLoad.clear();
	m_lpnReleaseLevel.clear();
	m_lpnStaleTime.clear();
}

//////////////////////////////////////////////////////////////////////////////

void		CLoadCache::SetMax(int nMax){
	InnerDelete();	//　これは、InnerReleaseに入れてしまうとデストラクタから
					//	呼び出されてしまってまずい
	//	↑このなかでm_lpnReleaseLevelと比較するコードがありうるので
	//	↓は後でないとマズイ。
	InnerRelease();

	m_nMax = nMax;
	if (nMax == 0) return ;

	m_lpbLoad.resize(nMax);
	m_lpnReleaseLevel.resize(nMax);
	m_lpnStaleTime.resize(nMax);

	InnerCreate();

	for(int i=0;i<nMax;i++){
		m_lpbLoad[i]		= false;
	}
}

LRESULT		CLoadCache::Set(SLOAD_CACHE*lp){
	//	読み込み済み＆２度読み禁止
	if (!m_bCanReloadAgain && m_nMax!=0) return 0;
		//	no errorのふりをして帰る

	m_lpSLOAD_CACHE = lp;

	//	定義ファイル名の数をカウント
	int n=0;
	while((lp->lpszFilename)!=NULL && *(lp->lpszFilename)!='\0'){
		//	if pointer is NULL or *pointer is '\0'
		lp++; n++;
	}

	SetMax(n);

	for(int i=0;i<n;i++){
		m_lpnReleaseLevel[i]	= m_lpSLOAD_CACHE->nReleaseLevel;
		m_lpnStaleTime[i]		= 0;	//	リセット
	}
	return 0;
}

LRESULT		CLoadCache::Set(string filename){
	//	読み込み済み＆２度読み禁止
	if (!m_bCanReloadAgain && m_nMax!=0) return 0;

	//	ファイルから設定ファイルを読み込むときのために．．．
	CFile file;
	if (file.Read(filename)!=0) return 1;
	int nNums = 0;
	//	↑設定されるファイル数
	while (true){
		CHAR buf[256];
		LRESULT lr = file.ReadLine(buf);
		if (lr==1) break ; // EOF
		if (lr!=0) return 1; // read error..
		if (buf[0]=='\0') continue;		//	空行の読み飛ばし
		if (buf[0]=='/' && buf[1]=='/') continue;	//	コメント行の読み飛ばし
		nNums ++;
	}

	//	最大数は確定したので、本読みする
	m_aLoadCache.resize(nNums+1);
	m_aFileName.resize(nNums);

	nNums = 0;	//	再度リセット
	file.Reset();

	while (true){
		CHAR buf[256];
		LRESULT lr = file.ReadLine(buf);
		if (lr==1) break ; // EOF
		if (buf[0]=='\0') continue;		//	空行の読み飛ばし
		if (buf[0]=='/' && buf[1]=='/') continue;	//	コメント行の読み飛ばし
		LPSTR lp = buf;
		m_aFileName[nNums] = CStringScanner::GetStr(lp);
		int n;
		if (CStringScanner::GetNum(lp,n)==0){
			m_aLoadCache[nNums].nReleaseLevel = n;
		} else {
			m_aLoadCache[nNums].nReleaseLevel = 0;
		}
		//	めんどくさいから、これでええや．．
		m_aLoadCache[nNums].lpszFilename = const_cast<LPSTR>(m_aFileName[nNums].c_str());
		nNums ++;
	}
	m_aLoadCache[nNums].lpszFilename = NULL;	//	デリミタ
	Set(m_aLoadCache);
	return 0;
}

LRESULT		CLoadCache::Load(int nNo,int nReleaseLevel){

	WARNING(nNo >= m_nMax,"CLoadCache::Loadで範囲外");

	if (nReleaseLevel!=-1) {
		m_lpnReleaseLevel[nNo] = nReleaseLevel; // 一応代入しておこう．．
		m_lpnStaleTime[nNo]	   = 0;
	}

	if (m_lpbLoad[nNo]) {
		//	既に読み込んどるでー
		return 0;
	}

	//	読み込みに失敗しようがフラグは立てておかないと、
	//	あとで解放しようとしたときに不正なアクセスになる
	m_lpbLoad[nNo] = true;
	return InnerLoad(nNo);
}

void		CLoadCache::Release(int nNo){
	WARNING(nNo >= m_nMax,"CLoadCache::Releaseで範囲外");

	if (!m_lpbLoad[nNo]) return ;
	m_lpbLoad[nNo] 		= false;
	m_lpnStaleTime[nNo]	= 0;	//	リセット
	InnerRelease(nNo);
}

void		CLoadCache::ReleaseAll(int nReleaseLevel){
	ReleaseAll(0,m_nMax,nReleaseLevel);
}

void		CLoadCache::ReleaseAll(int nStart,int nEnd,int nReleaseLevel){
	//	WARNING(nEnd >= m_nMax,"CLoadCache::ReleaseAllで範囲外");
	//	↑nStart==nEndで、Relaseを実行しないならばＯｋなので、これは不必要。
	for(int i=nStart;i<nEnd;i++){
		if (m_lpnReleaseLevel[i] <= nReleaseLevel)
			Release(i);
	}
}

void		CLoadCache::ReleaseStaleAll(int nReleaseTime){
	ReleaseStaleAll(0,m_nMax,nReleaseTime);
}

void		CLoadCache::ReleaseStaleAll(int nStart,int nEnd,int nReleaseTime){
	for(int i=nStart;i<nEnd;i++){
		if (m_lpnStaleTime[i] >= nReleaseTime)
			Release(i);
	}
}

void		CLoadCache::IncStaleTime(void){
	//	時間を経過させる
	for(int i=0;i<m_nMax;++i){
		m_lpnStaleTime[i]++;
	}
}
