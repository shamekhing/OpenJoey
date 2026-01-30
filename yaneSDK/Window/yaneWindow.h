// yaneWindow.h
//	window base class
//

#ifndef __yaneWindow_h__
#define __yaneWindow_h__


#include "yaneWinHook.h"
#include "../Thread/yaneCriticalSection.h"

namespace yaneuraoGameSDK3rd {
namespace Window {

class CWindowOption {
/**
	class CWindow �Ŏw�肷�邽�߂̃E�B���h�D�I�v�V����
*/
public:
	string	caption;	///	�L���v�V����
	string	classname;	///	�N���X��(caption�Ɠ����ł��ǂ�)
	LPCTSTR dialog;		///	�_�C�A���O�̏ꍇ�́A�����Ń��\�[�X�����w�肷���ok
		/**
			�����string�ɂ��Ă��Ȃ��̂́A
			MAKEINTRESOURCE�}�N�����A�P��int�^�������I��LPCTSTR�ɃL���X�g����
			::DialogBox�֐��ɓn���d�l�ɂȂ��Ă��邽�߁Astring�ɂ����
			MAKEINTERSOURCE�}�N�����g���Ȃ�����B

			IWindow�h���N���X��OnPreCreate���I�[�o�[���C�h���āA

			virtual LRESULT OnPreCreate(CWindowOption &opt){
				opt.dialog = MAKEINTRESOURCE(IDD_DIALOG1);	//	�_�C�A���O�Ȃ̂��I
				return 0;
			}
			�̂悤�ɂ���΁A�_�C�A���O��\���ł���
	
		*/

	int		size_x;		///	�������̃T�C�Y
	int		size_y;		///	�c�����̃T�C�Y
	LONG	style;		///	�E�B���h�D�X�^�C���̒ǉ��w��	

	bool	bCentering;	///	�E�B���h�D�͉�ʑS�̂ɑ΂��ăZ���^�����O���ĕ\����

	CWindowOption() {
		/**
			�f�B�t�H���g�ł���B�K�v������Ώ���������ׂ�
			caption = "���Ղ肿���";
			classname = "YANEAPPLICATION";
			style = WS_VISIBLE | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU;
			size_x = 800; size_y = 600;
			bCentering = true;
		*/
		caption = "���Ղ肿���";
		classname = "YANEAPPLICATION";
		style = WS_VISIBLE | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU;
		size_x = 800; size_y = 600;
		bCentering = true;
		dialog = NULL;
	}
};

class IWindow {
public:
	virtual LRESULT		Create(CWindowOption& opt,HWND lpParent=NULL)=0;
	virtual HWND		GetHWnd()const=0;
	virtual LRESULT		SetWindowPos(int x,int y)=0;
	virtual void		ChangeWindowStyle()=0;
	virtual LRESULT		Resize(int sx,int sy)=0;
	virtual void		SetSize(int sx,int sy)=0;
	virtual void		UseMouseLayer(bool bUse)=0;
	virtual void		ShowCursor(bool bShow)=0;
	virtual bool		IsShowCursor()=0;
	virtual CWindowOption* GetWindowOption()=0;
	volatile virtual bool		IsMinimized()=0;
	virtual CWinHookList* GetHookList()=0;
	virtual void		ClearAllHook()=0;
	
	virtual void		SetResized(bool bResized)=0;
	// DirectDraw�ɂ��t���X�N���[�����[�h�������Ƃ���
	//	�E�B���h�D������Ƀ��T�C�Y�����̂ŁA���̂Ƃ��ɒʒm���Ă����Ȃ���
	//	�E�B���h�D�N���X�̓�����ԂƈقȂ���̂ɂȂ��Ă��܂�

	virtual void ChangeScreen(bool bFullScr)=0;
	virtual bool IsDialog() const=0;

	virtual ~IWindow(){}
};

class CWindow : public IWinHook , public IWindow{
/**
	���𐶐�����N���X�ł��B���̃N���X�̃C���X�^���X����A
	����ɑΉ����܂��B�}���`�E�B���h�D�̃T�|�[�g�͂��Â��ł��B
	�i�ǂ����t���X�N���[���ɂȂ��DirectDraw��p���ĕ`�悵�Ă���ȏ�A
	�ǂ����悤�������Ƃ����b������c�j

	���@�`��̂��߂�

	class CFastDraw���g�����`����s�Ȃ��ꍇ�A
	��������Q�Ƃ��Ă��������BCFastDraw���g���Ă���Ƃ���
	��ʃT�C�Y��ύX����ꍇ�ACFastDraw::ChangeDisplayMode��
	�Ăяo���Ă��������B

*/
public:
	/// �������B�e�E�B���h�E�����݂���̂Ȃ�΍Ō�̃p�����[�^�Ŏw�肷�邱��
	LRESULT		Create(CWindowOption& opt,HWND lpParent=NULL);

	HWND		GetHWnd()const { return m_hWnd; }	///	HWND��Ԃ�

	LRESULT		SetWindowPos(int x,int y);			///	�E�B���h�D���ړ�

	void		ChangeWindowStyle();
	///	���݂̃X�N���[�����[�h�i�t���X�N���[�� or �E�B���h�D���[�h�j�p��
	///	WindowStyle��ύX����

	LRESULT		Resize(int sx,int sy);				///	���̃��T�C�Y

	//	�{����static�Ȋ֐��Ȃ̂���IWindow�o�R�ŃA�N�Z�X����K�v�����邽�߁A�����Ȃ�
	virtual void ChangeScreen(bool bFullScr){
		g_bFullScreen = bFullScr;
	}
	/**
		�t���X�N���[���̃E�B���h�D���[�h�̐؂�ւ������������ꍇ�A
		���̊֐����Ăяo�������Ƃ��ꂼ��̃E�B���h�D�Ɋւ���
		ChangeWindowStyle���Ăяo������
		�������A�t���X�N���[�����̃}���`�E�B���h�D��DirectDraw���g���֌W��A
		��T�|�[�g
	*/

	void		SetSize(int sx,int sy);
	/**
		���̃��T�C�Y�i�ݒ�݂̂Ŏ��ۂɕύX�͂��Ȃ��j
		�Ȃ�����Ȃ��̂��K�v�ɂȂ邩�Ƃ����ƁAResize��ChangeWindowStyle��
		���s����ƂQ��E�B���h�D�T�C�Y�̕ύX���s�Ȃ����ƂɂȂ��āA
		���̃��[�V�����������č��邩��B
		SetSize�@�ˁ@ChangeWindowStyle�Ȃ�Έ��S�B
	*/

	///		MouseLayer
	void	UseMouseLayer(bool bUse);
	///	�\�t�g�E�F�A�J�[�\���̂��߂ɃJ�[�\��������
	void	ShowCursor(bool bShow);
	///	�n�[�h�E�F�A�}�E�X�J�[�\���̕\���^��\��
	bool	IsShowCursor();
	///	�n�[�h�E�F�A�}�E�X�J�[�\���̕\���^��\�����擾

	///	�������Ă���̂̓_�C�A���O���H
	virtual bool IsDialog() const
	{	return const_cast<CWindow*>(this)->GetWindowOption()->dialog!=NULL; }

	///		Property
	DWORD		m_dwFillColor;		///	�w�i�F

	CWindowOption* GetWindowOption() { return &m_opt; }
	///	Window�I�v�V�����̎擾�B�E�B���h�D�����O�Ȃ�Ώ��������Ă��ǂ��B

	volatile bool	IsMinimized() { return m_bMinimized; }
	///	�ŏ�������Ă��邩�H

	///	���b�Z�[�W���t�b�N���邽�߂̃|�C���^���X�g
	///	�iclass IWinHook �̃x�N�^�j
	CWinHookList* GetHookList() { return &m_HookPtrList; }

	///	���b�Z�[�W���t�b�N���邽�߂̃|�C���^���X�g���N���A����
	void	ClearAllHook() { m_HookPtrList.Clear(); }

	CWindow();
	virtual ~CWindow();

	/**
		 ���j���[�̑��݂��`�F�b�N���A���j���[������̂Ȃ��
		�����t���O(m_bUseMenu)���X�V����B
		���I�Ƀ��j���[�����O�������Ƃ��ɁA���̃N���X��
		����𔽉f������̂Ɏg��
	*/
	void CheckMenu() {
		if(m_hWnd!=NULL&&::GetMenu(m_hWnd)!=NULL){
			m_bUseMenu = true;
		}else{
			m_bUseMenu = true;
		}
	}

	///		���̑�
	static void	GetScreenSize(int &x,int &y);
	///	���݂̉�ʑS�̂̃T�C�Y�̎擾

	static bool	IsFullScreen() { return g_bFullScreen; }
	///	�t���X�N���[�����H

	virtual void		SetResized(bool bResized) { m_bResized = true; }
	/**
		DirectDraw�ɂ��t���X�N���[�����[�h�������Ƃ���
		�E�B���h�D������Ƀ��T�C�Y�����̂ŁA���̂Ƃ��ɒʒm���Ă����Ȃ���
		�E�B���h�D�N���X�̓�����ԂƈقȂ���̂ɂȂ��Ă��܂�
		(CFastDraw����Ăяo���Ďg��)
	*/

protected:
	HWND		m_hWnd;				//	�E�B���h�D�n���h��
	bool		m_bFullScreen;		//	���݂ǂ���̃��[�h�ɍ��킹�đ�������Ă���̂��H
	CWindowOption	m_opt;			//	�E�B���h�D�I�v�V����
	bool		m_bCentering;		//	�E�B���h�D�͉�ʑS�̂ɑ΂��ăZ���^�����O���ĕ\����(default:true)

	static LRESULT CALLBACK gWndProc(HWND,UINT,WPARAM,LPARAM);
	static LRESULT CALLBACK gDlgProc(HWND,UINT,WPARAM,LPARAM);
	LRESULT Dispatch(HWND,UINT,WPARAM,LPARAM);	//	windows message dispatcher

	LRESULT		Initializer();	//	�N����A��x�����E�B���h�D�N���X��o�^����
	//	�t�b�N���Ă��邷�ׂẴC���X�^���X�ւ̃`�F�C��
	CWinHookList	m_HookPtrList;

	////////////////////////////////////////////////////////////////////////////
	//	�}�E�X�J�[�\����On/Off�́A�E�B���h�D�ɑ΂��鑮���Ȃ̂�
	//	�E�B���h�D�N���X���S���ׂ�
//	void	InnerShowCursor(bool bShow);
	bool	m_bShowCursor;			//	�}�E�X�J�[�\���̕\�����
	bool	m_bUseMouseLayer;		//	�\�t�g�E�F�A�}�E�X�J�[�\�����g�����H

	//	�E�B���h�D�T�C�Y��adjust
	void	InnerAdjustWindow(RECT&,CWindowOption&);

	//	���j���[�t���E�B���h�D���H
	bool	m_bUseMenu;

	//	ChangeWindowStyle��SetSize�̃t���O
	bool	m_bResized;

	//	Windows��{���^�C�v���쐬�����Ƃ��ɁA���b�Z�[�W�n���h����
	//	�t�b�N����̂ŁA���̊֐��|�C���^��ۑ����Ă����K�v������B
	WNDPROC	m_pWndProc;

	//	�ŏ�������Ă��邩�H
	volatile bool	m_bMinimized;

	bool IsWindowsClassName(const string& strClassName);
	//	Windows�ŗp�ӂ���Ă���Window�N���X�����ǂ����𒲂ׂ�

/*
	//	���j���[���m���ɑ��݂��邱�Ƃ����O�ɓ`���Ă�����
	//	�������ɐ��m�ȃE�B���h�D�T�C�Y�������Ȃ苁�܂�
	void		UseMenu(bool bUseMenu) { m_bUseMenu = bUseMenu; }
*/	//	�����I�ɔ��肷��悤�ɂ���

	static	bool g_bFullScreen;			//	�t���X�N���[�����[�h�Ȃ̂��H

	static CCriticalSection m_cs;
	static CCriticalSection* GetCriticalSection() { return &m_cs; }
	//	�����̏��static�����o�Q�ɃA�N�Z�X���邽�߂̃N���e�B�J���Z�N�V����

	// override from CWinHook
	virtual LRESULT WndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
};

class CDialogHelper {
/**
	�_�C�A���O�������Ƃ��̃w���p�N���X
*/
public:
	CDialogHelper(IWindow *pWin=NULL) : m_pWindow(pWin){ hook(pWin); }
	~CDialogHelper() { unhook(GetWindow()); }

	///	�R���X�g���N�^�œn�����тꂽ���̂��߂�setter / getter
	IWindow* GetWindow() { return m_pWindow; }
	void SetWindow(IWindow*pWin) { unhook(m_pWindow); m_pWindow = pWin; hook(pWin); }

	///	HWND�擾
	HWND	GetHWnd() { return GetWindow()->GetHWnd();}

	///	����R���g���[����HWnd�𓾂�
	HWND	GetHWnd(int nEditControlID) { return ::GetDlgItem(GetHWnd(),nEditControlID);}

	///	�G�f�B�b�g�R���g���[��etc..����A�����ɕ\������Ă���e�L�X�g���擾����
	///	(StaticText,Edit Control)
	string	GetText(int nEditControlID);

	///	�G�f�B�b�g�R���g���[��etc..�ɑ΂��āA�����Ƀe�L�X�g�������ݒ肷��
	///	(StaticText,Edit Control)
	LRESULT	SetText(int nEditControlID,const string& str);

	///	�{�^����Ԃ̎擾
	///	(CheckBox,RadioButton)
	/**
		-1:���̃{�^���͑��݂��Ȃ�
		0:�{�^���̃`�F�b�N�̓I�t�ɂȂ��Ă��܂��B 
		1:�{�^���̃`�F�b�N�̓I���ɂȂ��Ă��܂��B 
		2:�{�^���̓O���C�\���i�s�m��j�̏�Ԃł��B
		�{�^�����ABS_3STATE �X�^�C���܂��� BS_AUTO3STATE �X�^�C�������Ƃ��ɂ����K�p����܂��B 
	*/
	LRESULT	GetCheck(int nEditControlID);

	///	�{�^���Ƀ`�F�b�N������
	///	(CheckBox,RadioButton)
	/**
		nCheck:
		0:�{�^���̃`�F�b�N�̓I�t��
		1:�{�^���̃`�F�b�N�̓I����
		2:�{�^���̓O���C�\���i�s�m��j�̏�Ԃ�
	*/
	LRESULT SetCheck(int nEditControlID,int nCheck=1);

	/**
		�{�^�����͉���������񂪗���̂ŁA����̃��b�Z�[�W��
		hook����`�ŏ����擾����

		��)
		�{�^���Ȃ��

			uMsg = WM_COMMAND , wParam = nEditControl
		������A���̂悤��
		���O��uMsg��wParam���y�A�ɂ��ă��b�Z�[�Wdispatcher��
		�o�^���Ă����A���Ƃŉ�̓L���[�̃f�[�^�𒲂ׂ�Ɨǂ�

		�� class CMessagePooler���Q�Ƃ̂��ƁB
	*/
	CMessagePooler* GetPooler() { return &m_vPooler; }

	/**
		Hook�n�́A���ʎq���߂�̂ŁAGetPoolInfo�̈����Ƃ���
		�n���āA�v�[������Ă�������擾���Ďg��
	*/

	int	HookButtonOnClick(int nEditControl)
	{	return GetPooler()->SetPoolData(WM_COMMAND,nEditControl,true,0,false); }
	/**
			�{�^������hook
		��)
			int nID = dialog.HookButtonOnClick(IDC_BUTTON1);
			while (IsThreadValid()){
				if (dialog.GetPoolInfo(nID)->isPool()){
				//	�{�^��������Ƃ�I
					dialog.GetPoolInfo(nID)->reset();
					//	�����������̃N���A
				}
			}
	*/

	int	HookHScroolBar(int nEditControl)
	{	return GetPooler()->SetPoolData(WM_HSCROLL,0,false,(LPARAM)GetHWnd(nEditControl),true);}
	/**
		�����X�N���[���o�[��hook

		���b�Z�[�W��wParam�ɓ���
			nScrollCode = (int) LOWORD(wParam); // �X�N���[���R�[�h
			nPos = (short int) HIWORD(wParam);	// �X�N���[���{�b�N�X�i�܂݁j�̈ʒu
	*/

	int	HookVScroolBar(int nEditControl)
	{	return GetPooler()->SetPoolData(WM_VSCROLL,0,false,(LPARAM)GetHWnd(nEditControl),true);}
	/**
		�����X�N���[���o�[��hook

		���b�Z�[�W��wParam�ɓ���
			nScrollCode = (int) LOWORD(wParam); // �X�N���[���R�[�h
			nPos = (short int) HIWORD(wParam);	// �X�N���[���{�b�N�X�i�܂݁j�̈ʒu
	*/

	///	�X�N���[���o�[�ɂ́A����Ƀw���p�֐��K�v���낤�i��肩���j

	CMessagePooler::CInfo* GetPoolInfo(int nID)
	{	return GetPooler()->GetInfo(nID); }

	///	---- �R���{�{�b�N�X�ƃ��X�g�{�b�N�X�p�̏��ێ��N���X ---
	/**
		���l�F�R���{�{�b�N�X�͈ȉ��̂R��ނ���
			�P�D�W�� �@�@�@�@�@�@�@�@��ɐ���Ă��� �@�@�@�@�@�G�f�B�b�g�\
			�Q�D�h���b�v�_�E�� �@�@�@���{�b�N�X�Ő���� �@�@�@�G�f�B�b�g�\ 
			�R�D�h���b�v�_�E�����X�g �t�H�[�J�X������Ɛ���� �G�f�B�b�g�֎~�i���X�g������I���̂݁j

		�h���b�v�_�E���̍���(����)�̐ݒ�F
			���\�[�X�G�f�B�^�̃R���{�{�b�N�X�R���g���[���̊G�́���
			�N���b�N���Ęg���o���B���̘g���������Ɉ��������Đ����𒲐��B
			���ӁI�F�f�t�H���g�̂܂܂��ƍ������O�Ȃ̂Ń��X�g���ڂ������Ȃ��B
	*/
	struct CListBoxInfo {
	typedef vector<string> mylist;

	mylist* GetList() { return& m_list;}
	const mylist* GetConstList() const { return& m_list; }

	///	���X�g�{�b�N�X�ł��邩��ݒ�^�擾����(���X�g�{�b�N�X�łȂ��Ȃ�R���{�{�b�N�X�j
	bool	IsListBox() const { return m_bListBox; }
	void	SetListBox(bool bListBox) { m_bListBox = bListBox; }

	///	����������X�g�ɒǉ�
	/// nPos�͒ǉ����B�w�肵�����O�ɒǉ������BnPos==-1(default)�ł́A�ŏI�s�ɒǉ������
	void	AddString(const string& str,int nPos = -1){
		if (nPos==-1){
			GetList()->push_back(str);
			nPos = (int)GetList()->size()-1;
		} else {
			//	�}��������1�����炷
			GetList()->push_back("");
			int size = (int)GetList()->size();
			for(int i=size-1;i>=nPos+1;--i){
				(*GetList())[i] = (*GetList())[i-1];
			}
		}
		::SendMessage(GetHWnd(),IsListBox()?LB_INSERTSTRING:CB_INSERTSTRING
			,(WPARAM)nPos,reinterpret_cast<LPARAM>(str.c_str()));

	}

	///	���݂̑I���s��ݒ�^�擾���� (0 origin)
	LRESULT	SetCurSel(int nSel) {
		if ((int)GetList()->size() <= nSel) return -1; // �͈͊O
		::SendMessage(GetHWnd(),IsListBox()?LB_SETCURSEL:CB_SETCURSEL
			, (WPARAM)nSel, 0L);
		return 0;
	}
	///	�i�擾����ꍇ�́A�I���s���Ȃ����-1���Ԃ�)
	int GetCurSel() const {
		int nPos = ::SendMessage(GetHWnd(),IsListBox()?LB_GETCURSEL:CB_GETCURSEL
			,0L, 0L);
		return nPos;
	}

	///	���ݑI�����Ă���ꏊ�ɂ��镶������擾
	string	GetSelectedString() const {
		int nPos = GetCurSel();
		if ((int)GetConstList()->size()<=nPos) {
			return "";
		} else {
			return (*GetConstList())[nPos];
		}
	}

	///	�ݒ肳��Ă��镶�����S�N���A
	LRESULT	Clear(){
		HWND hWnd = GetHWnd();
		if (hWnd==NULL) return -1;
		::SendMessage(hWnd,
			IsListBox()?LB_RESETCONTENT:CB_RESETCONTENT,0,0L);
		GetList()->clear();
		return 0;
	}

	CListBoxInfo(HWND hWnd = NULL) : m_hWnd(hWnd),m_bListBox(true) {}
	
	///	�R���X�g���N�^�œn�����тꂽ�Ƃ��̂��߂ɃR���{�{�b�N�X(���X�g�{�b�N�X)
	///	��hWnd��n���Ă�邽�߂̃����o
	void	SetHWnd(HWND hWnd) { m_hWnd = hWnd; }
	HWND	GetHWnd() const { return m_hWnd; }

	protected:
	mylist m_list;		//	�ێ����Ă����񃊃X�g
	HWND   m_hWnd;
	bool	m_bListBox; // true:listbox false:combobox
	};

	/**
		���X�g�{�b�N�Xor�R���{�{�b�N�X���ACListBoxInfo�Ɗ֘A�Â���
		bListBox : true == ���X�g�{�b�N�X�ł���@false == �R���{�{�b�N�X�ł���
	*/
	void	Attach(CDialogHelper::CListBoxInfo& info,int nEditControl,bool bListBox=true)
	{
		info.SetHWnd(GetHWnd(nEditControl));
		info.SetListBox(bListBox);
	}
	///--------------------------------------------------------

	///	---- ���̂ق��A�~����������ǉ�����f�H�H ----

protected:
	IWindow* m_pWindow;	//	�eWindow�|�C���^��ݒ肵�Ă���
	CMessagePooler m_vPooler; // ���b�Z�[�WPooler

	void	hook(IWindow*pWin);
	void	unhook(IWindow*pWin);
};

class CMsgDlg {
/**
	���b�Z�[�W�_�C�A���O�o�͗p(��Ƀf�o�b�O�p)
*/
public:
	void	Out(const string& caption,const string& message);
};

} // end of namespace Window
} // end of namespace yaneuraoGameSDK3rd

#endif
