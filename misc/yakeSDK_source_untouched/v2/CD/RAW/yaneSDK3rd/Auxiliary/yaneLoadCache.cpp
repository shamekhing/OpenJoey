
#include "stdafx.h"
#include "yaneLoadCache.h"
#include "yaneFile.h"
#include "yaneStringScanner.h"

//////////////////////////////////////////////////////////////////////////////

CLoadCache::CLoadCache() :
	m_vLoadCacheListener(new ILoadCacheListener),
	m_bCanReloadAgain(true),
	m_nCacheMax(INT_MAX)
{}

CLoadCache::~CLoadCache(){}

//////////////////////////////////////////////////////////////////////////////

LRESULT		CLoadCache::Set(const string& filename){
	//	読み込み済み＆２度読み禁止
	if (!m_bCanReloadAgain) return 0;

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
	GetLoadCache()->clear();
	InnerDelete();

	if (nNums==0) return 0;

	GetLoadCache()->resize(nNums);

	nNums = 0;	//	再度リセット
	file.Reset();

	while (true){
		CHAR buf[256];
		LRESULT lr = file.ReadLine(buf);
		if (lr==1) break ; // EOF
		if (buf[0]=='\0') continue;		//	空行の読み飛ばし
		if (buf[0]=='/' && buf[1]=='/') continue;	//	コメント行の読み飛ばし
		LPSTR lp = buf;

		CLoadCacheInfo* pInfo = new CLoadCacheInfo;
		pInfo->strFilename = CStringScanner::GetStrFileName(lp);
		(*GetLoadCache())[nNums].Add(pInfo);
		nNums ++;
	}
	InnerCreate(nNums);
	return 0;
}

LRESULT		CLoadCache::Load(int nNo){

	if (nNo<0 || GetSize()<=nNo) return -1;
	//	範囲外

	CLoadCacheInfo &info = *GetLoadCache(nNo);

	InnerUse(nNo);	//	こいつを参照しました！

	if (info.bLoaded) {
		//	既に読み込んどるでー
		return 0;
	}

	//	読み込みに失敗しようがフラグは立てておかないと、
	//	あとで解放しようとしたときに不正なアクセスになる
	info.bLoaded = true;

	//	読み込んだことによって、キャッシュオーバーした分を解放
	InnerPushOut();

	return InnerLoad(nNo);
}

LRESULT		CLoadCache::Release(int nNo){
	if (nNo<0 || GetSize()<=nNo) return -1;
	//	範囲外

	CLoadCacheInfo &info = *GetLoadCache(nNo);
	
	if (!info.bLoaded) return -1;	//	読み込んでない
	info.bLoaded	= false;
	info.nStaleTime = -1;	//	リセット
	return InnerRelease(nNo);
}

LRESULT		CLoadCache::ReleaseAll(){
	int nSize = GetSize();
	LRESULT lr =0;
	for (int i=0;i<nSize;i++){
		lr |= Release(i);
	}
	return lr;
}

void	CLoadCache::InnerUse(int nNo){
/**
	-1でないときのインクリメント
前		1 2 3 -1 4 -1 5 0
後	⇒	2 3 0 -1 4 -1 5 1
			↑を使用したので、こいつを0に

  -1からのインクリメント
前		1 2 3 -1 4 -1 5 0
後	⇒	2 3 4  0 4 -1 6 1
			  ↑を使用したので、こいつを0に
*/

	int nSize = GetSize();
	CLoadCacheInfo &info = *GetLoadCache(nNo);
	int nStaleTime = info.nStaleTime;
	if (nStaleTime == -1) nStaleTime = INT_MAX;
	for (int i=0;i<nSize;i++){
		//	使用カウンタのインクリメント
		CLoadCacheInfo &info2 = *GetLoadCache(nNo);
		if (info2.nStaleTime < nStaleTime && info2.nStaleTime!=-1){
			//	これより低いやつのみ
			info2.nStaleTime++;
		}
	}
	info.nStaleTime = 0;
}

void	CLoadCache::SetCacheMax(int nCacheMax){
	m_nCacheMax = nCacheMax;
	InnerPushOut();
}

void	CLoadCache::InnerPushOut(){
	int nSize = GetSize();
	for (int i=0;i<nSize;i++){
		CLoadCacheInfo &info = *GetLoadCache(i);
		if (info.nStaleTime >= m_nCacheMax) {
		//	これよりカウンタの進んだ奴は解放
			Release(i);
		}
	}
}
