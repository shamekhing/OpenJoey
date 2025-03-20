// yaneTimer.h :
//
//	CTimer			:	経過時間をカウントするタイマ
//
//		programmed by yaneurao(M.Isozaki) '99/07/25
//		modified by yaneurao '00/02/28-'00/03/13
//

#ifndef __yaneTime_h__
#define __yaneTime_h__

class CTimeGetTimeWrapper {
/**
	早い話がtimeGetTimeのwrapper

	timeGetTimeが使えない状況に置いては、
	GetTickCountを使うように設定する

	このコンストラクタ～デストラクタで
	timeBeginPeriodMinとtimeEndPeriodMinを
	呼び出している
	
	このオブジェクトはref_createrで用いる。
	(singletonでも良かったのだが..)
*/
public:

	DWORD	GetTime() {
		///	生のタイマの取得
		if (m_bUseTGT) {
			return ::timeGetTime();
		} else {
			return ::GetTickCount();
		}
	}

	CTimeGetTimeWrapper();
	virtual ~CTimeGetTimeWrapper();

	static	ref_creater<CTimeGetTimeWrapper>*	GetRefObj()
		{ return & m_vTimeGetTime; }

protected:
	bool		m_bUseTGT;		//	timeGetTimeを使用するのか？
	int			m_nRef;			//	timeBeginPeriodMin～timeEndPeriodMinの参照カウント
	TIMECAPS	m_dwTimeCaps;	//	タイマー性能

	static	ref_creater<CTimeGetTimeWrapper>	m_vTimeGetTime;
};

//////////////////////////////////////////////////////////////////////////////

class ITimer {
public:
	virtual void	Reset()=0;			///	現在の時刻を０に
	virtual DWORD	Get()=0;			///	現在の時刻の取得
	virtual void	Set(DWORD)=0;		///	現在の時刻の設定
	virtual void	Pause()=0;			///	Pause機能
	virtual void	Restart()=0;		///	Pause解除
	virtual ~ITimer() {}
};

class CTimer : public ITimer {
/**
	早い話が、独立タイマ。
	Reset()すると、タイマがリセットされ、それ以降、Get()を呼び出すと、
	前回、Reset()されたときからの経過時間（[ms]単位）が返ってくるようになる。
*/
public:
	virtual void	Reset();			///	現在の時刻を０に
	virtual DWORD	Get() ;				///	現在の時刻の取得
	virtual void	Set(DWORD);			///	現在の時刻の設定
	virtual void	Pause();			///	Pause機能
	virtual void	Restart();			///	Pause解除

	CTimer();
	virtual ~CTimer();

protected:
	DWORD	m_dwOffsetTime;					//	オフセット値
	DWORD	m_dwPauseTime;					//	PauseかけたときのTime
	int		m_bPaused;						//	pause中か？

private:
	static	ref_creater<CTimeGetTimeWrapper>*	GetRefObj()
	{ return CTimeGetTimeWrapper::GetRefObj(); }
	static	CTimeGetTimeWrapper*	GetObj()
	{ return GetRefObj()->get(); }
};

//////////////////////////////////////////////////////////////////////////////

class CFixTimer : public ITimer {
/**
	ゲームで class CTimer を使う場合、
	１フレームの間は、固定値が返ってきたほうが望ましい。

	class CMouse に対する class CMouseEx と同じ関係である。
	Flush関数の説明を読むこと。
*/
public:
	virtual void	Reset();			///	現在の時刻を０に
	virtual DWORD	Get();				///	現在の時刻の取得
	virtual void	Set(DWORD);			///	現在の時刻の設定
	virtual void	Pause();			///	Pause機能
	virtual void	Restart();			///	Pause解除

	virtual	void	Flush();
	/**
		時刻を更新する。
		これをした瞬間の時刻値に基づいてGetで値が返るようになる。
		以降、再度この関数を呼び出すまで、Flushと同じ値が返る
		この関数以外は、class CTimer と同じ
	*/

	CFixTimer();

protected:
	CTimer	m_vTimer;
	DWORD	m_dwTimeGetTime;				//	前回Flushした時間

	CTimer* GetTimer() { return& m_vTimer; }
};

#endif
