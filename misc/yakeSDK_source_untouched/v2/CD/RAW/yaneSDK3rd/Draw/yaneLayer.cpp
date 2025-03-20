
#include "stdafx.h"
#include "yaneLayer.h"
#include "yaneSurface.h"

//////////////////////////////////////////////////////////////////////////////

//	デストラクタでCDirectDrawからフック外すので、
//	そのときにCDirectDrawが存在しないと死亡しないように．．．

// todo

ILayer::ILayer(){
/**
	CDIBDraw* lpDIBDraw = CAppManager::GetMyDIBDraw();
	if (lpDIBDraw!=NULL) lpDIBDraw->GetLayerList()->Add(this);
#endif

#ifdef USE_SAVER
	CSaverDraw* lpSaverDraw = CAppManager::GetMySaverDraw();
	if (lpSaverDraw!=NULL) lpSaverDraw->GetLayerList()->Add(this);
#endif
*/
}

ILayer::~ILayer(){
/**
#ifdef USE_DirectDraw
	CDirectDraw* lpDraw = CAppManager::GetMyDirectDraw();
	if (lpDraw!=NULL) lpDraw->GetLayerList()->Del(this);
#endif

#ifdef USE_FastDraw
	CFastDraw* lpFastDraw = CAppManager::GetMyFastDraw();
	if (lpFastDraw!=NULL) lpFastDraw->GetLayerList()->Del(this);
#endif

#ifdef USE_DIB32
	CDIBDraw* lpDIBDraw = CAppManager::GetMyDIBDraw();
	if (lpDIBDraw!=NULL) lpDIBDraw->GetLayerList()->Del(this);
#endif

#ifdef USE_SAVER
	CSaverDraw* lpSaverDraw = CAppManager::GetMySaverDraw();
	if (lpSaverDraw!=NULL) lpSaverDraw->GetLayerList()->Del(this);
#endif
*/
}

IAfterLayer::IAfterLayer(){
/*
#ifdef USE_DirectDraw
	CDirectDraw* lpDraw = CAppManager::GetMyDirectDraw();
	if (lpDraw!=NULL) lpDraw->GetAfterLayerList()->Add(this);
#endif

#ifdef USE_FastDraw
	CFastDraw* lpFastDraw = CAppManager::GetMyFastDraw();
	if (lpFastDraw!=NULL) lpFastDraw->GetAfterLayerList()->Add(this);
#endif

#ifdef USE_DIB32
	CDIBDraw* lpDIBDraw = CAppManager::GetMyDIBDraw();
	if (lpDIBDraw!=NULL) lpDIBDraw->GetAfterLayerList()->Add(this);
#endif

#ifdef USE_SAVER
	CSaverDraw* lpSaverDraw = CAppManager::GetMySaverDraw();
	if (lpSaverDraw!=NULL) lpSaverDraw->GetAfterLayerList()->Add(this);
#endif
*/
}

IAfterLayer::~IAfterLayer(){
/*
#ifdef USE_DirectDraw
	CDirectDraw* lpDraw = CAppManager::GetMyDirectDraw();
	if (lpDraw!=NULL) lpDraw->GetAfterLayerList()->Del(this);
#endif

#ifdef USE_FastDraw
	CFastDraw* lpFastDraw = CAppManager::GetMyFastDraw();
	if (lpFastDraw!=NULL) lpFastDraw->GetAfterLayerList()->Del(this);
#endif

#ifdef USE_DIB32
	CDIBDraw* lpDIBDraw = CAppManager::GetMyDIBDraw();
	if (lpDIBDraw!=NULL) lpDIBDraw->GetAfterLayerList()->Del(this);
#endif

#ifdef USE_SAVER
	CSaverDraw* lpSaverDraw = CAppManager::GetMySaverDraw();
	if (lpSaverDraw!=NULL) lpSaverDraw->GetAfterLayerList()->Del(this);
#endif
*/
}

IHDCLayer::IHDCLayer(){
/*
#ifdef USE_DirectDraw
	CDirectDraw* lpDraw = CAppManager::GetMyDirectDraw();
	if (lpDraw!=NULL) lpDraw->GetHDCLayerList()->Add(this);
#endif

#ifdef USE_FastDraw
	CFastDraw* lpFastDraw = CAppManager::GetMyFastDraw();
	if (lpFastDraw!=NULL) lpFastDraw->GetHDCLayerList()->Add(this);
#endif

#ifdef USE_DIB32
	CDIBDraw* lpDIBDraw = CAppManager::GetMyDIBDraw();
	if (lpDIBDraw!=NULL) lpDIBDraw->GetHDCLayerList()->Add(this);
#endif

#ifdef USE_SAVER
	CSaverDraw* lpSaverDraw = CAppManager::GetMySaverDraw();
	if (lpSaverDraw!=NULL) lpSaverDraw->GetHDCLayerList()->Add(this);
#endif
*/
}

IHDCLayer::~IHDCLayer(){
/*
#ifdef USE_DirectDraw
	CDirectDraw* lpDraw = CAppManager::GetMyDirectDraw();
	if (lpDraw!=NULL) lpDraw->GetHDCLayerList()->Del(this);
#endif

#ifdef USE_FastDraw
	CFastDraw* lpFastDraw = CAppManager::GetMyFastDraw();
	if (lpFastDraw!=NULL) lpFastDraw->GetHDCLayerList()->Del(this);
#endif

#ifdef USE_DIB32
	CDIBDraw* lpDIBDraw = CAppManager::GetMyDIBDraw();
	if (lpDIBDraw!=NULL) lpDIBDraw->GetHDCLayerList()->Del(this);
#endif

#ifdef USE_SAVER
	CSaverDraw* lpSaverDraw = CAppManager::GetMySaverDraw();
	if (lpSaverDraw!=NULL) lpSaverDraw->GetHDCLayerList()->Add(this);
#endif
*/
}

void CLayerList::Add(ILayerBase* hook)
{
	GetList()->push_back(hook);
}

void CLayerList::Del(ILayerBase* hook){

	// 自分がHookしたやつを探して削除してゆく（複数ありうる）
	for(list_chain<ILayerBase>::iterator it=GetList()->begin();
		it!=GetList()->end();) {
		if (*it==hook) {
			it = GetList()->erase(it);
		} else {
			it ++;
		}
	}
}

void CLayerList::Clear()
{
	GetList()->clear();
}

bool CLayerList::IsEmpty() const {
	return const_cast<CLayerList*>(this)->GetList()->empty();
}

#ifdef USE_DirectDraw
void CLayerList::OnDraw(CDirectDraw* lpDraw){

	list<CLayerBase*>::iterator it=m_LayerPtrList.begin();
	while (it!=m_LayerPtrList.end()) {
		if ((*it)->IsEnable()){
			(*it)->OnDraw(lpDraw);
		}
		it ++ ;
	}
}
#endif

void CLayerList::OnDraw(ISurface* lpDraw){

	list_chain<ILayerBase>::iterator it=GetList()->begin();
	while (it!=GetList()->end()) {
		if ((*it)->IsEnable()){
			(*it)->OnDraw(lpDraw);
		}
		it ++ ;
	}
}

void CLayerList::OnDraw(HDC hDC){

	list_chain<ILayerBase>::iterator it=GetList()->begin();
	while (it!=GetList()->end()) {
		if ((*it)->IsEnable()){
			(*it)->OnDraw(hDC);
		}
		it ++ ;
	}
}
