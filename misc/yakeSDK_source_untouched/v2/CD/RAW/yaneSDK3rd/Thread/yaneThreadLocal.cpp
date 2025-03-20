#include "stdafx.h"
#include "yaneThreadLocal.h"
#include "../AppFrame/yaneAppInitializer.h"
#include "../YTL/function_callback.h"

singleton<ThreadLocalBaseHelper> ThreadLocalBaseHelper::m_obj;

void	ThreadLocalBaseHelper::insert(ThreadLocalBase*p){
	CCriticalLock(GetCriticalSection());
	GetChain()->insert(p);
}

void	ThreadLocalBaseHelper::erase(ThreadLocalBase*p){
	CCriticalLock(GetCriticalSection());
	GetChain()->erase(p);
}

void	ThreadLocalBaseHelper::OnThreadEnd(){
	//	全てのThreadLocalのインスタンスに対してClearを呼び出す
	CCriticalLock(GetCriticalSection());
	GetChain()->for_each(ThreadLocalBase::Clear);
}

void	ThreadLocalBaseHelper::OnExit(){
	//	全てのThreadLocalのインスタンスに対してSetExit(true)を呼び出す
	CCriticalLock(GetCriticalSection());
	GetChain()->for_each(ThreadLocalBase::SetExit,true);
}

//	終了時コールバックを依頼
ThreadLocalBaseHelper::ThreadLocalBaseHelper(){
	smart_ptr<function_callback> p(
		function_callback_v::Create(&ThreadLocalBaseHelper::OnExit,this));
	CAppInitializer::RegistExitCallback(p);
}
