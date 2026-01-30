#include "stdafx.h"

#include "yaneAppBase.h"
#include "yaneAppManager.h"
#include "yaneAppInitializer.h"

//	IAppBase*	IAppBase::m_lpMainApp=NULL;

//////////////////////////////////////////////////////////////////////////////

namespace yaneuraoGameSDK3rd {
namespace AppFrame {

CAppBase::CAppBase() {
	m_bIdle		=	false;
	m_bMessage	=	false;
	m_bWaitIfMinimized = false;
	m_bClose	=	false;	//	OnPreClose�𖳎�����Close�����ԂȂ̂��H
	m_hAccel	=	NULL;
	m_bMainApp	=	false;

	CAppManager::Inc();	//	�Q�ƃJ�E���g�̃C���N�������g
}

CAppBase::~CAppBase() {
	StopThread();
	CAppManager::Dec();	//	�Q�ƃJ�E���g�̃f�N�������g
	//	���̃N���X�́ACThread�h���N���X�Ȃ̂ŁA
	//	��Dec��StopThread����ɂȂ��Ă��܂��B
	//	����āA��s���āAStopThread���Ăяo�����Ƃɂ���āA
	//	�X���b�h�̒�~���ۏ؂����B
}

//////////////////////////////////////////////////////////////////////////////

LRESULT CAppBase::OnPreCreate(CWindowOption& opt){

	opt.caption		= "���Ղ肿���";
	opt.classname	= "YANEAPPLICATION";
	opt.size_x		= 800;
	opt.size_y		= 600;
	opt.style		= WS_MINIMIZEBOX | WS_CAPTION | WS_SYSMENU;

	return 0;
}

//////////////////////////////////////////////////////////////////////////////

LRESULT CAppBase::Run(){
	//	���ꂪ���C���X�^���X�Ȃ�΁A�����e�E�B���h�D�Ƃ݂Ȃ�
//	if (m_lpMainApp==NULL) m_lpMainApp = this;
	/**
		�E�B���h�D�Ɏ�]�֌W�͂Ȃ��Ȃ����̂�
		���C���Ƃ��T�u�Ƃ�������ʎ��̂��Ȃ�
	*/

/*
	if (IsMainApp()) {
		return JumpToThread();
	}
	//	���C���E�B���h�D�ȊO�Ȃ�΁A�����p�ɃX���b�h����遫
*/
	/**
		���̃t���[����yaneSDK3rd�ł͔p�~
		�K���X���b�h�𐶐�����悤�ɕύX
	*/

	m_nThreadStatus = -1;
	if (CreateThread()) return 1;

	//	�E�B���h�D�̊����܂ő҂�
	while (true){
		if (m_bMessage || m_nThreadStatus>=0) break;
		::Sleep(100);
	}
	return 0;
}

//	���ꂪ�쐬���ꂽ���C���X���b�h
void CAppBase::ThreadProc(){		//	override from CThread
	//	�E�B���h�D�̍쐬��WorkThread�̍쐬��MessageLoop
	if (OnInit()) return ;
	CWindowOption opt;
	if (OnPreCreate(opt)) return ;			//	�E�B���h�D�������O�ɌĂяo�����
	if (m_oWindow.Create(opt)) return ;		//	�E�B���h�D�̍쐬
	if (OnCreate()) return ;				//	�E�B���h�D������Ă���Ăяo�����
	CAppManager::Add(this);					//	����CAppBase�̓o�^
	CAppManager::Hook(this);				//	���b�Z�[�W�t�b�N�J�n
	m_bMessage = true;						//	����ƃE�B���h�D�͊�������
	MainThread();							//	���[�U�[���ŗp�ӂ��ꂽ�A���C���֐�
	m_bMessage = false;						//	�E�B���h�D�͔j�󂳂��̂�...
	OnDestroy();							//	�I�����O
	//	Thread��app�𔻕ʂ��Ă���̂�Hook����Thread��Del���Ȃ��Ă͂Ȃ�Ȃ�
	//	::SendMessage(GetHWnd(),WM_CLOSE,0,0);	//	���b�Z�[�W�X���b�h���~������

	//	::SendMessage(GetHWnd(),WM_DESTROY,0,0);	//	���b�Z�[�W�X���b�h���~������
	//	������K�v�Ȃ񂩁H�H���ꂢ��Ă�ƁA�X���b�h���f�b�h���b�N����Ƃ�������D�D

	::PostMessage(GetHWnd(),WM_DESTROY,0,0);	//	���b�Z�[�W�X���b�h���~������
	MessagePump(true);	//	���œ��������b�Z�[�W�̏���
	//	�����ɂ��Ă�����͎��s���Ȃ��ƁA�E�B���h�D�̎c�[���c�邱�Ƃ�����

	//	WM_Destory���������Ȃ��Ă͂Ȃ�Ȃ��̂ł����Ńt�b�N����
	CAppManager::Unhook(this);				//	���b�Z�[�W�t�b�N�̏I��

	/*
		������CAppManager����؂藣���ƁAStopThread�����O�ɁA
		�I�����肪�s�Ȃ��A�X���b�h���c������\��������B
		����āACThread�̑S�ẴX���b�h���I�������̂��m�F���Ă����~���ׂ�
	*/

	CAppManager::Del(this);					//	����CAppBase�̍폜
	InnerStopThread();						//	�X���b�h���~
}

LRESULT CAppBase::MessagePump(bool bPeek/*=true*/){
	MSG msg;
	HWND hWnd = GetHWnd();
	if (bPeek) {
		while (::PeekMessage(&msg, NULL /* hWnd */,0,0,PM_REMOVE)) {
			//	�˂����AhWnd�ɂ��Ȃ��ƃ}���`�E�B���h�D�ɂ����Ƃ��A�}�Y�C�̂����A
			//�@�ǂ����ANULL�E�B���h�D�ɂ������ł��Ȃ����b�Z�[�W������悤��...
			//	���b�Z�[�W�����݂�����菈�����Â���
			if(::TranslateAccelerator(hWnd,m_hAccel,&msg)==0) {
				::TranslateMessage(&msg); 
				::DispatchMessage(&msg);
			}
		}
		//	���S�ɂ���Ȃ�APeekMessage���G���[�I�����Ă��Ȃ������ׂ�ׂ��Ȃ̂����m��Ȃ�
		//	GetMessage�ɑ΂��Ė߂�l��-1���ǂ������`�F�b�N���Ă���킯�ŁAPeekMessage�ɂ��Ă�
		//	����ɑΉ����鏈�����K�v���ƍl������
		return 0;
	} else {
		LRESULT lr = ::GetMessage(&msg,NULL,0,0);
		//	�˂����AGetHWnd�ɂ��Ȃ��ƃ}���`�E�B���h�D�ɂ����Ƃ��A�}�Y�C�̂����A
		//�@�ǂ����ANULL�E�B���h�D�ɂ������ł��Ȃ����b�Z�[�W������悤��...
		if (lr!=-1) {
			//	�G���[�R�[�h���Ԃ��Ă��Ă���ꍇ�A�����
			//	dispatch���Ă͂����Ȃ��B
			//	�������A���̏ꍇ�A�A�v�����I��������ׂ����낤���H
			if(::TranslateAccelerator(hWnd,m_hAccel,&msg)==0) {
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
		} else {
			// -1���Ԃ��Ă����Ȃ�A���̃X���b�h�j�������ق��������񂶂�ˁH
			InvalidateThread();
		}
		return lr;
	}
}

bool CAppBase::IsThreadValid()	{

	GetIntervalTimer()->CallBack();
	//	�t�b�N����Ă���^�C�}�ɃR�[���o�b�N��������

	if ( m_bIdle ){
		// Idle���[�h�ǉ�
		MessagePump(false); // get
	}else{
		MessagePump(true); // peek
	}
	if (m_bWaitIfMinimized) {
		//	WM_QUIT���ŏ��������������̂�҂�
		while (GetMyWindow()->IsMinimized()){
			if (MessagePump(false)!=0) break; // get
		}
	}

	//	���̃`�F�b�N�̂Ƃ��ɃX���b�h�̐��������`�F�b�N����
	return CThread::IsThreadValid();
}

void	CAppBase::InnerStopThread(){
	if (IsMainApp()) {
		CAppManager::StopAllThread();			//	�S�X���b�h���~������
	} else {
		//	�V���O���X���b�h�Ȃ�΂���͌Ăяo���܂ł��Ȃ��A��~���Ă���͂�����
//		StopThread();							//	���C���X���b�h�̏I���ҋ@����
	}
	CThread::InvalidateThread();
}

void	CAppBase::InvalidateThread(){	//	overriden from CThread
//	Idle���[�h���̗p���Ă���̂ŁA���b�Z�[�W�����Ȃ�����A
//	IsThreadValid���Ăяo����Ȃ��A���Ȃ킿�AWM_CLOSE�ɉ������ĕ��邱�Ƃ��o���Ȃ�
	if (m_bIdle){
		HWND hWnd = GetHWnd();
		if (hWnd!=NULL) {	//	���̃��b�Z�[�W�����Ĕj�󂵂Ă܂�
			//	::SendMessage(hWnd,WM_CLOSE,0,0);
			//	�����́A���b�Z�[�W�L���[�ɐς܂�邽�߁A
			//	������Ăяo�����X���b�h�̃��b�Z�[�W���[�v���Ă΂��
			//	�܂ŁA����̎��s���x�������B
			//	����āA
			::PostMessage(hWnd,WM_CLOSE,0,0);
			//	���ꂪ�����B
		}
	}
	CThread::InvalidateThread();	//	super�N���X�ɈϏ�����
}

IIntervalTimer*	CAppBase::GetIntervalTimer(){ return& m_vIntervalTimer; }

//////////////////////////////////////////////////////////////////////////////
//	���̃N���X�̃��b�Z�[�W�����p
//		�V���Ƀ��b�Z�[�W�������������Ƃ��́ACWinHook����h���������N���X������āA
//		�����Ńt�b�N�������Ă˂�B
LRESULT CAppBase::WndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam){

	switch (uMsg){

	///////////////////////////////////////////////////////////////////////////
/*
	//	GetMessage�Ŏ����̑�����E�B���h�D���b�Z�[�W�������o���Ă��Ȃ��̂ŁA
	//	�A�v�����؂�ւ�����Ƃ���WM_PAINT���E���Ȃ����Ƃ�����悤�����c(Windows2000���̃o�O?)
	case WM_ACTIVATEAPP : {
		if( wParam ) UpdateWindow(hWnd);
		break;
						  }
*/
	case WM_CLOSE: { // �E�C���h�E������ꂽ
		if (!m_bClose && OnPreClose()) return 1; // �����������Ƃɂ��ċA��
		CThread::InvalidateThread();
		//	���̃N���X��InvalidateThread�́A�A�C�h�����[�h����WM_CLOSE�𔭍s����̂ŉi�v���[�v�ɂȂ�
//		InnerStopThread();		//	�����Worker���~�����Ă���łȂ���
								//	WM_DESTORY��hWnd�������ɂȂ�ƁA���̓r�[�A
								//	���[�J�[�X���b�h�����邱�ƂɂȂ�
		return 1;
		//	invalidate���Ă����΁A���[�J�[�X���b�h�͎����I�ɋA�҂���
		//	�A�҂������[�J�[�X���b�h��WM_DESTROY�𔭍s���Ă��炤
				   }

	case WM_DESTROY:{

		if (IsMainApp()) {
			PostQuitMessage(0); //	���C���E�B���h�D�������Ȃ�I������
		}
		break;
					}
	}
	return 0;
}

void	CAppBase::Close(){
	m_bClose=true;
	::SendMessage(GetHWnd(),WM_CLOSE,0,0);
}

} // end of namespace AppFrame
} // end of namespace yaneuraoGameSDK3rd