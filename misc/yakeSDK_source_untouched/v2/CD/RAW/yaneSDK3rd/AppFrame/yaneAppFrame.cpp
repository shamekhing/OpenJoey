#include "stdafx.h"
#include "yaneAppFrame.h"
#include "../Timer/yaneTimer.h"

//	コンストラクタ～デストラクタで、CAppManagerに登録しておく
IAppFrame::IAppFrame(void) {
	CAppManager::Add(this);
}

IAppFrame::~IAppFrame(){
	CAppManager::Del(this);
}

void	CAppFrame::MesSleep(int nTime) {
	CTimer t;
	while (IsThreadValid() && t.Get()<nTime)
		::Sleep(10);	//	ちょぴっとずつ＾＾；
}

