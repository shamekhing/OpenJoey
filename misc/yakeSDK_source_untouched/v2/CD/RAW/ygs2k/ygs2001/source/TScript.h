//
//		yaneuraoGameScript 2000 for yaneSDK2nd
//

#ifndef __TScript_h__
#define __TScript_h__

#include "../../yaneSDK/yaneSDK.h"
#include "yaneScriptCompiler.h"
#include "CFineMapLayer.h"

class CApp;

//	どうせフレームワークが中途半端にしかマルチスレッドに対応していないので
//	これもシングルスレッド専用にコーディングしてしまう．．．
class TScript : public CScriptCompiler {
public:

	TScript(CApp*);
	virtual ~TScript();

//	新規追加分
static TUserFunc(Dbg);		//	LPSTR

//	デバッグ用
static TUserFunc(InnerLog);					//	(long)
static TUserFunc(InnerLogMes);				//	(LPSTR)

//	Wave関連
static CSound* m_sound;

static TUserFunc(LoadWave);					//	(LPSTR filename,long chno)
static TUserFunc(PlayWave);					//	(long chno)
static TUserFunc(StopWave);					//	(long chno)
static TUserFunc(PauseWave);				//	(long chno)
static TUserFunc(ReplayWave);				//	(long chno)
static TUserFunc(IsPlayWave);				//	(long chno) : bool
static TUserFunc(ReleaseWave);				//	(long chno)
static TUserFunc(ReleaseWaveAll);			//	(void)
static TUserFunc(SetLoopModeWave);			//	(long chno,bool)
static TUserFunc(SetVolumeWave);			//	(long chno,long volume)
static TUserFunc(SetVolumeWaveAll);			//	(long volume)
static TUserFunc(SetWaveFormat);			//	(int type)

//	MIDI再生関連
static TUserFunc(LoadMIDI);					//	(LPSTR filename)
static TUserFunc(PlayMIDI);					//	(void)
static TUserFunc(StopMIDI);					//	(void)
static TUserFunc(PauseMIDI);				//	(void)
static TUserFunc(ReplayMIDI);				//	(void)
static TUserFunc(ReleaseMIDI);				//	(void)
static TUserFunc(IsPlayMIDI);				//	(void) : bool
static TUserFunc(SetLoopModeMIDI);			//	(bool)

//	CD関連
static TUserFunc(OpenCD);					//	(void)
static TUserFunc(CloseCD);					//	(void)
static TUserFunc(StopCD);					//	(void)
static TUserFunc(PauseCD);					//	(void)
static TUserFunc(ReplayCD);					//	(void)
static TUserFunc(PlayCDFromStart);			//	(void)		曲頭から
static TUserFunc(PlayCD);					//	(int)		曲番号
static TUserFunc(PlayCDFromTo);				//	(int,int)	曲間セレクト
static TUserFunc(PlayCDPos);				//	(int,int)	pos間
static TUserFunc(IsPlayCD);					//	(void) : bool
static TUserFunc(SetLoopModeCD);			//	(bool)
static TUserFunc(GetSongMaxCD);				//	() : int
static TUserFunc(GetCurrentPosCD);			//	() : DWORD
static TUserFunc(EjectCD);					//	(bool bEject)

// FPS関連
static CTimer* m_gametime;

static TUserFunc(SetFPS);					//	(DWORD)
static TUserFunc(GetFPS);					//	(void) DWORD
static TUserFunc(GetRealFPS);				//	(void) DWORD
static TUserFunc(ResetTime);				//	(int)
static TUserFunc(GetTime);					//	(int) DWORD
static TUserFunc(PauseTime);				//	(int)
static TUserFunc(RestartTime);				//	(int)
static TUserFunc(PauseTimeAll);				//	bool
static TUserFunc(RestartTimeAll);			//	bool

// FPS表示関連
static CFPSLayer*	m_fpslayer;
static TUserFunc(FPSLayerOn);				//	(int x,int y);
static TUserFunc(FPSLayerOff);				//	(void);

// 文字表示関連
static CTextLayer*	m_textlayer;
static TUserFunc(TextLayerOn);				//	(int n,int x,int y);
static TUserFunc(TextLayerOff);				//	(int n);
static TUserFunc(TextMove);					//	(int n,int x,int y);
static TUserFunc(TextOut);					//	(int n,LPSTR);
static TUserFunc(TextSize);					//	(int n,int size);
static TUserFunc(TextHeight);				//	(int n,int height);
static TUserFunc(TextColor);				//	(int n,int r,int g,int b);
static TUserFunc(TextBackColor);			//	(int n,int r,int g,int b);
static TUserFunc(TextBackColorDisable);		//	(int n);
static TUserFunc(TextFont);					//	(int n); (LPSTR);
//static TUserFunc(TextBlend);				//	(int n);
static TUserFunc(TextBlt);					//	(int n);
static TUserFunc(TextGetSize);				//	(int n,int &x,int &y);

//	ファイル入出力
static TUserFunc(SaveFile);					//	(LPSTR,BYTE*,size_t) LRESULT
static TUserFunc(LoadFile);					//	(LPSTR,BYTE*,size_t) LRESULT

//	文字列操作関連
static LPSTR* m_alpString;					//	無駄かなぁ...
static TUserFunc(strcpy);					//	(LPSTR,LPSTR);
static TUserFunc(strcat);					//	(LPSTR,LPSTR);
static TUserFunc(strncpy);					//	(LPSTR,LPSTR,size_t n);
static TUserFunc(sprintf);					//	(LPSTR,LPSTR,vpara...);

//	その他
static TUserFunc(Rand);						//	(long)
static TUserFunc(GetCurrentDirectory);		//	(LPSTR)
static TUserFunc(SetCurrentDirectory);		//	(LPSTR)

//	user変数
static TUserFunc(LoadGameFlag);				//	(LPSTR) LRESULT
static TUserFunc(SaveGameFlag);				//	(LPSTR) LRESULT
static TUserFunc(LoadGameFlag2);			//	(LPSTR) LRESULT
static TUserFunc(SaveGameFlag2);			//	(LPSTR) LRESULT
static TUserFunc(ResetGameFlag);			//	()
static TUserFunc(ResetGameFlag2);			//	()

static LONG* m_GameFlag;
static LONG* m_GameFlag2;

//	Bitmap関連
static CFastPlane* m_plane;

static TUserFunc(GetBpp);					//	(void) : int
	// この２つはLoadBitmapだとWinAPIとかぶるので名前を変更している
static TUserFunc(LoadBitmapFile);			//	(LPSTR,int,bool)
static TUserFunc(LoadBitmapFileW);			//	(LPSTR,LPSTR,int,bool)
static TUserFunc(SaveBitmapFile);			//	(LPSTR filename,int planeNo,int x,int y,int sx,int sy)
static TUserFunc(ReleaseBitmap);			//	(int)
static TUserFunc(GetPlaneSize);				//	(int,int&,int&)
static TUserFunc(SetColorKeyRGB);			//	(int plane,int r,int g,int b)
static TUserFunc(SetColorKeyPos);			//	(int plane,int x,int y)
static TUserFunc(ClearSecondary);			//	(void)
static TUserFunc(ClearSecondaryRect);		//	(int x1,int y1,int x2,int y2)
static TUserFunc(SetFillColorRGB);			//	(int r,int g,int b)
//static TUserFunc(SystemMemoryPlane);		//	(int planeNo,bool bEnable)

static TUserFunc(Blt);						//	(int plane,int x,int y)
static TUserFunc(BltRect);					//	(int plane,int x,int y,int sx,int sy,int hx,int hy)
static TUserFunc(BltFast);					//	(int plane,int x,int y)
static TUserFunc(BltFastRect);				//	(int plane,int x,int y,int sx,int sy,int hx,int hy)
//static TUserFunc(BlendBlt);					//	(int plane,int x,int y,int ar,int ag,int ab,int br,int bg,int bb)
//static TUserFunc(BlendBltRect);				//	(int plane,int x,int y,int sx,int sy,int hx,int hy,int ar,int ag,int ab,int br,int bg,int bb)
//static TUserFunc(BlendBltFast);				//	(int plane,int x,int y,int ar,int ag,int ab,int br,int bg,int bb)
//static TUserFunc(BlendBltFastRect);			//	(int plane,int x,int y,int sx,int sy,int hx,int hy,int ar,int ag,int ab,int br,int bg,int bb)
static TUserFunc(ClipBlt);					//	(int plane,int x,int y,int cx,int cy,int chx,int chy)
static TUserFunc(ClipBltRect);				//	(int plane,int x,int y,int sx,int sy,int hx,int hy,int cx,int cy,int chx,int chy)
static TUserFunc(ClipBltFast);				//	(int plane,int x,int y,int cx,int cy,int chx,int chy)
static TUserFunc(ClipBltFastRect);			//	(int plane,int x,int y,int sx,int sy,int hx,int hy,int cx,int cy,int chx,int chy)
//static TUserFunc(ClipBlendBlt);				//	(int plane,int x,int y,int cx,int cy,int chx,int chy,int ar,int ag,int ab,int br,int bg,int bb)
//static TUserFunc(ClipBlendBltRect);			//	(int plane,int x,int y,int sx,int sy,int hx,int hy,int cx,int cy,int chx,int chy,int ar,int ag,int ab,int br,int bg,int bb)
//static TUserFunc(ClipBlendBltFast);			//	(int plane,int x,int y,int cx,int cy,int chx,int chy,int ar,int ag,int ab,int br,int bg,int bb)
//static TUserFunc(ClipBlendBltFastRect);		//	(int plane,int x,int y,int sx,int sy,int hx,int hy,int cx,int cy,int chx,int chy,int ar,int ag,int ab,int br,int bg,int bb)

static TUserFunc(SetBltMode);				//	(int nMode,nAlpha);

//　プレーンに転送する関数
static TUserFunc(BltSecondaryTo);			//  (int nPlaneNo)

//	拡大縮小機能付
static TUserFunc(BltR);						//	(int plane,int x,int y,double r)
static TUserFunc(BltRectR);					//	(int plane,int x,int y,int sx,int sy,int hx,int hy,double r)
static TUserFunc(BltFastR);					//	(int plane,int x,int y,double r)
static TUserFunc(BltFastRectR);				//	(int plane,int x,int y,int sx,int sy,int hx,int hy,double r)
//static TUserFunc(BlendBltR);				//	(int plane,int x,int y,int ar,int ag,int ab,int br,int bg,int bb,double r)
//static TUserFunc(BlendBltRectR);			//	(int plane,int x,int y,int sx,int sy,int hx,int hy,int ar,int ag,int ab,int br,int bg,int bb,double r)
//static TUserFunc(BlendBltFastR);			//	(int plane,int x,int y,int ar,int ag,int ab,int br,int bg,int bb,double r)
//static TUserFunc(BlendBltFastRectR);		//	(int plane,int x,int y,int sx,int sy,int hx,int hy,int ar,int ag,int ab,int br,int bg,int bb,double r)
static TUserFunc(ClipBltR);					//	(int plane,int x,int y,int cx,int cy,int chx,int chy,double r)
static TUserFunc(ClipBltRectR);				//	(int plane,int x,int y,int sx,int sy,int hx,int hy,int cx,int cy,int chx,int chy,double r)
static TUserFunc(ClipBltFastR);				//	(int plane,int x,int y,int cx,int cy,int chx,int chy,double r)
static TUserFunc(ClipBltFastRectR);			//	(int plane,int x,int y,int sx,int sy,int hx,int hy,int cx,int cy,int chx,int chy,double r)
//static TUserFunc(ClipBlendBltR);			//	(int plane,int x,int y,int cx,int cy,int chx,int chy,int ar,int ag,int ab,int br,int bg,int bb,double r)
//static TUserFunc(ClipBlendBltRectR);		//	(int plane,int x,int y,int sx,int sy,int hx,int hy,int cx,int cy,int chx,int chy,int ar,int ag,int ab,int br,int bg,int bb,double r)
//static TUserFunc(ClipBlendBltFastR);		//	(int plane,int x,int y,int cx,int cy,int chx,int chy,int ar,int ag,int ab,int br,int bg,int bb,double r)
//static TUserFunc(ClipBlendBltFastRectR);	//	(int plane,int x,int y,int sx,int sy,int hx,int hy,int cx,int cy,int chx,int chy,int ar,int ag,int ab,int br,int bg,int bb,double r)

//	キー入力関連
static TUserFunc(KeyInput);					//	()
static TUserFunc(IsPushKey);				//	() bool
static TUserFunc(IsPressKey);				//	() bool
static TUserFunc(IsPressUpKey);				//	() bool
static TUserFunc(IsPressDownKey);			//	() bool
static TUserFunc(IsPressLeftKey);			//	() bool
static TUserFunc(IsPressRightKey);			//	() bool
static TUserFunc(IsPressReturnKey);			//	() bool
static TUserFunc(IsPressSpaceKey);			//	() bool
static TUserFunc(IsPressEscKey);			//	() bool
static TUserFunc(IsPushUpKey);				//	() bool
static TUserFunc(IsPushDownKey);			//	() bool
static TUserFunc(IsPushLeftKey);			//	() bool
static TUserFunc(IsPushRightKey);			//	() bool
static TUserFunc(IsPushReturnKey);			//	() bool
static TUserFunc(IsPushSpaceKey);			//	() bool
static TUserFunc(IsPushEscKey);				//	() bool

//　ジョイスティックによる入力関連
static TUserFunc(SetJoyButtonMax);
static TUserFunc(IsPushJoyKey);
static TUserFunc(IsPressJoyKey);
static TUserFunc(SelectJoyStick);

//	MIDI入力
static TUserFunc(IsPressMIDIKey);			//	(int n) : bool
static TUserFunc(IsPushMIDIKey);			//	(int n) : bool
static TUserFunc(GetVelocityMIDIKey);		//	(int buttonmax) : byte


//	効果
static TUserFunc(SetSecondaryOffset);		//	(int ox,int oy);
static TUserFunc(SetBrightness);			//	(int);
static TUserFunc(CreateSurface);			//	(int plane,int sx,int sy);
static TUserFunc(SwapToSecondary);			//	(int);

//	MOUSE入力
static TUserFunc(GetMouseInfo);				//	(int *x,int *y,int *b)
static TUserFunc(SetMouseInfo);				//	(int x,int y)
static TUserFunc(EnableMouseCursor);		//	(bool)
static TUserFunc(MouseInput);				//	()

//	ディスプレイモードの変更
static TUserFunc(SetDisplayMode);			//	(int d1,int d2)
static TUserFunc(ChangeDisplayMode);		//	(int n)
static int m_nDisplayMode1;
static int m_nDisplayMode2;

//	シナリオの描画関係
static TUserFunc(ScenarioLayerOn);			//	()
static TUserFunc(ScenarioLayerOff);			//	()
static TUserFunc(ScenarioLoad);				//	(LPSTR szFileName)
static TUserFunc(ScenarioOnDraw);			//	()
static TUserFunc(ScenarioEnableSkip);		//	(bool bSkip)

//	マップ描画関係
static TUserFunc(MapLayerOn);				//	()
static TUserFunc(MapLayerOff);				//	()
static TUserFunc(MapLoad);					//	(LPSTR szFileName)
static TUserFunc(MapOnDraw);				//	(int x,int y,int nLayerType)
											//	nLayerType 0==all 1:クリッピング 2:下 3:中 4:上 5:側面
static TUserFunc(MapGetSize);				//	(int &sx,int &sy)
static TUserFunc(MapSetView);				//	(int left,int top,int right,int bottom)
	
//////////////////////////////////////////////////////////////////////////////
protected:
static CApp*	m_lpApp;
static CApp*	GetApp() { return m_lpApp; }

//	各種定数は、途中変更可能にした
static	int		m_nSoundMax;					//	総Soundチャンネル数
static	int		m_nPlaneMax;					//	総プレーン数
static	int		m_nGameTimeMax;					//	総GameTime数
static	int		m_nTextLayerMax;				//	総テキストレイヤー数
static	int		m_nStringMax;					//	string配列の数
static	int		m_nStringLength;				//	string配列の大きさ
static	bool	m_bUSE_XZ_FOR_BUTTON;			//	XとZキーをスペース、リターンに割り当てる
static	int		m_nGameFlagMax;					//	総GameFlag数
static	int		m_nGameFlagMax2;				//	総GameFlag2数

//	BltMode
static	int		m_nBltMode;
	//	0:通常(BltNatural)
	//	1:ブレンド	2:抜き色ブレンド
	//	3:加色		4:抜き色付き加色
	//	5:減色		6:抜き色付き減色
static	int		m_nBltAlpha;

static	CScenarioDraw* m_lpScenario;
static	CFineMapLayer* m_lpMapLayer;
};

#endif
