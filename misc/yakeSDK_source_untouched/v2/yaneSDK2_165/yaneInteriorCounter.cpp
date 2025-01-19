//
//	CInteriorCounter
//		内分カウンタ
//

#include "stdafx.h"
#include "yaneInteriorCounter.h"

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
