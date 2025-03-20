//
//		yaneuraoGameScript 2000 for yaneSDK2nd
//

#ifndef __CApp_h__
#define __CApp_h__

#include "../../yaneSDK/yaneSDK.h"
#include "yaneScriptCompiler.h"

//	メインアプリケーションクラス
class CApp : public CAppFrame {
public:
	//	フレームワーク内の各デバイスの外部からの取得
	CFastDraw*		GetDraw(void)		{ return& m_draw; }
	//	FastDraw Compiled Version

	CKey2*			GetKey(void)		{ return &m_key; }
	CDirectInput*	GetDirectInput(void){ return &m_directinput; }
	CJoyStick*		GetJoystick(void)	{ return &m_joystick; }
	CMouseEx*		GetMouse(void)		{ return &m_mouse; }
	CMouseLayer*	GetMouseLayer(void)	{ return &m_mouselayer; }
	CMIDIOutput*	GetMIDIOut(void)	{ return &m_midiout; }
	CMIDIInput*		GetMIDIIn(void)		{ return &m_midiin; }
	CCDDA*			GetCDDA(void)		{ return &m_cdda; }
	CFPSTimer*		GetFPSTimer(void)	{ return &m_fpstime; }
	
protected:
	CFastDraw	m_draw;			//	DirectDraw描画
	CFPSTimer	m_fpstime;		//	FPS制御
	CMIDIOutput	m_midiout;		//	MIDI出力
	CMIDIInput	m_midiin;		//	MIDI入力
	CKey2		m_key;			//	汎用キー入力
	CDirectInput m_directinput;	//	キーボードのIsPushKey及びIsPressKey用
	CJoyStick	m_joystick;
	CMouseLayer	m_mouselayer;	//	ソフトウェアマウスカーソル
	CFastPlane	m_mouseplane;	//	マウスカーソル用プレーン
	CMouseEx	m_mouse;		//	マウス入力
	CCDDA		m_cdda;			//	CDDA再生用

	void	Initialize();
	void	MainThread(void);
};

#endif
