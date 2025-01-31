//	yaneIntervalTimer.cpp
#include "stdafx.h"
#include "yaneAppIntervalTimer.h"

chain<CAppIntervalTimer> CAppIntervalTimer::m_alpTimerList;	//	コールバックすべきリスト

CAppIntervalTimer::CAppIntervalTimer(void){
	m_bStop	 = true;
}

CAppIntervalTimer::~CAppIntervalTimer(){
	Stop();
	m_alpTimerList.erase(this);	//	自前で削除しなくてはならない
}

bool	CAppIntervalTimer::IsTimer(void) const {
	return !m_bStop;
}

LRESULT CAppIntervalTimer::Start(void){
	//	setの仕様により、既に存在するのならばinsertされない
	m_alpTimerList.insert(this);
	m_bStop = false;
	return 0;
}

LRESULT CAppIntervalTimer::Stop(void){
//	m_alpTimerList.erase(this);
	//	↑ここでeraseすると、コールバックの中で困る^^;
	m_bStop = true;
	return 0;
}

void	CAppIntervalTimer::TimerCallBackAll(void){
	//	生きているすべてのタイマにコールバックをかける
	chain<CAppIntervalTimer>::iterator it = m_alpTimerList.begin();
	while (it!=m_alpTimerList.end()){
		if (!(*it)->IsTimer()) {
			//	ガーベジャーを兼ねる＾＾；
			it = m_alpTimerList.erase(it);
		} else {
			(*it)->TimerProc();
			it++;
		}
	}
}
