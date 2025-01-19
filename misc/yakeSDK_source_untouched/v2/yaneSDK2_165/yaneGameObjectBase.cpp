//	yaneGameObjectBase.cpp

#include "stdafx.h"
#include "yaneGameObjectBase.h"

//////////////////////////////////////////////////////////////////////////////

set<CGameObjectBase*> CGameObjectBase::m_GameObjectList;

CGameObjectBase::CGameObjectBase(void){
	m_GameObjectList.insert(this);
	m_bValid = true;
	m_nX = m_nY = 0;
	m_nID = 0;
}

CGameObjectBase::~CGameObjectBase(){
	m_GameObjectList.erase(this);
}

void	CGameObjectBase::Kill(void) {
	m_bValid = false;
}

int		CGameObjectBase::GetReleaseLevel(void) {
	return 0;	//	ディフォルトで0
}

/////////////////////////////////////////////
//	まーこれはいるやろ、みたいな（笑）

void	CGameObjectBase::SetPos(int x,int y){
	m_nX = x; m_nY = y;
}

void	CGameObjectBase::GetPos(int&x,int&y){
	x = m_nX; y = m_nY;
}

/////////////////////////////////////////////

//	全インスタンスに関わる操作

void	CGameObjectBase::OnPreDrawAll(void){
	for(set<CGameObjectBase*>::iterator it = m_GameObjectList.begin();it!=m_GameObjectList.end();it++){
		if ((*it)->m_bValid)
			(*it)->OnPreDraw();
	}
}

void	CGameObjectBase::OnDrawAll(void){
	for(set<CGameObjectBase*>::iterator it = m_GameObjectList.begin();it!=m_GameObjectList.end();it++){
		if ((*it)->m_bValid)
			(*it)->OnDraw();
	}
}

void	CGameObjectBase::OnMoveAll(void){
	for(set<CGameObjectBase*>::iterator it = m_GameObjectList.begin();it!=m_GameObjectList.end();it++){
		if ((*it)->m_bValid)
			(*it)->OnMove();
	}
}

void	CGameObjectBase::KillAll(int nReleaseLevel){
	// このレベル以下のものをKillする
	for(set<CGameObjectBase*>::iterator it = m_GameObjectList.begin();it!=m_GameObjectList.end();it++){
		if ((*it)->GetReleaseLevel() <= nReleaseLevel){
			(*it)->Kill();
		}
	}
}

void	CGameObjectBase::Garbage(void){
	//	ガーベジする (C) yaneurao '00/08/01
	for(set<CGameObjectBase*>::iterator it = m_GameObjectList.begin();it!=m_GameObjectList.end();){
		if (!(*it)->IsValid()) {
			CGameObjectBase* lp = *it;
			it++;
			DELETE_SAFE(lp);	//	イテレータはeraseするときにその要素を
								//	指していると不正になる
		} else {
			it++;
		}
	}
}

/////////////////////////////////////////////
