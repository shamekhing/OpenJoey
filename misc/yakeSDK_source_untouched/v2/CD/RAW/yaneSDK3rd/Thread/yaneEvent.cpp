#include "stdafx.h"
#include "yaneEvent.h"

void	CEvent::SetEvent(){
	::SetEvent(m_hHandle);
}

void	CEvent::ResetEvent(){
	::ResetEvent(m_hHandle);
}

LRESULT	CEvent::Wait(int nTime){
	DWORD dwResult;
	if (nTime == 0){
		dwResult = ::WaitForSingleObject(m_hHandle,INFINITE);
	} else {
		dwResult = ::WaitForSingleObject(m_hHandle,nTime);
	}

	switch (dwResult){
	case WAIT_OBJECT_0	:	//	所有権が獲得できた
	case WAIT_ABANDONED :	//	解放されたイベント
		return 0;
	case WAIT_TIMEOUT	:	//	タイムアウトになった
		return 1;
	default:
		//	しょれ以外って何よ？？
		return 2;
	}
}

CEvent::CEvent(){
	m_hHandle = ::CreateEvent(NULL,FALSE,FALSE,NULL);
}

CEvent::~CEvent(){
	if (m_hHandle!=NULL){
		::CloseHandle(m_hHandle);
	}
}

