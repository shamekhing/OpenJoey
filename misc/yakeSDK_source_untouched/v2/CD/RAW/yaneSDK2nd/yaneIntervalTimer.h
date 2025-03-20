// yaneTimer.h :
//
//	CIntervalTimer	:	マルチメディアタイマ
//						と通常のインターバルタイマーの統合クラス
//
//		programmed by yaneurao(M.Isozaki) '99/07/25
//		modified by yaneurao '00/02/28-'00/03/13
//

#ifndef __yaneIntervalTime_h__
#define __yaneIntervalTime_h__

// マルチメディアタイマー
class CIntervalTimer {
public:

	CIntervalTimer(void);
	virtual ~CIntervalTimer();

	//	タイマを開始する
	LRESULT Start(DWORD dwInterval,bool bMultiMediaTimer = true);

	//	こちらは、一発タイマを何度も呼び出す
	LRESULT	StartM(DWORD dwInterval);

	LRESULT Stop(void);					//	タイマを止める
	bool	IsTimer(void) const;		//	タイマが回っているのか
	virtual void TimerProc(void) { }	//	コールバックされる関数

protected:
	UINT		m_hTimer;
	UINT		m_uTimer;
	DWORD		m_dwIntervalMin;	//	タイマcall backの最小インターバル
	bool		m_bStop;			//	StartMで回しているタイマの制御用

	DWORD		m_dwDelay;			//	StartMで使う
	DWORD		m_dwInterval;

private:
	//	TimerProcへのJump台　（マルチメディアタイマのとき）
	static void CALLBACK StaticTimeProc(UINT uID,UINT uMsg,DWORD dwUser,DWORD dw1,DWORD dw2);
	//	TimerProcへのJump台　（通常のインターバルタイマのとき）
	static void CALLBACK StaticTimerProc(HWND hwnd,UINT uMsg,UINT idEvent,DWORD dwTime);
	//	TimerProcへのJump台　（マルチメディアタイマでワンショットタイマのとき）
	static void CALLBACK StaticTimeProcM(UINT uID,UINT uMsg,DWORD dwUser,DWORD dw1,DWORD dw2);
};

#endif
