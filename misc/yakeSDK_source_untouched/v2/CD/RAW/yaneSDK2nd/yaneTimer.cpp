//	yaneTimer.cpp
#include "stdafx.h"
#include "yaneTimer.h"

//////////////////////////////////////////////////////////////////////////////
int			CTimeBase::m_nRef = 0;			//	参照カウント
bool		CTimeBase::m_bUseTGT;
TIMECAPS	CTimeBase::m_dwTimeCaps;
//MMTIME		CTimeBase::m_mmtime;
CTimeBase	CTimeBaseInitializer;			//	こいつが起動時にCTimeBase::CTimeBase(void)を呼び出す
CTimeBase __timebase_dummy;	// コンストラクタで初期化するためのもの

CTimeBase::CTimeBase(void) {
	if (::timeGetDevCaps(&m_dwTimeCaps,sizeof(TIMECAPS))!=TIMERR_NOERROR) {
		//	なんや？タイマ腐ってんのか？
		m_dwTimeCaps.wPeriodMin = 1;	//	のつもりになる:p
		m_bUseTGT	= false;			// GetTickCountを使用する
		return ;
	}
	if (m_dwTimeCaps.wPeriodMin>=10) {
		//	これより大きいと使いものにならん
		m_dwTimeCaps.wPeriodMin = 1;		//	のつもりになる:p
		m_bUseTGT	= false;		// GetTickCountを使用する
	} else {
		m_bUseTGT	= true;			// timeGetTimeを使用する
//		m_mmtime.wType = TIME_MS;	//	timeGetSystemTime
	}
}

//////////////////////////////////////////////////////////////////////////////

//	timeBeginPeriodとtimeEndPeriodは同じ分解能にて対応させること。
//	また、同じ分解能に対してはネスティング出来ない。（逆アセして調べた）

int CTimeBase::timeBeginPeriodMin(void){

	//---	（コンストラクタでやっているのと同じ内容）
	::timeGetDevCaps(&m_dwTimeCaps,sizeof(TIMECAPS));
	if (m_dwTimeCaps.wPeriodMin>=10) {
	//	これより大きいと使いものにならん
		m_dwTimeCaps.wPeriodMin = 1;		//	のつもりになる:p
		m_bUseTGT	= false;		// GetTickCountを使用する
	} else {
		m_bUseTGT	= true;			// timeGetTimeを使用する
//		m_mmtime.wType = TIME_MS;	//	timeGetSystemTime
	}
	//---

	if (m_nRef++==0 && m_bUseTGT) {
		// timeGetTimeを使わないならばtimeBeginPeriodは無意味
		::timeBeginPeriod(m_dwTimeCaps.wPeriodMin);
	}
	return m_dwTimeCaps.wPeriodMin;
}

void CTimeBase::timeEndPeriodMin(void){
	if (--m_nRef==0 && m_bUseTGT) {
		::timeEndPeriod(m_dwTimeCaps.wPeriodMin);
	}
}

//////////////////////////////////////////////////////////////////////////////

set<CTimer*>	CTimer::m_aTimer;	//	全インスタンスのチェーン

CTimer::CTimer(void){
	m_aTimer.insert(this);	//	チェインにインスタンス追加
	Reset();
}

CTimer::~CTimer(){
	m_aTimer.erase(this);
}

DWORD CTimer::Get(void) {
	if (m_bPaused>0) return m_dwPauseTime-m_dwOffsetTime;
	return (DWORD)(CTimeBase::timeGetTime()-m_dwOffsetTime);
}

void CTimer::Set(DWORD dw) {
	if (m_bPaused>0) {
		//	いらんかな～？
		m_dwOffsetTime = m_dwPauseTime-dw;
	} else {
		m_dwOffsetTime = (DWORD)(CTimeBase::timeGetTime() - dw);
	}
}

void CTimer::Reset(void) {
	m_dwOffsetTime = CTimeBase::timeGetTime();
	m_bPaused = 0;
}

void CTimer::Pause(void){
	if (m_bPaused++ == 0) {
	//	Pauseさせる
		m_dwPauseTime = CTimeBase::timeGetTime();
	}
}

void CTimer::Restart(void){
	if (--m_bPaused == 0) {
	//	Pause解除
		m_dwOffsetTime += CTimeBase::timeGetTime() - m_dwPauseTime;
	}
}

//////////////////////////////////////////////////////////////////////////////
LRESULT CTimer::PauseAll(void){
	if (m_aTimer.empty()) return 0;
	for (set<CTimer*>::iterator it=m_aTimer.begin();it!=m_aTimer.end();it++){
		(*it)->Pause();
	}
	return 0;
}

LRESULT CTimer::RestartAll(void){
	if (m_aTimer.empty()) return 0;
	for (set<CTimer*>::iterator it=m_aTimer.begin();it!=m_aTimer.end();it++){
		(*it)->Restart();
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////////
//	class CTimerEx

// static members..
DWORD	CTimerEx::m_dwTimeGetTime = 0;

DWORD	CTimerEx::Get(void){
	if (m_bPaused>0) return m_dwPauseTime-m_dwOffsetTime;
	return (DWORD)(m_dwTimeGetTime-m_dwOffsetTime);
}

void	CTimerEx::Flush(void){
	m_dwTimeGetTime = CTimeBase::timeGetTime();
}

void	CTimerEx::Set(DWORD dw) {
	if (m_bPaused>0) {
		//	いらんかな～？
		m_dwOffsetTime = m_dwPauseTime-dw;
	} else {
		m_dwOffsetTime = (DWORD)(m_dwTimeGetTime - dw);
	}
}

void	CTimerEx::Reset(void) {
	m_dwOffsetTime = m_dwTimeGetTime;
	m_bPaused = 0;
}

void	CTimerEx::Pause(void){
	if (m_bPaused++ == 0) {
	//	Pauseさせる
		m_dwPauseTime = m_dwTimeGetTime;
	}
}

void	CTimerEx::Restart(void){
	if (--m_bPaused == 0) {
	//	Pause解除
		m_dwOffsetTime += m_dwTimeGetTime - m_dwPauseTime;
	}
}
