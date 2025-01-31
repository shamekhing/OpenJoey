#include "stdafx.h"
#include "yaneCriticalSection.h"

CCriticalSection::CCriticalSection(void){
	::InitializeCriticalSection(&m_oCriticalSection);
}

CCriticalSection::~CCriticalSection(){
	::DeleteCriticalSection(&m_oCriticalSection);
}

void CCriticalSection::Enter(void){
	::EnterCriticalSection(&m_oCriticalSection);
}

void CCriticalSection::Leave(void){
	::LeaveCriticalSection(&m_oCriticalSection);
}
