
#include "stdafx.h"
#include "yaneLayer.h"
#include "yaneAppManager.h"
#include "yaneDirectDraw.h"
#include "yaneFastDraw.h"
#include "yaneDIBDraw.h"

//////////////////////////////////////////////////////////////////////////////

CLayerBase::CLayerBase() {
	m_nX = 0;
	m_nY = 0;
	m_bEnable = true;
}

CLayerBase::~CLayerBase() {
}

#ifdef USE_DirectDraw
void	CLayerBase::OnDraw(CDirectDraw*lpDraw)	{ InnerOnDraw(lpDraw); }
#endif

#ifdef USE_FastDraw
void	CLayerBase::OnDraw(CFastDraw*lpDraw)	{ InnerOnDraw(lpDraw->GetSecondary()); }
#endif

#ifdef USE_DIB32
void	CLayerBase::OnDraw(CDIBDraw*lpDraw)		{ InnerOnDraw(lpDraw); }
#endif

#ifdef USE_SAVER
void	CLayerBase::OnDraw(CSaverDraw*lpDraw)	{ InnerOnDraw(lpDraw); }
#endif
//////////////////////////////////////////////////////////////////////////////

//	デストラクタでCDirectDrawからフック外すので、
//	そのときにCDirectDrawが存在しないと死亡しないように．．．

CLayer::CLayer(){
#ifdef USE_DirectDraw
	CDirectDraw* lpDraw = CAppManager::GetMyDirectDraw();
	if (lpDraw!=NULL) lpDraw->GetLayerList()->Add(this);
#endif

#ifdef USE_FastDraw
	CFastDraw* lpFastDraw = CAppManager::GetMyFastDraw();
	if (lpFastDraw!=NULL) lpFastDraw->GetLayerList()->Add(this);
#endif

#ifdef USE_DIB32
	CDIBDraw* lpDIBDraw = CAppManager::GetMyDIBDraw();
	if (lpDIBDraw!=NULL) lpDIBDraw->GetLayerList()->Add(this);
#endif

#ifdef USE_SAVER
	CSaverDraw* lpSaverDraw = CAppManager::GetMySaverDraw();
	if (lpSaverDraw!=NULL) lpSaverDraw->GetLayerList()->Add(this);
#endif
}

CLayer::~CLayer(){
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
}

CAfterLayer::CAfterLayer(){
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
}

CAfterLayer::~CAfterLayer(){
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
}

CHDCLayer::CHDCLayer(){
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
}

CHDCLayer::~CHDCLayer(){
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
}

void CLayerList::Add(CLayerBase* hook)
{
	m_LayerPtrList.push_back(hook);
}

void CLayerList::Del(CLayerBase* hook){

	// 自分がHookしたやつを探して削除してゆく（複数ありうる）
	for(list<CLayerBase*>::iterator it=m_LayerPtrList.begin();it!=m_LayerPtrList.end();) {
		if (*it==hook) {
			it = m_LayerPtrList.erase(it);
		} else {
			it ++;
		}
	}
}

void CLayerList::Clear()
{
	m_LayerPtrList.clear();
}

bool CLayerList::IsEmpty(){
	return m_LayerPtrList.empty();
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

#ifdef USE_FastDraw
void CLayerList::OnDraw(CFastDraw* lpDraw){

	list<CLayerBase*>::iterator it=m_LayerPtrList.begin();
	while (it!=m_LayerPtrList.end()) {
		if ((*it)->IsEnable()){
			(*it)->OnDraw(lpDraw);
		}
		it ++ ;
	}
}
#endif

#ifdef USE_DIB32
void CLayerList::OnDraw(CDIBDraw* lpDraw){

	list<CLayerBase*>::iterator it=m_LayerPtrList.begin();
	while (it!=m_LayerPtrList.end()) {
		if ((*it)->IsEnable()){
			(*it)->OnDraw(lpDraw);
		}
		it ++ ;
	}
}
#endif

#ifdef USE_SAVER
void CLayerList::OnDraw(CSaverDraw* lpDraw){

	list<CLayerBase*>::iterator it=m_LayerPtrList.begin();
	while (it!=m_LayerPtrList.end()) {
		if ((*it)->IsEnable()){
			(*it)->OnDraw(lpDraw);
		}
		it ++ ;
	}
}
#endif

void CLayerList::OnDraw(HDC hDC){

	list<CLayerBase*>::iterator it=m_LayerPtrList.begin();
	while (it!=m_LayerPtrList.end()) {
		if ((*it)->IsEnable()){
			(*it)->OnDraw(hDC);
		}
		it ++ ;
	}
}
