#include "stdafx.h"
#include "yaneMsgSR.h"

CMsgReceiver::~CMsgReceiver(){
	//	自分に接続されているセンダーに遡及して削除
	vectorEx<CMsgSender*>::iterator it;
	for (it = m_lpaMsgSender.begin();it!=m_lpaMsgSender.end();it++){
		(*it)->m_lpaMsgReceiver.erase(this);
	}
}

void	CMsgSender::AddReceiver(CMsgReceiver*lp){		//	レシーバー追加
	//	もし、このレシーバーとの接続が未確立ならば、レシーバー側にも接続を登録する
	if (m_lpaMsgReceiver.insert(lp).second) lp->m_lpaMsgSender.insert(this);
}

void	CMsgSender::DeleteReceiver(CMsgReceiver*lp){	//	レシーバー削除
	//	もし、このレシーバーと確立していた接続を断ち切るならば、レシーバー側からも削除する
	if (m_lpaMsgReceiver.erase(lp)) lp->m_lpaMsgSender.erase(this);
}

void	CMsgSender::DeleteReceivers(void){				//	全レシーバー削除
	//	自分に接続されているレシーバーに遡及して削除
	vectorEx<CMsgReceiver*>::iterator it;
	for (it = m_lpaMsgReceiver.begin();it!=m_lpaMsgReceiver.end();it++){
		(*it)->m_lpaMsgSender.erase(this);
	}
	m_lpaMsgReceiver.clear();
}

void	CMsgSender::NotifyReceivers(void* lpArg){		//	接続されているレシーバーに全通知
	//	自分に接続されているセンダーに遡及して削除
	vectorEx<CMsgReceiver*>::iterator it;
	for (it = m_lpaMsgReceiver.begin();it!=m_lpaMsgReceiver.end();it++){
		(*it)->OnNotifiedMsg(this,lpArg);
	}
}

CMsgSender::~CMsgSender(){
	DeleteReceivers();
}
