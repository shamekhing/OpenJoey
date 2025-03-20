//
//	yaneSELoader.h :
//
//		ＳＥの統括的ローディング
//

#ifndef __yaneSELoader_h__
#define __yaneSELoader_h__

#include "../Multimedia/yaneSoundLoader.h"
#include "../Math/yaneCounter.h"

class CSELoader : public CSoundLoader {
public:
	enum { nSELoaderCount = 5 };
	//	ディフォルトでは５フレーム以内ならば再生再開しない

	//	フレームが経過した場合
	void		OnPlay();				///	毎フレーム呼び出してね
	void		PlayN(int nNo);			///	いますぐ再生する(非ループ再生)
	void		Play(int nNo);			///	鳴っていなければ再生する(非ループ再生)
	void		PlayLN(int nNo);		///	いますぐ再生する(ループ再生)
	void		PlayL(int nNo);			///	鳴っていなければ再生する(ループ再生)
	void		PlayT(int nNo,int nTimes,int nInterval=0);
										///	再生回数指定再生
	void		Stop(int nNo);			///	サウンドを止める
	void		Reset();				///	サウンドの排他カウンタのリセット
	bool		IsPlay(int nNo);		///	サウンドは再生中なのか？

	///	virtual smart_ptr<ISound>	GetSound(int nNo);	///	サウンドの取得
	///	↑これは class CSoundLoader のメンバを使えば良い

	///	再生キャンセルフラグ
	///	(このフラグがtrueならば、すべての再生はキャンセルされる)
	bool*		GetCancelFlag()		{ return &m_bCancel; }

	void		OnPlayAndReset();
	///	現在、再生依頼のあるものを強制的に再生する

	///	再生インターバルの設定(ディフォルトではnSELoaderCount)
	void	SetLockInterval(int nLockInterval) { m_nLockInterval = nLockInterval; }
	int		GetLockInterval() { return m_nLockInterval; }

	CSELoader();
	virtual ~CSELoader();

protected:
	vector<int>		m_anPlay;
	//	プラスのときは、guard timeを意味する
	//		nSELoaderCountで再生されて、0になるまでデクリメントされる
	//	これマイナスならば再生回数なのら＾＾；

	vector<CRootCounter>	m_anInterval;
	//	連続再生のときの再生インターバル


	//	ひとつの要素に読み込み／解放
//	virtual	LRESULT InnerLoad(int nNo);
//	virtual LRESULT InnerRelease(int nNo);
	//	m_nMax分だけ配列を確保／解放
	virtual void	InnerCreate(int nMax);
	virtual void	InnerDelete();

	bool	m_bCancel;			//	BGMキャンセルフラグ
	int		m_nLockInterval;	//	このフレーム以内ならば再生再開しない
};

#endif
