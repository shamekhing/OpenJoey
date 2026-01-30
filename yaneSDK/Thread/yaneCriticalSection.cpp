#include "stdafx.h"
#include "yaneCriticalSection.h"

namespace yaneuraoGameSDK3rd {
namespace Thread {

CCriticalSection::CCriticalSection(){
	::InitializeCriticalSection(&m_csCriticalSection);
	m_dwLockedThread = (DWORD)-1;
	m_nLockCount	= 0;
}

CCriticalSection::~CCriticalSection(){
	::DeleteCriticalSection(&m_csCriticalSection);
	// Do not throw from destructor (C++ forbids it; can cause terminate).
	if (m_nLockCount!=0) {
#ifdef _DEBUG
		::OutputDebugStringA("CCriticalSection: destructor called while still entered (Enter/Leave mismatch).\n");
#endif
	}
}

void CCriticalSection::Enter(){
	::EnterCriticalSection(&m_csCriticalSection);
	m_dwLockedThread = ::GetCurrentThreadId();
	//	ªÙÈéXbh©ç±±ÉüÁÄ­é±ÆÍÅ«È¢
	//	(CriticalSectionÌè`æè)
	m_nLockCount++;
}

void CCriticalSection::Leave(){
#ifdef USE_EXCEPTION
	if (m_nLockCount==0){
		throw CRuntimeException("CCriticalSectionðEnterµÄ¢È¢ÌÉLeaveµÄ¢é");
	}
#endif
	if (--m_nLockCount==0) {
		m_dwLockedThread = (DWORD)-1;
	}
	//	ªLeaveµ½¼ãÉ¼XbhªEnter·éÂ\«ª é
	::LeaveCriticalSection(&m_csCriticalSection);
}

bool CCriticalSection::IsLockedByThisThread() const{
	DWORD dw = ::GetCurrentThreadId();
	return m_dwLockedThread == dw;
}

/////////////////////////////////////////////////////////////////////

CCriticalLock::CCriticalLock(ICriticalSection* cs): m_cs(cs),m_nLockCount(0)
{
	Enter();
}

void	CCriticalLock::Leave(){
	if (m_nLockCount-- == 0){
/*
#ifdef USE_EXCEPTION
		throw CRuntimeException("CCriticalLock::Leave(Enter³êÄ¢È¢)");
#endif
*/
		//	ª±êÍÀÍ è¤é
		//	á)CriticalLockµÄ¢éÈ©ÅA¢Á½ñLeaveµÄA»Ì Æ
		//		áOª­¶µÄEnterµÈ¨·OÉ²¯½ÈÇ..
		return ;
	}
	m_cs->Leave();
}

void	CCriticalLock::Enter(){
	m_cs->Enter();
	m_nLockCount++;
	//	ª if (++m_nLockCount==0) { m_cs->Enter(); }
	//	ÆÍ¯È¢BÈºÈçA++m_nLockCountªatomicÅÍ³¢
}

CCriticalLock::~CCriticalLock()
{
	//	guard³êÄ¢½Æ«ÌÝðú
	if (0<m_nLockCount){
		for(int i=0;i<m_nLockCount;i++) m_cs->Leave();
	}
}

} // end of namespace Thread
} // end of namespace yaneuraoGameSDK3rd
