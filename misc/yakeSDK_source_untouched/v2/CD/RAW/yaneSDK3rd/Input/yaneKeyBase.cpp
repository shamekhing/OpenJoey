// yaneKeyBase.cpp

#include "stdafx.h"
#include "yaneKeyBase.h"

//////////////////////////////////////////////////////////////////////////////

IKeyBase::IKeyBase(){
	m_nKeyBufNo=0;	// — ‚Æ•\‚ğflip‚µ‚ÄA·•ª‚ğ‚Æ‚é‚Ì‚É—˜—p
	Reset();
}

void	IKeyBase::Reset(){
	ZERO(m_byKeyBuffer);
}

//////////////////////////////////////////////////////////////////////////////

bool IKeyBase::IsKeyPress(int key) const {
	return	(m_byKeyBuffer[m_nKeyBufNo][key]) !=0;
};

bool IKeyBase::IsKeyPush(int key) const {
	// ‰Ÿ‚³‚ê‚½uŠÔ‚Étrue‚É‚·‚éê‡
	if (!(m_byKeyBuffer[  m_nKeyBufNo][key])) return false;
	if (  m_byKeyBuffer[1-m_nKeyBufNo][key] ) return false;
	return true;
}

BYTE*	IKeyBase::GetKeyData() const {
	return	(BYTE*)&m_byKeyBuffer[m_nKeyBufNo];
}

//////////////////////////////////////////////////////////////////////////////
