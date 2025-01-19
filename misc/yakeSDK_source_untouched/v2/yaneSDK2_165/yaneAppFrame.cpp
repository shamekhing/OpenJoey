#include "stdafx.h"
#include "yaneAppFrame.h"
#include "yaneTimer.h"

void	CAppFrame::MesSleep(int nTime) {
	CTimer t;
	while (IsThreadValid() && t.Get()<nTime)
		::Sleep(10);	//	‚¿‚å‚Ò‚Á‚Æ‚¸‚ÂOOG
}

//	ƒRƒ“ƒXƒgƒ‰ƒNƒ^`ƒfƒXƒgƒ‰ƒNƒ^‚ÅACAppManager‚É“o˜^‚µ‚Ä‚¨‚­
CAppFrame::CAppFrame(void) {
	CAppManager::Add(this);
}

CAppFrame::~CAppFrame(){
	CAppManager::Del(this);
}
