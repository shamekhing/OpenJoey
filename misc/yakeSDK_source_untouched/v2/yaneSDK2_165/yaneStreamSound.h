#ifdef USE_StreamSound

//
//	CStreamSound :
//		derived from CSoundBase
//		enable Streaming Sound Play
//
//	This is adaptor for LongStream
//		programmed by まつけん(matuken1@geocities.co.jp)
//

#ifndef __yaneStreamSound_h__
#define __yaneStreamSound_h__

#include "mtknlongsound.h"
#include "yaneSoundBase.h"
#include "yaneDirectSound.h"
#include "yaneFile.h"
#include "yaneTimer.h"

class CStreamSound : public CSoundBase {
public:
	//	CSoundBaseメンバのオーバーライド
	virtual LRESULT Open(LPCTSTR pFileName){
		Stop();
		//m_szFileName=CFile::MakeFullName(pFileName);
		bool ret = m_sound->open(pFileName, IsLoopPlay());
		m_szFileName = pFileName;
		//--- 追加 '02/01/05  by enra ---
		SetCurrentPos(0);
		//-------------------------------
		return (ret)?0:1;
	}
	//--- 修正 '02/01/05  by enra ---
	virtual LRESULT Play(void){
		// 開いてないでしょ
		if(m_szFileName==""||m_szFileName.empty()) return 2;
		// 今Pause中でしょ
		if(IsPause()) return Replay();
		if(IsPlay()) Stop();

		if(GetCurrentPos()==GetLength()){
			m_sound->SetOffset(0);
		}
		m_bPlaying = m_sound->play();
		m_sound->SetVolume(m_Volume);
		m_bPausing = false;
		m_PausePos = 0;
		return (m_bPlaying)?0:1;
	}
	virtual LRESULT Replay(void)
	{
		// 再生中？ポーズ中じゃない？じゃ駄目ですな
		if(IsPlay()||!IsPause()) return 99;

		m_sound->stop();	// 念のため
		// ポーズ位置をセットしてと
		m_sound->SetOffset(m_PausePos);
		m_bPlaying = m_sound->play();
		m_sound->SetVolume(m_Volume);
		m_bPausing = false;
		m_PausePos = 0;
		return (m_bPlaying)?0:1;
	}
	virtual LRESULT Close(void)
	{
		Stop();
		m_szFileName = "";
		return m_sound->close();
	}
	virtual LRESULT Pause(void)
	{
		// ポーズ中？再生中じゃない？じゃ駄目ですな
		if(!IsPlay()||IsPause()) return 99;

		// 位置を最初に得る
		m_PausePos = m_sound->GetCurrentPos();
		bool ret = m_sound->stop();
		// つじつまを合わせる^^;
		m_sound->SetOffset(m_PausePos);
		m_bPlaying = false;
		m_bPausing = true;
		return (ret)?0:1;
	}
	virtual bool IsPause(void)	 { return m_bPausing; }
	//-------------------------------

	//--- 修正 '02/01/05  by enra ---
	virtual LRESULT Stop(void)
	{
		// すぐに止めるとノイズが出るのでボリュームを細工する
		const LONG now = GetVolume();
		const LONG step = (now/*<=0*/-DSBVOLUME_MIN/*=-10000*/)/25;
		CTimer vTimer;
		vTimer.Reset();
		for(int i=0; i<25; i=vTimer.Get()){
			// ちょっと誤差出るけど気にしないヽ(´▽｀)ノ
			m_sound->SetVolume(now - step*i);
		}
		// 最後の仕上げ
		m_sound->SetVolume(DSBVOLUME_MIN);
		// んで止める
		LRESULT ret = m_sound->stop();
		// ボリュームを元に戻す
		m_sound->SetVolume(now);
		m_bPlaying = false;
		m_bPausing = false;
		m_sound->SetOffset(0);	// 自分のを使うと無限ループ^^;
//		m_sound->SetOffset(GetLength());
		return ret;
	}
	//-------------------------------

	virtual bool IsPlay(void)
	{
		if(IsLoopPlay()) return m_bPlaying;
		if(m_bPlaying==false) return false;

		const LONG len = GetLength();
		const int diff = len - m_sound->GetCurrentPos();	// 自分のを使うと無限ループ^^;
		// ループ再生でない場合、m_soundはこの辺で停止するようになっている
		if(-100<=diff&&diff<=100){
			// つじつまを合わせる^^;
			Stop();
			m_sound->SetOffset(len);
		};
		return m_bPlaying;
	}
	virtual bool IsLoopPlay(void){ return m_bLoop; }// 追加
	virtual LRESULT SetLoopMode(bool bLoop) { m_bLoop=bLoop; m_sound->SetLoop(bLoop); return 0; }
	//--- 修正 '02/01/05  by enra ---
	virtual LONG GetCurrentPos(void)
	{
		if(IsPlay()) return m_sound->GetCurrentPos();
		ef(IsPause()) return m_PausePos;
		return m_sound->GetOffset();
	}
	virtual LRESULT SetCurrentPos(LONG lPos)
	{
		// クリッピング
		if(lPos>GetLength()){ lPos = GetLength(); }
		if(lPos<0){ lPos = 0; }

		// Pause中なら、m_PausePosをいじくってやる
		if(IsPause()) { m_PausePos = lPos;  return 0; }

		// 一度止めてオフセット指定してPlay()してやらないと、
		// 再生位置が正しく得られない。
		bool bPlaying = IsPlay();
		Stop();
		m_sound->SetOffset(lPos);
		LRESULT ret = (bPlaying) ? Play() : 0;
		return ret;
		// LRESULT ret = m_sound->SetCurrentPos(lPos);
	}
	//-------------------------------

	virtual LONG	GetLength(void){ return m_sound->GetLength();}
	virtual LRESULT SetVolume(LONG volume){ m_Volume = volume; return m_sound->SetVolume(m_Volume);} // このチャンネルに対するヴォリューム
	virtual LONG	GetVolume(void){ return m_sound->GetVolume();}	// 取得は、特定チャンネルに対してしかできない...

	//	CSoundに似せる＾＾；
	LRESULT Load(string filename) { return Open(CFile::MakeFullName(filename).c_str()); }
	LRESULT Release(void)		{ return Close(); }

	//--- 追加 '01/11/19  by enra ---
	virtual void SetReaderFactory(smart_ptr<CPCMReaderFactory> p)
	{
		m_lpReaderFactory = p;
		m_sound->SetFactory(m_lpReaderFactory);
	}
	virtual smart_ptr<CPCMReaderFactory> GetReaderFactory(void)
	{
		return m_lpReaderFactory;
	}
	//-------------------------------

	CStreamSound(void) {
		m_sound.Add(mtknLib::IkmLongSound::create());
		m_bLoop = false;
		m_lpReaderFactory.Add();
		m_sound->SetFactory(m_lpReaderFactory);
		//--- 追加 '02/01/05  by ENRA ---
		m_bPlaying = false;
		m_bPausing = false;
		m_PausePos = 0;
		//-------------------------------
		//--- 追加 '02/02/09  by ENRA ---
		m_Volume = 0;
		//-------------------------------
	}
	virtual ~CStreamSound() {
		Close();
	}

protected:
	auto_ptrEx<mtknLib::IkmLongSound>	m_sound;
	smart_ptr<CPCMReaderFactory> m_lpReaderFactory;
	string	m_szFileName;
	bool	m_bLoop;
	LONG	m_lLength;
	//--- 追加 '02/01/05  by ENRA ---
	bool	m_bPlaying;
	bool	m_bPausing;
	LONG	m_PausePos;
	//-------------------------------
	//--- 追加 '02/02/09  by ENRA ---
	LONG	m_Volume;
	//-------------------------------
	//--- 追加 '02/02/28  by enra ---
	smart_ptr<CTimer>	m_vTimer;
	CTimer*	GetTimer(){ return m_vTimer; }
	//-------------------------------
};

#endif

#endif // USE_StreamSound
