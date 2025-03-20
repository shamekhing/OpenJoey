// JoyStick Input Wrapper

#include "stdafx.h"
#include "yaneJoyStick.h"
#ifdef USE_DIRECTINPUT_JOYSTICK
#include "yaneJoyStickDI.h"
#endif

#include "yaneDirectInput.h"
#include "yaneAppInitializer.h"
#include "yaneDebugWindow.h"

IJoyStick::IJoyStick(void){
	m_nKeyBufNo=0;	// 裏と表をflipして、差分をとるのに利用
	ZERO(m_byKeyBuffer);
}

IJoyStick::~IJoyStick(){
}

//////////////////////////////////////////////////////////////////////////////

inline bool IJoyStick::IsKeyPress(int key) const {
	return	(m_byKeyBuffer[m_nKeyBufNo][key]) !=0;
};

inline bool IJoyStick::IsKeyPush(int key) const {
	// 押された瞬間にtrueにする場合
	if (!(m_byKeyBuffer[  m_nKeyBufNo][key])) return false;
	if (  m_byKeyBuffer[1-m_nKeyBufNo][key] ) return false;
	return true;
}

inline BYTE*	IJoyStick::GetKeyData(void) const {
	return	(BYTE*)&m_byKeyBuffer[m_nKeyBufNo];
}


CJoyStick::CJoyStick(void){
	m_nButtonMax = 2;
#ifdef USE_WIN32_JOYSTICK
	m_joystick_selector = jsJOYSTICK1OR2;
#else 
	#ifdef USE_DIRECTINPUT_JOYSTICK
	m_joystick_selector = jsDIRECT_JOYSTICK1;
	#else
	m_joystick_selector = jsNODEVICE;
	#endif
#endif
}

CJoyStick::~CJoyStick(){
	Terminate();
}

#ifdef USE_DIRECTINPUT_JOYSTICK
//	DirectInput経由でJoyStickの数を問い合わせる
int		CJoyStick::GetDirectInputJoyStickNum(void){
	CDirectInputJoyStick::AddRef();
	int n = CDirectInputJoyStick::GetDirectInputJoyStickNum();
	CDirectInputJoyStick::DelRef();
	//	このあと、JoyStickをひっこ抜かれたら、たまらんですけど＾＾；
	//	それが怖ければ、自前でAddRefして、自前でDefRefすべし。
	return n;
}
#endif

void CJoyStick::Reset(void){
	// 別にしなくても困らない…
	if ( m_lpIJoyStick != NULL ){
		m_lpIJoyStick->Reset();
	}
}

LRESULT CJoyStick::SetButtonMax(int max){
	m_nButtonMax = max;
	// 非NULLなら今渡す
	if ( m_lpIJoyStick != NULL ){
		return m_lpIJoyStick->SetButtonMax(max);
	}
	return 0;
}

LRESULT CJoyStick::SelectDevice(JoySelector j){
//	int i = j;
	m_joystick_selector = j;
	Terminate();
//	m_lpIJoyStick.Delete();
/*#ifdef USE_WIN32_JOYSTICK
	if ( jsJOYSTICK1 <= i && i <= jsJOYSTICK1OR2 ){
		m_lpIJoyStick.Add( (IJoyStick*) new CWIN32JoyStick(j,m_nButtonMax) );
		return 0;
	}
#endif
#ifdef USE_DIRECTINPUT_JOYSTICK
	if( jsDIRECT_JOYSTICK1 <= i && i <= jsDIRECT_JOYSTICK16 ){
		m_lpIJoyStick.Add( (IJoyStick*) new CDIJoyStick(j,m_nButtonMax) );
		return 0;
	}
#endif
	*/
	return 0;
}

LRESULT CJoyStick::GetKeyState(void){
	HRESULT hr;
	// 初期化していない
	if ( m_lpIJoyStick == NULL ){
		hr = Initialize();
		if ( hr != 0 ){
			Err.Out("JoyStick Number %d",m_joystick_selector+1);
			if ( hr == 1 )	Err.Out("CJoyStick::GetKeyState IJoyStickの生成に失敗");
			if ( hr == 2 )	Err.Out("CJoyStick::GetKeyState IJoyStickの初期化に失敗");
			return -1;
		}
	}
	return m_lpIJoyStick->GetKeyState();
}

inline bool CJoyStick::IsKeyPress(int key) const{
	if ( m_lpIJoyStick == NULL ) return false;
	return m_lpIJoyStick->IsKeyPress(key);
}
inline bool CJoyStick::IsKeyPush(int key) const{
	if ( m_lpIJoyStick == NULL ) return false;
	return m_lpIJoyStick->IsKeyPush(key);
}
inline BYTE* CJoyStick::GetKeyData(void) const{
	if ( m_lpIJoyStick == NULL ) return NULL;
	return m_lpIJoyStick->GetKeyData();
}

LRESULT CJoyStick::Terminate(void){
	m_lpIJoyStick.Delete();
	return 0;
}

LRESULT CJoyStick::Initialize(void){
	int i = m_joystick_selector;
	if ( i == jsNODEVICE ) return 2;
#ifdef USE_WIN32_JOYSTICK
	if ( jsJOYSTICK1 <= i && i <= jsJOYSTICK1OR2 ){
		m_lpIJoyStick.Add( (IJoyStick*) new CWIN32JoyStick(m_joystick_selector,m_nButtonMax) );
		if ( m_lpIJoyStick->IsInit() ) return 0;
		else return 2;
	}
#endif
#ifdef USE_DIRECTINPUT_JOYSTICK
	if( jsDIRECT_JOYSTICK1 <= i && i <= jsDIRECT_JOYSTICK16 ){
		m_lpIJoyStick.Add( (IJoyStick*) new CDIJoyStick(m_joystick_selector,m_nButtonMax) );
		if ( m_lpIJoyStick->IsInit() ) return 0;
		else return 2;
	}
#endif
	return 1;
}

bool	CJoyStick::IsInit(void){
	if ( m_lpIJoyStick == NULL ) return false;
	return m_lpIJoyStick->IsInit();
}


#ifdef USE_WIN32_JOYSTICK

CWIN32JoyStick::CWIN32JoyStick(void){
	m_joystick_selector = jsJOYSTICK1OR2;	//	１か２の見つかったほう
	m_nButtonMax	=	2;					//	２ボタンがディフォルト
	Initialize();
}

CWIN32JoyStick::CWIN32JoyStick(JoySelector j,int nButtonMax){
	m_nButtonMax = nButtonMax;
	m_joystick_selector = j;
	Initialize();
}

CWIN32JoyStick::~CWIN32JoyStick(){
	Terminate();
}

void	CWIN32JoyStick::Reset(void){
	ZERO(m_byKeyBuffer);	//	途中でのデバイス切り替えに対応するため一応、クリアする。
}

LRESULT CWIN32JoyStick::SetButtonMax(int max) {
	// ボタンの数を変えるくらいならInitializeはいらない
	m_nButtonMax = max;
	Reset();
	return 0;
}

LRESULT CWIN32JoyStick::SelectDevice(JoySelector j) {
	m_joystick_selector = j;
	Terminate();
	return Initialize();
}

LRESULT CWIN32JoyStick::Terminate(void){ // 終了処理
	return 0; // 正常終了
}

LRESULT CWIN32JoyStick::Initialize(void){  // 初期化処理
	m_bSuccessInit	=	false;
	if ( m_joystick_selector == jsNODEVICE ) return 2;
	int n = joyGetNumDevs();
	if ( n == 0) return 1;	//	ジョイスティックあらへんやん...

	JOYCAPS joyCaps;
	switch ( m_joystick_selector ){
	case jsJOYSTICK1:
		{
			if ( joyGetDevCaps(JOYSTICKID1,&joyCaps,sizeof(JOYCAPS)) == JOYERR_NOERROR) {
				m_uID = JOYSTICKID1;
			} else {
				m_joystick_selector = jsNODEVICE;
				return 2;	// failed...
			}
			break;
		}
	case jsJOYSTICK2:
		{
			if ( joyGetDevCaps(JOYSTICKID2,&joyCaps,sizeof(JOYCAPS)) == JOYERR_NOERROR) {
				m_uID = JOYSTICKID2;
			} else {
				m_joystick_selector = jsNODEVICE;
				return 2;	// failed...
			}
			break;
		}
	case jsJOYSTICK1OR2:
		{
			if ( joyGetDevCaps(JOYSTICKID1,&joyCaps,sizeof(JOYCAPS)) == JOYERR_NOERROR) {
				m_uID = JOYSTICKID1;
			} else if ( joyGetDevCaps(JOYSTICKID2,&joyCaps,sizeof(JOYCAPS)) == JOYERR_NOERROR) {
				m_uID = JOYSTICKID2;
			} else {
				m_joystick_selector = jsNODEVICE;
				return 2;	// failed...
			}
			break;
		}
	default: 
		m_joystick_selector = jsNODEVICE;
		return 2;
		break;
	}

	//	センターはニュートラルだろうから、１：２に内分するところを閾値とする
	m_uX1	=	(joyCaps.wXmax	 +	joyCaps.wXmin*2)/3;
	m_uX2	=	(joyCaps.wXmax*2 +	joyCaps.wXmin  )/3;
	m_uY1	=	(joyCaps.wYmax	 +	joyCaps.wYmin*2)/3;
	m_uY2	=	(joyCaps.wYmax*2 +	joyCaps.wYmin  )/3;

	JOYINFO joyinfo;	//	実際に読み込めるのか確認しておく必要あり
	if (joyGetPos(m_uID,&joyinfo)!= JOYERR_NOERROR) return 2;

	m_bSuccessInit	=	true;
	ZERO(m_byKeyBuffer);	//	途中でのデバイス切り替えに対応するため一応、クリアする。

	return 0;
}


// ---------------------------------------------------------------------------

LRESULT CWIN32JoyStick::GetKeyState(void){
	
	if (!m_bSuccessInit) return 1;	//	きみぃ、そもそも初期化失敗しとるがに..

	//	バッファのflip!
	FlipKeyBuffer(m_nKeyBufNo);

	//	ボタン４つ以下のときは、joyGetPosで十分...
	if (m_nButtonMax<=4) {
		UINT x,y,b;
		//	本来ならば、コールバック関数にしてメッセージハンドラを用意すべき...
		JOYINFO joyinfo;
		if (joyGetPos(m_uID,&joyinfo)!= JOYERR_NOERROR) return 2;
		x = WORD(joyinfo.wXpos);		//	X座標
		y = WORD(joyinfo.wYpos);		//	Y座標
		b = WORD(joyinfo.wButtons);		//	ボタン情報
		m_byKeyBuffer[m_nKeyBufNo][0] = y<m_uY1?1:0;	//	上
		m_byKeyBuffer[m_nKeyBufNo][1] = y>m_uY2?1:0;	//	下
		m_byKeyBuffer[m_nKeyBufNo][2] = x<m_uX1?1:0;	//	左
		m_byKeyBuffer[m_nKeyBufNo][3] = x>m_uX2?1:0;	//	右
		//	４番～３５（最大）番までが、各ボタン情報である！
		//	４番がボタン１，５番がボタン２．．以下３５番まで
		for(int i=0;i<m_nButtonMax;i++){
			m_byKeyBuffer[m_nKeyBufNo][i+4] = b & 1;
			b >>= 1;
		}
	} else {
		DWORD x,y,b;
		JOYINFOEX joyinfo;
		joyinfo.dwSize	= sizeof(JOYINFOEX);
		joyinfo.dwFlags	= JOY_RETURNALL;
		if (joyGetPosEx(m_uID,&joyinfo)!= JOYERR_NOERROR) return 2;
		x = WORD(joyinfo.dwXpos);		//	X座標
		y = WORD(joyinfo.dwYpos);		//	Y座標
		b =	 joyinfo.dwButtons;			//	ボタン情報
		m_byKeyBuffer[m_nKeyBufNo][0] = y<m_uY1?1:0;	//	上
		m_byKeyBuffer[m_nKeyBufNo][1] = y>m_uY2?1:0;	//	下
		m_byKeyBuffer[m_nKeyBufNo][2] = x<m_uX1?1:0;	//	左
		m_byKeyBuffer[m_nKeyBufNo][3] = x>m_uX2?1:0;	//	右
		for(int i=0;i<m_nButtonMax;i++){
			m_byKeyBuffer[m_nKeyBufNo][i+4] = b & 1;
			b >>= 1;
		}
	}
	return 0;
}
#endif
