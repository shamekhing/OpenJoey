#include "stdafx.h"
#include "yaneCounter.h"
#include <limits.h>

//////////////////////////////////////////////////////////////////////////////

CRootCounter::CRootCounter() {
	Set(0,INT_MAX,1);
	Reset();
}

CRootCounter::CRootCounter(int nEnd){
	Set(0,nEnd,1);
	Reset();
}

CRootCounter::CRootCounter(int nStart,int nEnd,int nStep){
	Set(nStart,nEnd,nStep);
	Reset();
}

void CRootCounter::inc(bool bAdd){
	bool bInc = (m_nStart > m_nEnd) ^ bAdd; // 逆方向カウンタ？
	if (bInc) {
	//	インクリメント
		if (m_nStep>0) {
		//	整数インクリメント
			m_nRootCount += m_nStep;
		} else {
		//	分数インクリメント
			m_nRate++; if (m_nRate>=(-m_nStep)) { m_nRate = 0; m_nRootCount++; }
		}
		//	サチュレートしたのか？
		int nMax = m_nStart < m_nEnd ? m_nEnd : m_nStart;
		if (m_nRootCount > nMax) m_nRootCount = nMax;
	} else {
	//	デクリメント
		if (m_nStep>0) {
		//	整数デクリメント
			m_nRootCount -= m_nStep;
		} else {
		//	分数デクリメント
			m_nRate--; if (m_nRate<=(m_nStep)) { m_nRate = 0; m_nRootCount--; }
			//	⇒　m_nRate++でないことに注意。
			//	++のあと--して、数の整合性がとれなくてはならない
			//	かつ、nStep<0のとき最初の１回目の--でRootCounterが
			//	1減ってはいけない。よってこういう実装になる
		}
		//	サチュレートしたのか？
		int nMin = m_nStart < m_nEnd ? m_nStart : m_nEnd;
		if (m_nRootCount < nMin) m_nRootCount = nMin;
	}
}
//////////////////////////////////////////////////////////////////////////////

CInteriorCounter::CInteriorCounter(){
	m_nStart	= 0;
	m_nEnd		= 0;
	m_nNow		= 0;
	m_nFrames	= 0;
	m_nFramesNow = 0;
}

void	CInteriorCounter::Set(int nStart,int nEnd,int nFrames){
	WARNING(nFrames == 0,"CInteriorCounter::SetでnFrames == 0");
	m_nStart	= nStart;
	m_nEnd		= nEnd;
	m_nNow		= nStart;
	m_nFrames	= nFrames;
	m_nFramesNow = 0;
}

void	CInteriorCounter::Inc(){
	//	カウンタは終了値か？
	if (m_nFramesNow >= m_nFrames) {
		m_nNow = m_nEnd;
		return ;
	}
	m_nFramesNow++;
	//	内分処理
	m_nNow =  m_nStart + m_nFramesNow * (m_nEnd-m_nStart) / m_nFrames;
}

void	CInteriorCounter::Dec(){
	//	カウンタは初期値か？
	if (m_nFramesNow == 0) return ;
	m_nFramesNow--;
	//	内分処理
	m_nNow =  m_nStart + m_nFramesNow * (m_nEnd-m_nStart) / m_nFrames;
}
//////////////////////////////////////////////////////////////////////////////
