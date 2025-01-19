#include "stdafx.h"
#include "yaneAppManager.h"
#include "yaneDebugWindow.h"	//	デバッグウィンドゥ(CDbg)は特殊なのでここでincludeしておく

CAppManageList CAppManager::m_alpInfo;
CCriticalSection CAppManager::m_oCriticalSection;
int	CAppManager::m_nRef = 0;

//////////////////////////////////////////////////////////////////////////////
void CAppManager::Inc(){
	m_oCriticalSection.Enter();
	{
		m_nRef++;
	}
	m_oCriticalSection.Leave();
}

void CAppManager::Dec(){
	m_oCriticalSection.Enter();
	{
		m_nRef--;
	}
	m_oCriticalSection.Leave();
}

int CAppManager::GetRef(){
	int n;
	m_oCriticalSection.Enter();
	{
		n = m_nRef;
	}
	m_oCriticalSection.Leave();
	return n;
}
//////////////////////////////////////////////////////////////////////////////

void CAppManager::Add(CAppBase*lpAppBase){

	DWORD	dwThreadID = ::GetCurrentThreadId();

	m_oCriticalSection.Enter();
	{
		CAppManageList::iterator it = m_alpInfo.begin();
		while(it!=m_alpInfo.end()) {
			if ((*it)->m_lpAppBase == lpAppBase) {
				if ((*it)->m_dwThreadID1 == NULL) { (*it)->m_dwThreadID1 = dwThreadID; goto Exit; }
//				if ((*it)->m_dwThreadID2 == NULL) { (*it)->m_dwThreadID2 = dwThreadID; goto Exit; }
//	マルチスレッドやめました＾＾
				Err.Out("CAppManager::Addするのに空きが無い");
				goto Exit;
			}
			it++;
		}

		{ // infoの変数スコープを制限するための { }
			CAppManageInfo* info = new CAppManageInfo;
			info->m_lpAppBase	=	lpAppBase;	
#ifdef USE_DirectDraw
			info->m_lpDirectDraw=	NULL;
#endif
#ifdef USE_FastDraw
			info->m_lpFastDraw=	NULL;
#endif
#ifdef USE_DIB32
			info->m_lpDIBDraw	=	NULL;
#endif
#ifdef USE_SAVER
			info->m_lpSaverDraw	=	NULL;
#endif
			info->m_lpAppFrame	=	NULL;
			info->m_dwThreadID1	=	dwThreadID;
//			info->m_dwThreadID2	=	NULL;
//	マルチスレッドやめました＾＾
			m_alpInfo.insert(info);
		}

Exit:;
	}
	m_oCriticalSection.Leave();

}

void CAppManager::Add(CAppFrame*lpAppFrame){

	DWORD	dwThreadID = ::GetCurrentThreadId();
	m_oCriticalSection.Enter();
	{
		CAppManageList::iterator it = m_alpInfo.begin();
		while(it!=m_alpInfo.end()) {
			if (((*it)->m_dwThreadID1 == dwThreadID) /* ||
				((*it)->m_dwThreadID2 == dwThreadID) */ ) {
				//	同じスレッドIDのチャンクを見つけて、そこに追加する
				(*it)->m_lpAppFrame = lpAppFrame;
				//	一つのスレッドに対して２つのCAppFrameのインスタンスが同時に
				//	存在することは想定していない
				break;
			}
			it++;
		}
	}
	m_oCriticalSection.Leave();
}

////////////////////////////////////////////////////////////

#ifdef USE_DirectDraw
void CAppManager::Add(CDirectDraw* lpDirectDraw){

	DWORD	dwThreadID = ::GetCurrentThreadId();

	m_oCriticalSection.Enter();
	{
		CAppManageList::iterator it = m_alpInfo.begin();
		while(it!=m_alpInfo.end()) {
			if ((*it)->m_dwThreadID1 == dwThreadID /* ||
				(*it)->m_dwThreadID2 == dwThreadID */ ) { (*it)->m_lpDirectDraw = lpDirectDraw; goto Exit; }
//	マルチスレッドやめました＾＾
			it++;
		}
		//	一致するスレッドが無いとはどうゆうこっちゃ？
		//	（CDirectDrawのコンストラクタが変なところで実行された？）
		Err.Out("CAppManager::Addするのに空きが無い");
Exit:;
	}
	m_oCriticalSection.Leave();

}

void CAppManager::Del(CDirectDraw*lpDraw) {

	DWORD	dwThreadID = ::GetCurrentThreadId();

	m_oCriticalSection.Enter();
	{
		CAppManageList::iterator it = m_alpInfo.begin();
		while(it!=m_alpInfo.end()) {
			if (((*it)->m_dwThreadID1 == dwThreadID) && ((*it)->m_lpDirectDraw == lpDraw)){
				(*it)->m_lpDirectDraw = NULL;
			}
//	マルチスレッドやめました＾＾
			it++;
		}
	}
	m_oCriticalSection.Leave();
}
CDirectDraw* CAppManager::GetMyDirectDraw(){
	CAppManageList::iterator it = m_alpInfo.begin();
	DWORD dwThreadID = ::GetCurrentThreadId();

	CDirectDraw* lpDraw = NULL;
	m_oCriticalSection.Enter();
	{
		while (it!=m_alpInfo.end()) {
			if ((*it)->m_dwThreadID1 == dwThreadID /* ||
				(*it)->m_dwThreadID2 == dwThreadID */ ) {
//	マルチスレッドやめました＾＾
				lpDraw = (*it)->m_lpDirectDraw;
				break;
			}
			it++;
		}
	}
	m_oCriticalSection.Leave();
	return lpDraw;		//	return null if it was not found...
}

#endif	//	USE_DirectDraw

////////////////////////////////////////////////////////////

#ifdef USE_FastDraw
void CAppManager::Add(CFastDraw* lpFastDraw){

	DWORD	dwThreadID = ::GetCurrentThreadId();

	m_oCriticalSection.Enter();
	{
		CAppManageList::iterator it = m_alpInfo.begin();
		while(it!=m_alpInfo.end()) {
			if ((*it)->m_dwThreadID1 == dwThreadID /* ||
				(*it)->m_dwThreadID2 == dwThreadID */ ) { (*it)->m_lpFastDraw = lpFastDraw; goto Exit; }
//	マルチスレッドやめました＾＾
			it++;
		}
		//	一致するスレッドが無いとはどうゆうこっちゃ？
		//	（CDirectDrawのコンストラクタが変なところで実行された？）
		Err.Out("CAppManager::Addするのに空きが無い");
Exit:;
	}
	m_oCriticalSection.Leave();

}

void CAppManager::Del(CFastDraw*lpDraw) {

	DWORD	dwThreadID = ::GetCurrentThreadId();

	m_oCriticalSection.Enter();
	{
		CAppManageList::iterator it = m_alpInfo.begin();
		while(it!=m_alpInfo.end()) {
			if (((*it)->m_dwThreadID1 == dwThreadID) && ((*it)->m_lpFastDraw == lpDraw)){
				(*it)->m_lpFastDraw = NULL;
			}
//	マルチスレッドやめました＾＾
			it++;
		}
	}
	m_oCriticalSection.Leave();
}

CFastDraw* CAppManager::GetMyFastDraw(){
	CAppManageList::iterator it = m_alpInfo.begin();
	DWORD dwThreadID = ::GetCurrentThreadId();

	CFastDraw* lpDraw = NULL;
	m_oCriticalSection.Enter();
	{
		while (it!=m_alpInfo.end()) {
			if ((*it)->m_dwThreadID1 == dwThreadID /* ||
				(*it)->m_dwThreadID2 == dwThreadID */ ) {
//	マルチスレッドやめました＾＾
				lpDraw = (*it)->m_lpFastDraw;
				break;
			}
			it++;
		}
	}
	m_oCriticalSection.Leave();
	return lpDraw;		//	return null if it was not found...
}

#endif	//	USE_FastDraw

	/////////////////////////////////////////////////////////////////

void CAppManager::Del(CAppBase*app) {

	DWORD	dwThreadID = ::GetCurrentThreadId();

	m_oCriticalSection.Enter();
	{
		CAppManageList::iterator it = m_alpInfo.begin();
		while(it!=m_alpInfo.end()) {
			if ((*it)->m_lpAppBase == app) {
				if ((*it)->m_dwThreadID1 == dwThreadID) { (*it)->m_dwThreadID1 = NULL; }
//				if ((*it)->m_dwThreadID2 == dwThreadID) { (*it)->m_dwThreadID2 = NULL; }
//	マルチスレッドやめました＾＾
#if 0
				if ((*it)->m_dwThreadID1 == NULL /* && (*it)->m_dwThreadID2 == NULL */ ) {
				//	DELETE_SAFE(*it);
				//	↑auto_ptrなので安全～
					it = m_alpInfo.erase(it);
				} else {
					it++;
				}
#endif
				//	マルチスレッドやめたので、スレッドの存在にかかわらず
				//	これを削除出来て良い。
				it = m_alpInfo.erase(it);
			} else {
				it++;
			}
		}
	}
	m_oCriticalSection.Leave();
}

void CAppManager::Del(CAppFrame*lpAppFrame){

	DWORD	dwThreadID = ::GetCurrentThreadId();
	m_oCriticalSection.Enter();
	{
		CAppManageList::iterator it = m_alpInfo.begin();
		while(it!=m_alpInfo.end()) {
			if ((*it)->m_lpAppFrame == lpAppFrame) {
				(*it)->m_lpAppFrame = NULL;	//	すべてに対して行なう
			}
			it++;
		}
	}
	m_oCriticalSection.Leave();
}

#ifdef USE_DIB32
void CAppManager::Add(CDIBDraw* lpDIBDraw){

	DWORD	dwThreadID = ::GetCurrentThreadId();

	m_oCriticalSection.Enter();
	{
		CAppManageList::iterator it = m_alpInfo.begin();
		while(it!=m_alpInfo.end()) {
			if ((*it)->m_dwThreadID1 == dwThreadID /* ||
				(*it)->m_dwThreadID2 == dwThreadID */ ) { (*it)->m_lpDIBDraw = lpDIBDraw; goto Exit; }
//	マルチスレッドやめました＾＾
			it++;
		}
		//	一致するスレッドが無いとはどうゆうこっちゃ？
		//	（CDIBDrawのコンストラクタが変なところで実行された？）
		Err.Out("CAppManager::Addするのに空きが無い");
Exit:;
	}
	m_oCriticalSection.Leave();

}
void CAppManager::Del(CDIBDraw*lpDraw) {

	DWORD	dwThreadID = ::GetCurrentThreadId();

	m_oCriticalSection.Enter();
	{
		CAppManageList::iterator it = m_alpInfo.begin();
		while(it!=m_alpInfo.end()) {
			if (((*it)->m_dwThreadID1 == dwThreadID) && ((*it)->m_lpDIBDraw == lpDraw)){
				(*it)->m_lpDIBDraw = NULL;
			}
			it++;
		}
	}
	m_oCriticalSection.Leave();
}
CDIBDraw* CAppManager::GetMyDIBDraw(){
	CAppManageList::iterator it = m_alpInfo.begin();
	DWORD dwThreadID = ::GetCurrentThreadId();

	CDIBDraw* lpDraw = NULL;
	m_oCriticalSection.Enter();
	{
		while (it!=m_alpInfo.end()) {
			if ((*it)->m_dwThreadID1 == dwThreadID /* ||
				(*it)->m_dwThreadID2 == dwThreadID */ ) {
//	マルチスレッドやめました＾＾
				lpDraw = (*it)->m_lpDIBDraw;
				break;
			}
			it++;
		}
	}
	m_oCriticalSection.Leave();
	return lpDraw;		//	return null if it was not found...
}
#endif

#ifdef USE_SAVER
void CAppManager::Add(CSaverDraw* lpSaverDraw){

	DWORD	dwThreadID = ::GetCurrentThreadId();

	m_oCriticalSection.Enter();
	{
		CAppManageList::iterator it = m_alpInfo.begin();
		while(it!=m_alpInfo.end()) {
			if ((*it)->m_dwThreadID1 == dwThreadID /* ||
				(*it)->m_dwThreadID2 == dwThreadID */ ) { (*it)->m_lpSaverDraw = lpSaverDraw; goto Exit; }
//	マルチスレッドやめました＾＾
			it++;
		}
		//	一致するスレッドが無いとはどうゆうこっちゃ？
		//	（CDIBDrawのコンストラクタが変なところで実行された？）
		Err.Out("CAppManager::Addするのに空きが無い");
Exit:;
	}
	m_oCriticalSection.Leave();

}
void CAppManager::Del(CSaverDraw*lpDraw) {

	DWORD	dwThreadID = ::GetCurrentThreadId();

	m_oCriticalSection.Enter();
	{
		CAppManageList::iterator it = m_alpInfo.begin();
		while(it!=m_alpInfo.end()) {
			if (((*it)->m_dwThreadID1 == dwThreadID) && ((*it)->m_lpSaverDraw == lpDraw)){
				(*it)->m_lpSaverDraw = NULL;
			}
			it++;
		}
	}
	m_oCriticalSection.Leave();
}
CSaverDraw* CAppManager::GetMySaverDraw(){
	CAppManageList::iterator it = m_alpInfo.begin();
	DWORD dwThreadID = ::GetCurrentThreadId();

	CSaverDraw* lpDraw = NULL;
	m_oCriticalSection.Enter();
	{
		while (it!=m_alpInfo.end()) {
			if ((*it)->m_dwThreadID1 == dwThreadID /* ||
				(*it)->m_dwThreadID2 == dwThreadID */ ) {
//	マルチスレッドやめました＾＾
				lpDraw = (*it)->m_lpSaverDraw;
				break;
			}
			it++;
		}
	}
	m_oCriticalSection.Leave();
	return lpDraw;		//	return null if it was not found...
}
#endif

CAppBase* CAppManager::GetMyApp(){
	CAppBase* app = NULL;
	m_oCriticalSection.Enter();
	{
		CAppManageList::iterator it = m_alpInfo.begin();
		DWORD dwThreadID = ::GetCurrentThreadId();
		while (it!=m_alpInfo.end()) {
			if ((*it)->m_dwThreadID1 == dwThreadID /* ||
				(*it)->m_dwThreadID2 == dwThreadID */ ) {
//	マルチスレッドやめました＾＾
				app = (*it)->m_lpAppBase;
				break;
			}
			it++;
		}
	}
	m_oCriticalSection.Leave();
	return app;		//	return null if it was not found...
}

CAppFrame* CAppManager::GetMyFrame(){
	CAppFrame* app = NULL;
	m_oCriticalSection.Enter();
	{
		CAppManageList::iterator it = m_alpInfo.begin();
		DWORD dwThreadID = ::GetCurrentThreadId();
		while (it!=m_alpInfo.end()) {
			if ((*it)->m_dwThreadID1 == dwThreadID /* ||
				(*it)->m_dwThreadID2 == dwThreadID */ ) {
//	マルチスレッドやめました＾＾
				app = (*it)->m_lpAppFrame;
				break;
			}
			it++;
		}
	}
	m_oCriticalSection.Leave();
	return app;		//	return null if it was not found...
}


//////////////////////////////////////////////////////////////////////////////

bool	CAppManager::IsDirectDraw() {

#ifdef USE_DirectDraw
	CDirectDraw*lp = CAppManager::GetMyDirectDraw();
	if (lp!=NULL) {
		return true;
	}
#endif

#ifdef USE_DIB32
	{
		CDIBDraw*lp = CAppManager::GetMyDIBDraw();
		if (lp!=NULL) {
			return false;
		}
	}
#endif

#ifdef USE_FastDraw
	{
		CFastDraw*lp = CAppManager::GetMyFastDraw();
		if (lp!=NULL) {
			return false;
		}
	}
#endif

#ifdef USE_SAVER
	{
		CSaverDraw*lp = CAppManager::GetMySaverDraw();
		if (lp!=NULL) {
			return false;
		} else {
			WARNING(true,"CAppManager::IsDirectDrawでCPlaneとCDIB32と、どちらが有効なのかかわからない");
			return false;
		}
	}
#endif

	WARNING(true,"CAppManager::IsDirectDrawでCPlaneとCDIB32と、どちらが有効なのかかわからない");
	return false;
}

int	CAppManager::GetDrawType() {

#ifdef USE_DirectDraw
	CDirectDraw*lp = CAppManager::GetMyDirectDraw();
	if (lp!=NULL) {
		return 1;
	}
#endif

#ifdef USE_DIB32
	{
		CDIBDraw*lp = CAppManager::GetMyDIBDraw();
		if (lp!=NULL) {
			return 2;
		}
	}
#endif

#ifdef USE_FastDraw
	{
		CFastDraw*lp = CAppManager::GetMyFastDraw();
		if (lp!=NULL) {
			return 3;
		}
	}
#endif

#ifdef USE_SAVER
	{
		CSaverDraw*lp = CAppManager::GetMySaverDraw();
		if (lp!=NULL) {
			return 4;
		}
	}
#endif

	return 0;
}

//////////////////////////////////////////////////////////////////////////////

int	CAppManager::GetAppInstanceNum(){
	int n;
	m_oCriticalSection.Enter();
	{
		n = m_alpInfo.size();
	}
	m_oCriticalSection.Leave();
	return n;
}

void CAppManager::StopAllThread(){
	
	//	まずは、全スレッドに停止信号を送る
	CAppManageList::iterator it;
	m_oCriticalSection.Enter();
	{
		it = m_alpInfo.begin();
		while (it!=m_alpInfo.end()) {
			static_cast<CThread*>((*it)->m_lpAppBase)->InvalidateThread();
			it++;
		}
	}
	m_oCriticalSection.Leave();

#if 0
	int nThreadNum;
	while (true) {		//	全スレッドが実行を停止したのか
		nThreadNum = 0;
//	マルチスレッドやめました＾＾
		m_oCriticalSection.Enter();
		{
		/*
			it = m_alpInfo.begin();
			while (it!=m_alpInfo.end()) {
				if (static_cast<CThread*>((*it)->m_lpAppBase)->IsThreadExecute()) nThreadNum++;
				if (static_cast<CThread*>((*it)->m_lpAppBase)->IsMessage()) nThreadNum++;
				it++;
			}
		*/
			nThreadNum = m_alpInfo.size();

		}
		m_oCriticalSection.Leave();

		//	自分の処理中のメッセージスレッドが一つあるので、
		//	それを除いた数になることが終了条件である。
		//	⇒マルチスレッドやめたので、０になることが終了条件！

		if (nThreadNum == 0) break;
		::Sleep(100);
	}
#endif
	//	↑このコードやめて、CAppBaseのインスタンスがひとつであることを保証したほうが
	//	良いと思う。
	//	CAppBase派生クラスのインスタンスにおいては、
	//	基底クラスであるCAppBaseのデストラクタがあとで呼び出されるので
	//	CAppBaseのデストラクタでCAppManager::Decされるときは、
	//	その派生クラスのデストラクタは終了していることが保証される。
	while (true){
		if (CAppManager::GetRef() <= 1+CDbg::GetInstanceCount()) break;
		::Sleep(100);
	}
	//	↑全スレッドが停止したのを確認してから
	CDbg::Release();
}
