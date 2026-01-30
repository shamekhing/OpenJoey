#include "stdafx.h"
#include "yaneCounter.h"
#include <limits.h>

namespace yaneuraoGameSDK3rd {
namespace Math {

//////////////////////////////////////////////////////////////////////////////
//	static members..

smart_ptr<ICounterFactory> ICounter::m_vFactory;

//////////////////////////////////////////////////////////////////////////////

ICounter* ICounter::CreateInstance(int nType){
	switch(nType){
	case 0 : return new CNullCounter;
	case 1 : return new CRootCounter;
	case 2 : return new CSaturationCounter;
	case 3 : return new CInteriorCounter;
	default :
		if (!m_vFactory.isNull()){
			ICounter* p = m_vFactory->CreateInstance(nType);
			if (p!=NULL) return p;
		}
#ifdef USE_EXCEPTION
		throw CRuntimeException();
		//	^CáOð­¶
#else
		return NULL;
#endif
	}
}

void CProxyCounter::Serialize(ISerialize&s){
	if (s.IsStoring()){
		int nType = GetCounter()->GetType();
		s << nType;	//	RTTI
		s << *GetCounter();
	} else {
		int nType;
		s << nType;			//	^©çfactoryÉæÁÄ³
		m_vCounter.Add(ICounter::CreateInstance(nType));
		s << *GetCounter();
	}
}

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
	bool bInc = (m_nStart > m_nEnd) ^ bAdd; // tûüJE^H
	if (bInc) {
	//	CNg
		if (m_nStep>0) {
		//	®CNg
			m_nRootCount += m_nStep;
		} else {
		//	ªCNg
			m_nRate++; if (m_nRate>=(-m_nStep)) { m_nRate = 0; m_nRootCount++; }
		}
		//	T`[gµ½Ì©H
		int nMax = m_nStart < m_nEnd ? m_nEnd : m_nStart;
		if (m_nRootCount > nMax) {
			int nMin = m_nStart < m_nEnd ? m_nStart : m_nEnd;
			m_nRootCount = nMin;
			//		CRootCounterÆÍ±±ªªá¤¾¯
		}
	} else {
	//	fNg
		if (m_nStep>0) {
		//	®fNg
			m_nRootCount -= m_nStep;
		} else {
		//	ªfNg
			m_nRate--; if (m_nRate<=(m_nStep)) { m_nRate = 0; m_nRootCount--; }
			//	Ë@m_nRate++ÅÈ¢±ÆÉÓB
			//	++Ì Æ--µÄAÌ®«ªÆêÈ­ÄÍÈçÈ¢
			//	©ÂAnStep<0ÌÆ«ÅÌPñÚÌ--ÅRootCounterª
			//	1¸ÁÄÍ¢¯È¢BæÁÄ±¤¢¤ÀÉÈé
		}
		//	T`[gµ½Ì©H
		int nMin = m_nStart < m_nEnd ? m_nStart : m_nEnd;
		if (m_nRootCount < nMin) {
			int nMax = m_nStart < m_nEnd ? m_nEnd : m_nStart;
			m_nRootCount = nMax;
			//		CRootCounterÆÍ±±ªªá¤¾¯
		}
	}
}

//////////////////////////////////////////////////////////////////////////////

CSaturationCounter::CSaturationCounter() {
	Set(0,INT_MAX,1);
	Reset();
}

CSaturationCounter::CSaturationCounter(int nEnd){
	Set(0,nEnd,1);
	Reset();
}

CSaturationCounter::CSaturationCounter(int nStart,int nEnd,int nStep){
	Set(nStart,nEnd,nStep);
	Reset();
}

void CSaturationCounter::inc(bool bAdd){
	bool bInc = (m_nStart > m_nEnd) ^ bAdd; // tûüJE^H
	if (bInc) {
	//	CNg
		if (m_nStep>0) {
		//	®CNg
			m_nRootCount += m_nStep;
		} else {
		//	ªCNg
			m_nRate++; if (m_nRate>=(-m_nStep)) { m_nRate = 0; m_nRootCount++; }
		}
		//	T`[gµ½Ì©H
		int nMax = m_nStart < m_nEnd ? m_nEnd : m_nStart;
		if (m_nRootCount > nMax) m_nRootCount = nMax;
	} else {
	//	fNg
		if (m_nStep>0) {
		//	®fNg
			m_nRootCount -= m_nStep;
		} else {
		//	ªfNg
			m_nRate--; if (m_nRate<=(m_nStep)) { m_nRate = 0; m_nRootCount--; }
			//	Ë@m_nRate++ÅÈ¢±ÆÉÓB
			//	++Ì Æ--µÄAÌ®«ªÆêÈ­ÄÍÈçÈ¢
			//	©ÂAnStep<0ÌÆ«ÅÌPñÚÌ--ÅRootCounterª
			//	1¸ÁÄÍ¢¯È¢BæÁÄ±¤¢¤ÀÉÈé
		}
		//	T`[gµ½Ì©H
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
	WARNING(nFrames == 0,"CInteriorCounter::SetÅnFrames == 0");
	m_nStart	= nStart;
	m_nEnd		= nEnd;
	m_nNow		= nStart;
	m_nFrames	= nFrames;
	m_nFramesNow = 0;
}

void	CInteriorCounter::Inc(){
	//	JE^ÍI¹l©H
	if (m_nFramesNow >= m_nFrames) {
		m_nNow = m_nEnd;
		return ;
	}
	m_nFramesNow++;
	//	àª
	m_nNow =  m_nStart + m_nFramesNow * (m_nEnd-m_nStart) / m_nFrames;
}

void	CInteriorCounter::Dec(){
	//	JE^Íúl©H
	if (m_nFramesNow == 0) return ;
	m_nFramesNow--;
	//	àª
	m_nNow =  m_nStart + m_nFramesNow * (m_nEnd-m_nStart) / m_nFrames;
}
//////////////////////////////////////////////////////////////////////////////

} // end of namespace Math
} // end of namespace yaneuraoGameSDK3rd
