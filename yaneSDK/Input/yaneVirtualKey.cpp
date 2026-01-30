//	yaneVirtualKey.cpp :

#include "stdafx.h"
#include "yaneVirtualKey.h"

namespace yaneuraoGameSDK3rd {
namespace Input {

//bool	operator <(const CVKeyBase& x,const CVKeyBase& y)  { return x.m_key < y.m_key; }

//////////////////////////////////////////////////////////////////////////////
CVirtualKey::CVirtualKey(){
	ClearKeyDevice();
	ClearKeyList();
}

CVirtualKey::~CVirtualKey() {}
//////////////////////////////////////////////////////////////////////////////

//	?L?[?f?o?C?X??o?^
void	CVirtualKey::ClearKeyDevice(){
	m_alpDevice.clear();
}

void	CVirtualKey::AddDevice(const smart_ptr<IKeyBase>& keybase){
	m_alpDevice.insert(keybase);
}

void	CVirtualKey::RemoveDevice(const smart_ptr<IKeyBase>& keybase){
	int nDeviceNo=0;
	smart_vector_ptr<IKeyBase>::iterator it = m_alpDevice.begin();
	while (true){
		if (it==m_alpDevice.end()){
			return ;	//	??�????[
		}
		if ((*it).get()==keybase.get()){
			m_alpDevice.erase(it);
			break;
		}
		it++; nDeviceNo++;
	}
	
	//	???????z?L?[??????
	for(int i=0;i<g_VIRTUAL_KEY_MAX;i++){
		CVKeyList::iterator it;
		for(it = m_VKey[i].begin();it!=m_VKey[i].end();){
			int& nDevNo = (*it)->m_nDeviceNo;
			if (nDevNo == nDeviceNo){
				it = m_VKey[i].erase(it);
			} else if (nDevNo > nDeviceNo) {
				nDevNo--;
				it++;
			} else {
				it++;
			}
		}
	}
}

smart_ptr<IKeyBase> CVirtualKey::GetDevice(int n){
	return m_alpDevice[n];
}

LRESULT	CVirtualKey::Input(){
	smart_vector_ptr<IKeyBase>::iterator it = m_alpDevice.begin();
	LRESULT lr = 0;
	while (it!=m_alpDevice.end()){
		lr |= (*it)->Input();
		it++;
	}

	FlipKeyBuffer(m_nKeyBufNo);
	::ZeroMemory(m_byKeyBuffer[m_nKeyBufNo],256);

	//	?e?f?o?C?X????e?????z?L?[????f??????
	for(int i=0;i<g_VIRTUAL_KEY_MAX;i++){
		CVKeyList::iterator keyIt;
		keyIt = m_VKey[i].begin();
		while (keyIt!=m_VKey[i].end()) {
			if (m_alpDevice[(*keyIt)->m_nDeviceNo]->IsKeyPress((*keyIt)->m_nKey)) {
				m_byKeyBuffer[m_nKeyBufNo][i] = 255;
				break;
			}
			keyIt++;
		}
	}
	return lr;
}

//////////////////////////////////////////////////////////////////////////////

//	???z?L?[?????E??
void	CVirtualKey::ClearKeyList(){
	for(int i=0;i<g_VIRTUAL_KEY_MAX;i++) {
		m_VKey[i].clear();	//	auto_ptr????A?????I?b?P?`
	}
}

void	CVirtualKey::AddKey(int vkey,int nDeviceNo,int nKey){
	CKeyInfo* lp = new CKeyInfo;
	lp->m_nDeviceNo = nDeviceNo;
	lp->m_nKey		= nKey;
	m_VKey[vkey].insert(lp);
}

void	CVirtualKey::RemoveKey(int vkey,int nDeviceNo,int nKey){
	CVKeyList::iterator it;
	for(it = m_VKey[vkey].begin();it!=m_VKey[vkey].end();){
		if (((*it)->m_nDeviceNo == nDeviceNo) &&
			((*it)->m_nKey		== nKey)){
			it = m_VKey[vkey].erase(it);
		} else {
			it++;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////

} // end of namespace Input
} // end of namespace yaneuraoGameSDK3rd
