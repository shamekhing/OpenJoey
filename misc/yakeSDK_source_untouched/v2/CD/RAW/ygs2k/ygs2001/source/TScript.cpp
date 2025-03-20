#include "stdafx.h"

#include "TScript.h"
#include "CApp.h"

//	static members..
CApp* TScript::m_lpApp = NULL;

TScript::TScript(CApp*lpApp){
	//	アプリクラスに通底する
	m_lpApp = lpApp;

	m_nSoundMax		= 512;			//	総Soundチャンネル数
	m_nPlaneMax		= 512;			//	総プレーン数
	m_nGameTimeMax	= 16;			//	総GameTime数
	m_nTextLayerMax = 64;			//	総テキストレイヤー数
	m_nStringMax	= 16;			//	string配列の数
	m_nStringLength = 512;			//	string配列の大きさ
	m_bUSE_XZ_FOR_BUTTON = false;	//	XとZキーをスペース、リターンに割り当てる
	m_nGameFlagMax	= 1024;			//	総GameFlag数
	m_nGameFlagMax2 = 1024;			//	総GameFlag2数

	m_nBltMode	=	0;
	m_nBltAlpha	=	0;

	//	各種インスタンス
	m_sound		=	new CSound		[m_nSoundMax];
	m_gametime	=	new CTimer		[m_nGameTimeMax];

	m_fpslayer	=	NULL;
	m_textlayer	=	new CTextLayer	[m_nTextLayerMax];

	{	// ディフォルトでTextLayerはfalse
	  for(int i=0;i<m_nTextLayerMax;i++){
		m_textlayer[i].Enable(false);
	  }
	}

	m_alpString = new LPSTR [m_nStringMax];
	{	//	ここで動的に確保
	  for(int i=0;i<m_nStringMax;i++) {
		m_alpString[i] = new CHAR [m_nStringLength];
	  }
	}

	m_plane		=	new CFastPlane		[m_nPlaneMax];

	//	ユーザー関数の定義
	
	//	新規追加分
	RegistUserFunction("Dbg",Dbg);

	//	デバッグ用
	RegistUserFunction("InnerLog",InnerLog);
	RegistUserFunction("InnerLogMes",InnerLogMes);

	//	Wave
	RegistUserFunction("LoadWave",LoadWave);
	RegistUserFunction("PlayWave",PlayWave);
	RegistUserFunction("StopWave",StopWave);
	RegistUserFunction("PauseWave",PauseWave);
	RegistUserFunction("ReplayWave",ReplayWave);
	RegistUserFunction("IsPlayWave",IsPlayWave);
	RegistUserFunction("ReleaseWave",ReleaseWave);
	RegistUserFunction("ReleaseWaveAll",ReleaseWaveAll);
	RegistUserFunction("SetLoopModeWave",SetLoopModeWave);
	RegistUserFunction("SetVolumeWave",SetVolumeWave);
	RegistUserFunction("SetVolumeWaveAll",SetVolumeWaveAll);
	RegistUserFunction("SetWaveFormat",SetWaveFormat);

	//	MIDI関連
	RegistUserFunction("LoadMIDI",LoadMIDI);
	RegistUserFunction("PlayMIDI",PlayMIDI);
	RegistUserFunction("StopMIDI",StopMIDI);
	RegistUserFunction("PauseMIDI",PauseMIDI);
	RegistUserFunction("ReplayMIDI",ReplayMIDI);
	RegistUserFunction("ReleaseMIDI",ReleaseMIDI);
	RegistUserFunction("IsPlayMIDI",IsPlayMIDI);
	RegistUserFunction("SetLoopModeMIDI",SetLoopModeMIDI);

	//	CD関連
	RegistUserFunction("OpenCD",OpenCD);
	RegistUserFunction("CloseCD",CloseCD);
	RegistUserFunction("StopCD",StopCD);
	RegistUserFunction("PauseCD",PauseCD);
	RegistUserFunction("ReplayCD",ReplayCD);
	RegistUserFunction("PlayCDFromStart",PlayCDFromStart);
	RegistUserFunction("PlayCD",PlayCD);
	RegistUserFunction("PlayCDFromTo",PlayCDFromTo);
	RegistUserFunction("PlayCDPos",PlayCDPos);
	RegistUserFunction("IsPlayCD",IsPlayCD);
	RegistUserFunction("SetLoopModeCD",SetLoopModeCD);
	RegistUserFunction("GetSongMaxCD",GetSongMaxCD);
	RegistUserFunction("GetCurrentPosCD",GetCurrentPosCD);
	RegistUserFunction("EjectCD",EjectCD);
	RegistUserVariable("SongLengthCD",*(LONG*)(&GetApp()->GetCDDA()->m_dwSongLength));
	RegistUserVariable("SongStartCD",*(LONG*)(&GetApp()->GetCDDA()->m_dwSongStart));

	//	キー入力関数
	RegistUserFunction("KeyInput",KeyInput);
	RegistUserFunction("IsPushKey",IsPushKey);
	RegistUserFunction("IsPressKey",IsPressKey);
	RegistUserFunction("IsPressUpKey",IsPressUpKey);
	RegistUserFunction("IsPressDownKey",IsPressDownKey);
	RegistUserFunction("IsPressLeftKey",IsPressLeftKey);
	RegistUserFunction("IsPressRightKey",IsPressRightKey);
	RegistUserFunction("IsPressReturnKey",IsPressReturnKey);
	RegistUserFunction("IsPressSpaceKey",IsPressSpaceKey);
	RegistUserFunction("IsPressEscKey",IsPressEscKey);
	RegistUserFunction("IsPushUpKey",IsPushUpKey);
	RegistUserFunction("IsPushDownKey",IsPushDownKey);
	RegistUserFunction("IsPushLeftKey",IsPushLeftKey);
	RegistUserFunction("IsPushRightKey",IsPushRightKey);
	RegistUserFunction("IsPushReturnKey",IsPushReturnKey);
	RegistUserFunction("IsPushSpaceKey",IsPushSpaceKey);
	RegistUserFunction("IsPushEscKey",IsPushEscKey);

	//	ジョイスティック関連
	RegistUserFunction("SetJoyButtonMax",SetJoyButtonMax);
	RegistUserFunction("IsPushJoyKey",IsPushJoyKey);
	RegistUserFunction("IsPressJoyKey",IsPressJoyKey);
	RegistUserFunction("SelectJoyStick",SelectJoyStick);

	//	MIDI出力関連
	RegistUserFunction("IsPushMIDIKey",IsPushMIDIKey);	//	汎用MIDI
	RegistUserFunction("IsPressMIDIKey",IsPressMIDIKey);
	RegistUserFunction("GetVelocityMIDIKey",GetVelocityMIDIKey);

	//	Bitmap関連
	RegistUserFunction("GetBpp",GetBpp);
	RegistUserFunction("LoadBitmap",LoadBitmapFile);
//	RegistUserFunction("LoadBitmapW",LoadBitmapFileW);
	RegistUserFunction("SaveBitmap",SaveBitmapFile);
	RegistUserFunction("ReleaseBitmap",ReleaseBitmap);
	RegistUserFunction("GetPlaneSize",GetPlaneSize);
	RegistUserFunction("SetColorKeyRGB",SetColorKeyRGB);
	RegistUserFunction("SetColorKeyPos",SetColorKeyPos);
	RegistUserFunction("ClearSecondary",ClearSecondary);
	RegistUserFunction("ClearSecondaryRect",ClearSecondaryRect);
//	RegistUserFunction("SetFillColor",SetFillColor);
	RegistUserFunction("SetFillColorRGB",SetFillColorRGB);
//	RegistUserFunction("SystemMemoryPlane",SystemMemoryPlane);

	RegistUserFunction("Blt",Blt);
	RegistUserFunction("BltRect",BltRect);
	RegistUserFunction("BltFast",BltFast);
	RegistUserFunction("BltFastRect",BltFastRect);
//	RegistUserFunction("BlendBlt",BlendBlt);
//	RegistUserFunction("BlendBltRect",BlendBltRect);
//	RegistUserFunction("BlendBltFast",BlendBltFast);
//	RegistUserFunction("BlendBltFastRect",BlendBltFastRect);
	RegistUserFunction("ClipBlt",ClipBlt);
	RegistUserFunction("ClipBltRect",ClipBltRect);
	RegistUserFunction("ClipBltFast",ClipBltFast);
	RegistUserFunction("ClipBltFastRect",ClipBltFastRect);
//	RegistUserFunction("ClipBlendBlt",ClipBlendBlt);
//	RegistUserFunction("ClipBlendBltRect",ClipBlendBltRect);
//	RegistUserFunction("ClipBlendBltFast",ClipBlendBltFast);
//	RegistUserFunction("ClipBlendBltFastRect",ClipBlendBltFastRect);
	RegistUserFunction("BltR",BltR);
	RegistUserFunction("BltRectR",BltRectR);
	RegistUserFunction("BltFastR",BltFastR);
	RegistUserFunction("BltFastRectR",BltFastRectR);
//	RegistUserFunction("BlendBltR",BlendBltR);
//	RegistUserFunction("BlendBltRectR",BlendBltRectR);
//	RegistUserFunction("BlendBltFastR",BlendBltFastR);
//	RegistUserFunction("BlendBltFastRectR",BlendBltFastRectR);
	RegistUserFunction("ClipBltR",ClipBltR);
	RegistUserFunction("ClipBltRectR",ClipBltRectR);
	RegistUserFunction("ClipBltFastR",ClipBltFastR);
	RegistUserFunction("ClipBltFastRectR",ClipBltFastRectR);
//	RegistUserFunction("ClipBlendBltR",ClipBlendBltR);
//	RegistUserFunction("ClipBlendBltRectR",ClipBlendBltRectR);
//	RegistUserFunction("ClipBlendBltFastR",ClipBlendBltFastR);
//	RegistUserFunction("ClipBlendBltFastRectR",ClipBlendBltFastRectR);
//	RegistUserFunction("EnableBlendColorKey",EnableBlendColorKey);
//	RegistUserFunction("FlushBlt",FlushBlt);
//	RegistUserFunction("MosaicBlt",MosaicBlt);

	//プレーンを転送する関数
	RegistUserFunction("BltSecondaryTo",BltSecondaryTo);			//  (int nPlaneNo)

	RegistUserFunction("SetBltMode",SetBltMode);

	RegistUserFunction("CreateSurface",CreateSurface);
//	RegistUserFunction("SwapToSecondary",SwapToSecondary);
	RegistUserFunction("SetSecondaryOffset",SetSecondaryOffset);
	RegistUserFunction("SetBrightness",SetBrightness);

	//	FPS関連
	RegistUserFunction("SetFPS",SetFPS);
	RegistUserFunction("GetFPS",GetFPS);
	RegistUserFunction("GetRealFPS",GetRealFPS);
	RegistUserFunction("ResetTime",ResetTime);
	RegistUserFunction("GetTime",GetTime);
	RegistUserFunction("PauseTime",PauseTime);
	RegistUserFunction("RestartTime",RestartTime);
	RegistUserFunction("PauseTimeAll",PauseTimeAll);
	RegistUserFunction("RestartTimeAll",RestartTimeAll);
	
	// FPS表示関連
	RegistUserFunction("FPSLayerOn",FPSLayerOn);
	RegistUserFunction("FPSLayerOff",FPSLayerOff);

	//	文字表示関連
	RegistUserFunction("TextLayerOn",TextLayerOn);
	RegistUserFunction("TextLayerOff",TextLayerOff);
	RegistUserFunction("TextMove",TextMove);
	RegistUserFunction("TextOut",TextOut);
	RegistUserFunction("TextSize",TextSize);
	RegistUserFunction("TextHeight",TextHeight);
	RegistUserFunction("TextColor",TextColor);
	RegistUserFunction("TextBackColor",TextBackColor);
	RegistUserFunction("TextBackColorDisable",TextBackColorDisable);
	RegistUserFunction("TextFont",TextFont);
//	RegistUserFunction("TextBlend",TextBlend);
	RegistUserFunction("TextBlt",TextBlt);
	RegistUserFunction("TextGetSize",TextGetSize);

	//	文字列操作関数
	RegistUserFunction("strcpy",strcpy);
	RegistUserFunction("strncpy",strncpy);
	RegistUserFunction("strcat",strcat);
	RegistUserFunction("sprintf",sprintf);

	//	ファイル入出力
	RegistUserFunction("LoadFile",LoadFile);
	RegistUserFunction("SaveFile",SaveFile);

	//	ゲームフラグ管理
	RegistUserFunction("LoadGameFlag",LoadGameFlag);
	RegistUserFunction("SaveGameFlag",SaveGameFlag);
	RegistUserFunction("LoadGameFlag2",LoadGameFlag2);
	RegistUserFunction("SaveGameFlag2",SaveGameFlag2);
	RegistUserFunction("ResetGameFlag",ResetGameFlag);
	RegistUserFunction("ResetGameFlag2",ResetGameFlag2);

	//	その他
	RegistUserFunction("Rand",Rand);
	RegistUserFunction("GetCurrentDirectory",GetCurrentDirectory);
	RegistUserFunction("SetCurrentDirectory",SetCurrentDirectory);

	//	MOUSE入力
	RegistUserFunction("GetMouseInfo",GetMouseInfo);
	RegistUserFunction("SetMouseInfo",SetMouseInfo);
	RegistUserFunction("EnableMouseCursor",EnableMouseCursor);
	RegistUserFunction("MouseInput",MouseInput);

	//	ディスプレイモードの変更
	RegistUserFunction("SetDisplayMode",SetDisplayMode);
	RegistUserFunction("ChangeDisplayMode",ChangeDisplayMode);

	//	シナリオの描画関連
	RegistUserFunction("ScenarioLayerOn",ScenarioLayerOn);
	RegistUserFunction("ScenarioLayerOff",ScenarioLayerOff);
	RegistUserFunction("ScenarioLoad",ScenarioLoad);
	RegistUserFunction("ScenarioOnDraw",ScenarioOnDraw);
	RegistUserFunction("ScenarioEnableSkip",ScenarioEnableSkip);

	m_lpScenario = NULL;

	//	マップの描画関連
	RegistUserFunction("MapLayerOn",MapLayerOn);
	RegistUserFunction("MapLayerOff",MapLayerOff);
	RegistUserFunction("MapLoad",MapLoad);
	RegistUserFunction("MapOnDraw",MapOnDraw);
	RegistUserFunction("MapGetSize",MapGetSize);
	RegistUserFunction("MapSetView",MapSetView);

	m_lpMapLayer = NULL;

	//////////////////////////////////////////////////////

	//	変数追加
	RegistUserVariable("string",(LONG&)m_alpString[0]);

	m_GameFlag	= new LONG [m_nGameFlagMax];
	m_GameFlag2 = new LONG [m_nGameFlagMax2];

	RegistUserVariable("gameflag",m_GameFlag[0]);
	RegistUserVariable("gameflag2",m_GameFlag2[0]);

}

TScript::~TScript(){
	DELETEPTR_SAFE(m_sound);

	DELETEPTR_SAFE(m_gametime);
	DELETE_SAFE(m_fpslayer);
	DELETEPTR_SAFE(m_textlayer);
	DELETEPTR_SAFE(m_alpString);
	DELETEPTR_SAFE(m_plane);
	DELETEPTR_SAFE(m_GameFlag);
	DELETEPTR_SAFE(m_GameFlag2);

	DELETEPTR_SAFE(m_lpScenario);
	DELETEPTR_SAFE(m_lpMapLayer);
}

//////////////////////////////////////////////////////////////////////////////

int		TScript::m_nSoundMax;					//	総Soundチャンネル数
int		TScript::m_nPlaneMax;					//	総プレーン数
int		TScript::m_nGameTimeMax;				//	総GameTime数
int		TScript::m_nTextLayerMax;				//	総テキストレイヤー数
int		TScript::m_nStringMax;					//	string配列の数
int		TScript::m_nStringLength;				//	string配列の大きさ
bool	TScript::m_bUSE_XZ_FOR_BUTTON;			//	XとZキーをスペース、リターンに割り当てる
int		TScript::m_nGameFlagMax;				//	総GameFlag数
int		TScript::m_nGameFlagMax2;				//	総GameFlag2数

int		TScript::m_nBltMode;
int		TScript::m_nBltAlpha;

//////////////////////////////////////////////////////////////////////////////
//	新規追加関数

//	デバッグウィンドゥ
TUserFunc(TScript::Dbg){
	CDbg dbg;
	dbg.Out(p);
	return 0;
}

/////////////////////////////////////////////////////////////////////
//	デバッグ用

TUserFunc(TScript::InnerLog) {
	Err.Out("スクリプトから %d が出力されました",*p);
	return 0;
}

TUserFunc(TScript::InnerLogMes){
	Err.Out("スクリプトから %s が出力されました",(LPSTR)*p);
	return 0;
}

/////////////////////////////////////////////////////////////////////
//	Wave関連
//		CSoundクラスを呼び出すだけだから、苦労はゼロ:p

CSound* TScript::m_sound = NULL;	// コンストラクタで確保しようね:p

TUserFunc(TScript::LoadWave)		{	return m_sound[*(p+1)].Load((LPSTR)*p);}
TUserFunc(TScript::PlayWave)		{	return m_sound[*p].Play();	}
TUserFunc(TScript::StopWave)		{	return m_sound[*p].Stop();	}
TUserFunc(TScript::PauseWave)		{	return m_sound[*p].Pause();	}
TUserFunc(TScript::ReplayWave)		{	return m_sound[*p].Replay();	}
TUserFunc(TScript::IsPlayWave)		{	return m_sound[*p].IsPlay();}
TUserFunc(TScript::SetLoopModeWave)	{	m_sound[*p].SetLoopMode(*(p+1)!=0); return 0; }
TUserFunc(TScript::ReleaseWave)		{	return m_sound[*p].Release();}
TUserFunc(TScript::ReleaseWaveAll)	{
	for(int i=0;i<m_nSoundMax;i++){
		m_sound[i].Release();
	}
	return 0;
}
TUserFunc(TScript::SetVolumeWave)	{	return m_sound[*p].SetVolume(*(p+1));}
TUserFunc(TScript::SetVolumeWaveAll){
	for(int i=0;i<m_nSoundMax;i++){
		m_sound[i].SetVolume(*p);
	}
	return 0;
}
TUserFunc(TScript::SetWaveFormat)	{	return CSound::SetFormat((int)*p); }

/////////////////////////////////////////////////////////////////////
//	MIDI関連
//		MIDIクラスを呼び出すだけだから、苦労はゼロ:p

TUserFunc(TScript::LoadMIDI)		{	return GetApp()->GetMIDIOut()->Open((LPSTR)*p); }
TUserFunc(TScript::PlayMIDI)		{	return GetApp()->GetMIDIOut()->Play(); }
TUserFunc(TScript::StopMIDI)		{	return GetApp()->GetMIDIOut()->Stop(); }
TUserFunc(TScript::PauseMIDI)		{	return GetApp()->GetMIDIOut()->Pause(); }
TUserFunc(TScript::ReplayMIDI)		{	return GetApp()->GetMIDIOut()->Replay(); }
TUserFunc(TScript::ReleaseMIDI)		{	return GetApp()->GetMIDIOut()->Close(); }
TUserFunc(TScript::IsPlayMIDI)		{	return GetApp()->GetMIDIOut()->IsPlay(); }
TUserFunc(TScript::SetLoopModeMIDI)	{	return GetApp()->GetMIDIOut()->SetLoopMode(*p!=0); }

//////////////////////////////////////////////////////////////////////////////
//	CDDA関連
//		CDDAクラスを呼び出すだけだから、苦労はゼロ:p

TUserFunc(TScript::OpenCD)	{ return GetApp()->GetCDDA()->Open();	}
TUserFunc(TScript::CloseCD)	{ return GetApp()->GetCDDA()->Close();	}
TUserFunc(TScript::StopCD)	{ return GetApp()->GetCDDA()->Stop();	}
TUserFunc(TScript::PauseCD)	{ return GetApp()->GetCDDA()->Pause();	}
TUserFunc(TScript::ReplayCD){ return GetApp()->GetCDDA()->Replay(); }

TUserFunc(TScript::IsPlayCD){
	if (GetApp()->GetCDDA()->IsPlay()) return 1; else return 0;
}

TUserFunc(TScript::PlayCDFromStart)	{ return GetApp()->GetCDDA()->Play(); }
TUserFunc(TScript::PlayCD)			{ return GetApp()->GetCDDA()->Play(*p);	}
TUserFunc(TScript::PlayCDFromTo)	{ return GetApp()->GetCDDA()->Play(*p,*(p+1)); }
TUserFunc(TScript::PlayCDPos)		{ return GetApp()->GetCDDA()->PlayDW((DWORD)*p,(DWORD)*(p+1)); }
TUserFunc(TScript::SetLoopModeCD)	{ GetApp()->GetCDDA()->SetLoopMode(*p!=0); return 0;}
TUserFunc(TScript::GetSongMaxCD)	{ return GetApp()->GetCDDA()->GetSongMax();}
TUserFunc(TScript::GetCurrentPosCD)	{ DWORD dw; GetApp()->GetCDDA()->GetCurrentPos(dw); return dw; }
TUserFunc(TScript::EjectCD)			{ return GetApp()->GetCDDA()->Eject(*p!=0);	}

//////////////////////////////////////////////////////////////////////////////
//	タイマー関連

CTimer* TScript::m_gametime = NULL;

TUserFunc(TScript::SetFPS)			{ GetApp()->GetFPSTimer()->SetFPS(*p); return 0; }
TUserFunc(TScript::GetFPS)			{ return GetApp()->GetFPSTimer()->GetFPS(); }
TUserFunc(TScript::GetRealFPS)		{ return GetApp()->GetFPSTimer()->GetRealFPS(); }
TUserFunc(TScript::ResetTime)		{ m_gametime[*p].Reset(); return 0; }
TUserFunc(TScript::GetTime)			{ return m_gametime[*p].Get(); }
TUserFunc(TScript::PauseTime)		{ m_gametime[*p].Pause(); return 0; }
TUserFunc(TScript::RestartTime)		{ m_gametime[*p].Restart(); return 0; }
TUserFunc(TScript::PauseTimeAll)	{ return m_gametime[*p].PauseAll(); }
TUserFunc(TScript::RestartTimeAll)	{ return m_gametime[*p].RestartAll(); }

/////////////////////////////////////////////////////////////////////

CFPSLayer*	TScript::m_fpslayer = NULL;

TUserFunc(TScript::FPSLayerOn)		{
	m_fpslayer = new CFPSLayer(GetApp()->GetFPSTimer());
	if (m_fpslayer!=NULL) {
		m_fpslayer->SetPos(*p,*(p+1));
	}
	return 0;
}
TUserFunc(TScript::FPSLayerOff)		{
	DELETE_SAFE(m_fpslayer);
	return 0;
}

/////////////////////////////////////////////////////////////////////
//	文字列表示関連

CTextLayer*	TScript::m_textlayer = NULL;

TUserFunc(TScript::TextLayerOn)			{
	m_textlayer[*p].Enable(true);
	m_textlayer[*p].SetPos(*(p+1),*(p+2));
	return 0;
}
TUserFunc(TScript::TextLayerOff)		{
	m_textlayer[*p].Enable(false);
	return 0;
}
TUserFunc(TScript::TextMove)			{	m_textlayer[*p].SetPos(*(p+1),*(p+2)); return 0; }
TUserFunc(TScript::TextOut)				{	m_textlayer[*p].GetFont()->SetText((LPSTR)*(p+1));	return 0; }
TUserFunc(TScript::TextSize)			{	m_textlayer[*p].GetFont()->SetSize(*(p+1)); return 0; }
TUserFunc(TScript::TextHeight)			{	m_textlayer[*p].GetFont()->SetHeight(*(p+1)); return 0; }
TUserFunc(TScript::TextColor)			{	m_textlayer[*p].GetFont()->SetColor(RGB(*(p+1),*(p+2),*(p+3))); return 0; }
TUserFunc(TScript::TextBackColor)		{	m_textlayer[*p].GetFont()->SetBackColor(RGB(*(p+1),*(p+2),*(p+3))); return 0; }
TUserFunc(TScript::TextBackColorDisable){	m_textlayer[*p].GetFont()->SetBackColor(CLR_INVALID); return 0; }
TUserFunc(TScript::TextFont)			{	
	if (0<=*(p+1) && *(p+1)<=4) {
		m_textlayer[*p].GetFont()->SetFont(*(p+1));	// 0から4か
	} else {
		m_textlayer[*p].GetFont()->SetFont((LPSTR)*(p+1));	// さもなくば文字列ポインタ
	}
	return 0;
}

// TUserFunc(TScript::TextBlend)			{	/* m_textlayer[*p].SetBlend(*(p+1)); */ return 0; }

TUserFunc(TScript::TextBlt)				{	
	//	textlayerの直接描画には、hdcも必要。
	HDC hdc = GetApp()->GetDraw()->GetSecondary()->GetDC();
	if (hdc!=NULL){
		m_textlayer[*p].OnDraw(hdc);
		GetApp()->GetDraw()->GetSecondary()->ReleaseDC();
		return 0;
	} else {
		return 1;
	}
}
TUserFunc(TScript::TextGetSize)			{	return m_textlayer[*p].GetFont()->GetSize((int&)*(int*)*(p+1),(int&)*(int*)*(p+2)); }

/////////////////////////////////////////////////////////////////////
//	文字列操作関数

LPSTR* TScript::m_alpString;

TUserFunc(TScript::strcpy)				{	return (LONG)::strcpy((LPSTR)*p,(LPSTR)*(p+1)); }
TUserFunc(TScript::strncpy)				{	// 漢字コードを考慮したn文字コピー
	LPSTR p1 = (LPSTR)*p;
	LPSTR p2 = (LPSTR)*(p+1);
	int	  n	 = *(p+2);
	for(;*p2!='\0' && n;n--,p1++){
		*p1 = *(p2++);
		//	漢字なのか？
		if ((((BYTE)*p1)>=0x80 && ((BYTE)*p1)<=0xa0)||(((BYTE)*p1)>=0xe0 && ((BYTE)*p1)<=0xff)) *(++p1) = *(p2++);
	}
	*p1 = '\0';	//	終端文字だけ入れとかなくっちゃ！
	return n;
}
TUserFunc(TScript::strcat)				{	return (LONG)::strcat((LPSTR)*p,(LPSTR)*(p+1)); }
TUserFunc(TScript::sprintf)				{	::wvsprintf((LPSTR)*p,(LPSTR)*(p+1),(va_list)(p+2)); return 0; }

/////////////////////////////////////////////////////////////////////
//	ファイル入出力

TUserFunc(TScript::LoadFile)	{
	CFile file;
	if (file.Read((LPSTR)*p)==0) {
		::CopyMemory((LPBYTE)*(p+1),file.GetMemory(),*(p+2));
		return 0;
	}
	return 1;
}

TUserFunc(TScript::SaveFile)	{
	CFile file;
	return file.Write((LPSTR)*p,(LPBYTE)*(p+1),*(p+2));
}

/////////////////////////////////////////////////////////////////////
//	その他

TUserFunc(TScript::Rand)				{	return rand() % *p; }
TUserFunc(TScript::GetCurrentDirectory)	{
	string dir;
	dir = CFile::GetCurrentDir();
	::lstrcpy((LPSTR)*p,dir.c_str());
	return 0;
}

TUserFunc(TScript::SetCurrentDirectory)	{
	CFile::SetCurrentDir((LPSTR)*p);
	return 0;
}

/////////////////////////////////////////////////////////////////////
//	Bitmap関連
//		CPlaneクラスを呼び出すだけだから、苦労はゼロ:p

CFastPlane* TScript::m_plane = NULL;	// コンストラクタで確保しようね:p

TUserFunc(TScript::GetBpp)			{ return CPlane::GetBpp(); }
TUserFunc(TScript::LoadBitmapFile)	{ return m_plane[*(p+1)].Load((LPSTR)*p /*,*(p+2)!=0*/ ); }
TUserFunc(TScript::SaveBitmapFile)	{
	RECT rc;
	SetRect(&rc,*(p+2),*(p+3),*(p+2) + *(p+4),*(p+3) + *(p+5));
	return m_plane[*(p+1)].Save((LPSTR)*p,&rc);
}
TUserFunc(TScript::ReleaseBitmap)	{ m_plane[*p].Release(); return 0; }
TUserFunc(TScript::GetPlaneSize)	{ m_plane[*p].GetSize((int&)*(long*)*(p+1),(int&)*(long*)*(p+2)); return 0; }
TUserFunc(TScript::SetColorKeyRGB)	{ return m_plane[*p].SetColorKey(RGB(*(p+1),*(p+2),*(p+3))); }
TUserFunc(TScript::SetColorKeyPos)	{ return m_plane[*p].SetColorKey(*(p+1),*(p+2)); }
TUserFunc(TScript::ClearSecondary)	{ return GetApp()->GetDraw()->GetSecondary()->Clear(); }
TUserFunc(TScript::ClearSecondaryRect)	{
	RECT r;
	SetRect(&r,*p,*(p+1),*p + *(p+2),*(p+1) + *(p+3));
	return GetApp()->GetDraw()->GetSecondary()->Clear(&r);
}
TUserFunc(TScript::SetFillColorRGB)	{ GetApp()->GetDraw()->GetSecondary()->SetFillColor(RGB(p[0],p[1],p[2])); return 0; }

//TUserFunc(TScript::SystemMemoryPlane){ return m_plane[*p].SetSystemMemoryUse(p[1]!=0); }

TUserFunc(TScript::BltSecondaryTo){
	int sec_X,sec_Y=0;
	GetApp()->GetDraw()->GetSecondary()->GetSize(sec_X,sec_Y);	//セカンダリサーフェースのサイズを取得
	m_plane[*p].CreateSurface(sec_X,sec_Y);						//上で取得した大きさでCreateSurface
	GetApp()->GetDraw()->GetSecondary()->BltFast(&m_plane[*p],0,0);	//セカンダリサーフェースの内容をBltFast
	return 0;
}

TUserFunc(TScript::SetBltMode){
	m_nBltMode = p[0];
	m_nBltAlpha = p[1];
	return 0;
}

TUserFunc(TScript::Blt)				{
	switch (m_nBltMode) {
	case 0: return GetApp()->GetDraw()->GetSecondary()->BltNatural(&m_plane[*p],*(p+1),*(p+2));
	case 1: return GetApp()->GetDraw()->GetSecondary()->BlendBltFast(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha);
	case 2: return GetApp()->GetDraw()->GetSecondary()->BlendBlt(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha);
	case 3: return GetApp()->GetDraw()->GetSecondary()->AddColorAlphaBltFast(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha);
	case 4: return GetApp()->GetDraw()->GetSecondary()->AddColorAlphaBlt(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha);
	case 5: return GetApp()->GetDraw()->GetSecondary()->SubColorAlphaBltFast(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha);
	case 6: return GetApp()->GetDraw()->GetSecondary()->SubColorAlphaBlt(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha);
	default: return 1;
	}
}
TUserFunc(TScript::BltRect)			{
	RECT sr;
	SetRect(&sr,*(p+3),*(p+4),*(p+3)+*(p+5),*(p+4)+*(p+6));
	switch (m_nBltMode) {
	case 0: return GetApp()->GetDraw()->GetSecondary()->BltNatural(&m_plane[*p],*(p+1),*(p+2),&sr);
	case 1: return GetApp()->GetDraw()->GetSecondary()->BlendBltFast(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha,&sr);
	case 2: return GetApp()->GetDraw()->GetSecondary()->BlendBlt(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha,&sr);
	case 3: return GetApp()->GetDraw()->GetSecondary()->AddColorAlphaBltFast(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha,&sr);
	case 4: return GetApp()->GetDraw()->GetSecondary()->AddColorAlphaBlt(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha,&sr);
	case 5: return GetApp()->GetDraw()->GetSecondary()->SubColorAlphaBltFast(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha,&sr);
	case 6: return GetApp()->GetDraw()->GetSecondary()->SubColorAlphaBlt(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha,&sr);
	default: return 1;
	}
}
TUserFunc(TScript::BltFast)			{ GetApp()->GetDraw()->GetSecondary()->BltFast(&m_plane[*p],*(p+1),*(p+2)); return 0; }
TUserFunc(TScript::BltFastRect)		{
	RECT sr;
	SetRect(&sr,*(p+3),*(p+4),*(p+3)+*(p+5),*(p+4)+*(p+6));
	return GetApp()->GetDraw()->GetSecondary()->BltFast(&m_plane[*p],*(p+1),*(p+2),&sr);
}

	//	転送先Clip系
TUserFunc(TScript::ClipBlt)				{
	RECT sr;
	SetRect(&sr,*(p+3),*(p+4),*(p+3)+*(p+5),*(p+4)+*(p+6));
	switch (m_nBltMode) {
	case 0: return GetApp()->GetDraw()->GetSecondary()->BltNatural(&m_plane[*p],*(p+1),*(p+2),NULL,NULL,&sr);
	case 1: return GetApp()->GetDraw()->GetSecondary()->BlendBltFast(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha,NULL,NULL,&sr);
	case 2: return GetApp()->GetDraw()->GetSecondary()->BlendBlt(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha,NULL,NULL,&sr);
	case 3: return GetApp()->GetDraw()->GetSecondary()->AddColorAlphaBltFast(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha,NULL,NULL,&sr);
	case 4: return GetApp()->GetDraw()->GetSecondary()->AddColorAlphaBlt(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha,NULL,NULL,&sr);
	case 5: return GetApp()->GetDraw()->GetSecondary()->SubColorAlphaBltFast(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha,NULL,NULL,&sr);
	case 6: return GetApp()->GetDraw()->GetSecondary()->SubColorAlphaBlt(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha,NULL,NULL,&sr);
	default: return 1;
	}
}
TUserFunc(TScript::ClipBltRect)			{
	RECT sr;
	SetRect(&sr,*(p+3),*(p+4),*(p+3)+*(p+5),*(p+4)+*(p+6));
	RECT clip;
	SetRect(&clip,*(p+7),*(p+8),*(p+7)+*(p+9),*(p+8)+*(p+10));
	switch (m_nBltMode) {
	case 0: return GetApp()->GetDraw()->GetSecondary()->BltNatural(&m_plane[*p],*(p+1),*(p+2),&sr,NULL,&clip);
	case 1: return GetApp()->GetDraw()->GetSecondary()->BlendBltFast(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha,&sr,NULL,&clip);
	case 2: return GetApp()->GetDraw()->GetSecondary()->BlendBlt(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha,&sr,NULL,&clip);
	case 3: return GetApp()->GetDraw()->GetSecondary()->AddColorAlphaBltFast(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha,&sr,NULL,&clip);
	case 4: return GetApp()->GetDraw()->GetSecondary()->AddColorAlphaBlt(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha,&sr,NULL,&clip);
	case 5: return GetApp()->GetDraw()->GetSecondary()->SubColorAlphaBltFast(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha,&sr,NULL,&clip);
	case 6: return GetApp()->GetDraw()->GetSecondary()->SubColorAlphaBlt(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha,&sr,NULL,&clip);
	default: return 1;
	}
}
TUserFunc(TScript::ClipBltFast)			{
	RECT sr;
	SetRect(&sr,*(p+3),*(p+4),*(p+3)+*(p+5),*(p+4)+*(p+6));
	return GetApp()->GetDraw()->GetSecondary()->BltFast(&m_plane[*p],*(p+1),*(p+2),NULL,NULL,&sr);
}
TUserFunc(TScript::ClipBltFastRect)		{
	RECT sr;
	SetRect(&sr,*(p+3),*(p+4),*(p+3)+*(p+5),*(p+4)+*(p+6));
	RECT clip;
	SetRect(&clip,*(p+7),*(p+8),*(p+7)+*(p+9),*(p+8)+*(p+10));
	return GetApp()->GetDraw()->GetSecondary()->BltFast(&m_plane[*p],*(p+1),*(p+2),&sr,NULL,&clip);
}

//	拡大縮小機能付
TUserFunc(TScript::BltR)			{
	int x,y;
	m_plane[*p].GetSize(x,y);
	SIZE size;
	size.cx = (double)(*(p+3)*x)/65536;
	size.cy = (double)(*(p+4)*y)/65536;
	switch (m_nBltMode) {
	case 0: return GetApp()->GetDraw()->GetSecondary()->BltNatural(&m_plane[*p],*(p+1),*(p+2),NULL,&size);
	case 1: return GetApp()->GetDraw()->GetSecondary()->BlendBltFast(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha,NULL,&size);
	case 2: return GetApp()->GetDraw()->GetSecondary()->BlendBlt(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha,NULL,&size);
	case 3: return GetApp()->GetDraw()->GetSecondary()->AddColorAlphaBltFast(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha,NULL,&size);
	case 4: return GetApp()->GetDraw()->GetSecondary()->AddColorAlphaBlt(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha,NULL,&size);
	case 5: return GetApp()->GetDraw()->GetSecondary()->SubColorAlphaBltFast(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha,NULL,&size);
	case 6: return GetApp()->GetDraw()->GetSecondary()->SubColorAlphaBlt(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha,NULL,&size);
	default: return 1;
	}
}
TUserFunc(TScript::BltRectR)		{
	int x,y;
	m_plane[*p].GetSize(x,y);
	SIZE size;
	size.cx = (double)(*(p+7)*x)/65536;
	size.cy = (double)(*(p+8)*y)/65536;
	RECT sr;
	SetRect(&sr,*(p+3),*(p+4),*(p+3)+*(p+5),*(p+4)+*(p+6));
	switch (m_nBltMode) {
	case 0: return GetApp()->GetDraw()->GetSecondary()->BltNatural(&m_plane[*p],*(p+1),*(p+2),&sr,&size);
	case 1: return GetApp()->GetDraw()->GetSecondary()->BlendBltFast(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha,&sr,&size);
	case 2: return GetApp()->GetDraw()->GetSecondary()->BlendBlt(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha,&sr,&size);
	case 3: return GetApp()->GetDraw()->GetSecondary()->AddColorAlphaBltFast(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha,&sr,&size);
	case 4: return GetApp()->GetDraw()->GetSecondary()->AddColorAlphaBlt(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha,&sr,&size);
	case 5: return GetApp()->GetDraw()->GetSecondary()->SubColorAlphaBltFast(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha,&sr,&size);
	case 6: return GetApp()->GetDraw()->GetSecondary()->SubColorAlphaBlt(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha,&sr,&size);
	default: return 1;
	}
//	return GetApp()->GetDraw()->GetSecondary()->Blt(&m_plane[*p],*(p+1),*(p+2),&sr,&size);
}
TUserFunc(TScript::BltFastR)		{
	int x,y;
	m_plane[*p].GetSize(x,y);
	SIZE size;
	size.cx = (double)(*(p+3)*x)/65536;
	size.cy = (double)(*(p+4)*y)/65536;
	return GetApp()->GetDraw()->GetSecondary()->BltFast(&m_plane[*p],*(p+1),*(p+2),NULL,&size);
}
TUserFunc(TScript::BltFastRectR)	{
	int x,y;
	m_plane[*p].GetSize(x,y);
	SIZE size;
	size.cx = (double)(*(p+7)*x)/65536;
	size.cy = (double)(*(p+8)*y)/65536;
	RECT sr;
	SetRect(&sr,*(p+3),*(p+4),*(p+3)+*(p+5),*(p+4)+*(p+6));
	return GetApp()->GetDraw()->GetSecondary()->BltFast(&m_plane[*p],*(p+1),*(p+2),&sr,&size);
}
TUserFunc(TScript::ClipBltR)		{
	int x,y;
	m_plane[*p].GetSize(x,y);
	SIZE size;
	size.cx = (double)(*(p+7)*x)/65536;
	size.cy = (double)(*(p+8)*y)/65536;
	RECT sr;
	SetRect(&sr,*(p+3),*(p+4),*(p+3)+*(p+5),*(p+4)+*(p+6));
	switch (m_nBltMode) {
	case 0: return GetApp()->GetDraw()->GetSecondary()->BltNatural(&m_plane[*p],*(p+1),*(p+2),NULL,&size,&sr);
	case 1: return GetApp()->GetDraw()->GetSecondary()->BlendBltFast(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha,NULL,&size,&sr);
	case 2: return GetApp()->GetDraw()->GetSecondary()->BlendBlt(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha,NULL,&size,&sr);
	case 3: return GetApp()->GetDraw()->GetSecondary()->AddColorAlphaBltFast(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha,NULL,&size,&sr);
	case 4: return GetApp()->GetDraw()->GetSecondary()->AddColorAlphaBlt(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha,NULL,&size,&sr);
	case 5: return GetApp()->GetDraw()->GetSecondary()->SubColorAlphaBltFast(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha,NULL,&size,&sr);
	case 6: return GetApp()->GetDraw()->GetSecondary()->SubColorAlphaBlt(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha,NULL,&size,&sr);
	default: return 1;
	}
//	return GetApp()->GetDraw()->GetSecondary()->Blt(&m_plane[*p],*(p+1),*(p+2),NULL,&size,&sr);
}
TUserFunc(TScript::ClipBltRectR)	{
	int x,y;
	m_plane[*p].GetSize(x,y);
	SIZE size;
	size.cx = (double)(*(p+11)*x)/65536;
	size.cy = (double)(*(p+12)*y)/65536;
	RECT sr;
	SetRect(&sr,*(p+3),*(p+4),*(p+3)+*(p+5),*(p+4)+*(p+6));
	RECT clip;
	SetRect(&clip,*(p+7),*(p+8),*(p+7)+*(p+9),*(p+8)+*(p+10));
	switch (m_nBltMode) {
	case 0: return GetApp()->GetDraw()->GetSecondary()->BltNatural(&m_plane[*p],*(p+1),*(p+2),&sr,&size,&clip);
	case 1: return GetApp()->GetDraw()->GetSecondary()->BlendBltFast(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha,&sr,&size,&clip);
	case 2: return GetApp()->GetDraw()->GetSecondary()->BlendBlt(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha,&sr,&size,&clip);
	case 3: return GetApp()->GetDraw()->GetSecondary()->AddColorAlphaBltFast(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha,&sr,&size,&clip);
	case 4: return GetApp()->GetDraw()->GetSecondary()->AddColorAlphaBlt(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha,&sr,&size,&clip);
	case 5: return GetApp()->GetDraw()->GetSecondary()->SubColorAlphaBltFast(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha,&sr,&size,&clip);
	case 6: return GetApp()->GetDraw()->GetSecondary()->SubColorAlphaBlt(&m_plane[*p],*(p+1),*(p+2),m_nBltAlpha,&sr,&size,&clip);
	default: return 1;
	}
//	return GetApp()->GetDraw()->GetSecondary()->Blt(&m_plane[*p],*(p+1),*(p+2),&sr,&size,&clip);
}
TUserFunc(TScript::ClipBltFastR)	{
	int x,y;
	m_plane[*p].GetSize(x,y);
	SIZE size;
	size.cx = (double)(*(p+7)*x)/65536;
	size.cy = (double)(*(p+8)*y)/65536;
	RECT sr;
	SetRect(&sr,*(p+3),*(p+4),*(p+3)+*(p+5),*(p+4)+*(p+6));
	return GetApp()->GetDraw()->GetSecondary()->BltFast(&m_plane[*p],*(p+1),*(p+2),NULL,&size,&sr);
}
TUserFunc(TScript::ClipBltFastRectR){
	int x,y;
	m_plane[*p].GetSize(x,y);
	SIZE size;
	size.cx = (double)(*(p+11)*x)/65536;
	size.cy = (double)(*(p+12)*y)/65536;
	RECT sr;
	SetRect(&sr,*(p+3),*(p+4),*(p+3)+*(p+5),*(p+4)+*(p+6));
	RECT clip;
	SetRect(&clip,*(p+7),*(p+8),*(p+7)+*(p+9),*(p+8)+*(p+10));
	return GetApp()->GetDraw()->GetSecondary()->BltFast(&m_plane[*p],*(p+1),*(p+2),&sr,&size,&clip);
}
TUserFunc(TScript::SetSecondaryOffset){
	GetApp()->GetDraw()->SetOffset(*p,*(p+1));
	return 0;
}
TUserFunc(TScript::CreateSurface){
	return m_plane[*p].CreateSurface(*(p+1),*(p+2));
}
/////////////////////////////////////////////////////////////////////
//	キー入力関数

TUserFunc(TScript::KeyInput)			{	GetApp()->GetKey()->Input(); return 0; }

TUserFunc(TScript::IsPushKey)			{
	GetApp()->GetDirectInput()->GetKeyState();
	return GetApp()->GetDirectInput()->IsKeyPush(*p)?1:0;
}

TUserFunc(TScript::IsPressKey)			{
	GetApp()->GetDirectInput()->GetKeyState();
	return GetApp()->GetDirectInput()->IsKeyPress(*p)?1:0;
}

TUserFunc(TScript::IsPressUpKey)		{	return GetApp()->GetKey()->IsVKeyPress(1)?1:0; }
TUserFunc(TScript::IsPressDownKey)		{	return GetApp()->GetKey()->IsVKeyPress(2)?1:0; }
TUserFunc(TScript::IsPressLeftKey)		{	return GetApp()->GetKey()->IsVKeyPress(3)?1:0; }
TUserFunc(TScript::IsPressRightKey)		{	return GetApp()->GetKey()->IsVKeyPress(4)?1:0; }
TUserFunc(TScript::IsPressSpaceKey)		{	return GetApp()->GetKey()->IsVKeyPress(5)?1:0; }
TUserFunc(TScript::IsPressReturnKey)	{	return GetApp()->GetKey()->IsVKeyPress(6)?1:0; }
TUserFunc(TScript::IsPressEscKey)		{	return GetApp()->GetKey()->IsVKeyPress(0)?1:0; }
TUserFunc(TScript::IsPushUpKey)			{	return GetApp()->GetKey()->IsVKeyPush(1)?1:0; }
TUserFunc(TScript::IsPushDownKey)		{	return GetApp()->GetKey()->IsVKeyPush(2)?1:0; }
TUserFunc(TScript::IsPushLeftKey)		{	return GetApp()->GetKey()->IsVKeyPush(3)?1:0; }
TUserFunc(TScript::IsPushRightKey)		{	return GetApp()->GetKey()->IsVKeyPush(4)?1:0; }
TUserFunc(TScript::IsPushSpaceKey)		{	return GetApp()->GetKey()->IsVKeyPush(5)?1:0; }
TUserFunc(TScript::IsPushReturnKey)		{	return GetApp()->GetKey()->IsVKeyPush(6)?1:0; }
TUserFunc(TScript::IsPushEscKey)		{	return GetApp()->GetKey()->IsVKeyPush(0)?1:0; }

////////////////////////////////////////////////////////////////////
//	ジョイスティック関連
TUserFunc(TScript::SetJoyButtonMax){
	GetApp()->GetJoystick()->SetButtonMax(*p);
	return 0;
}
TUserFunc(TScript::IsPushJoyKey){
	GetApp()->GetJoystick()->GetKeyState();
	return GetApp()->GetJoystick()->IsKeyPush(*p)?1:0;
}
TUserFunc(TScript::IsPressJoyKey){
	GetApp()->GetJoystick()->GetKeyState();
	return GetApp()->GetJoystick()->IsKeyPress(*p)?1:0;
}
TUserFunc(TScript::SelectJoyStick){
	switch (*p){
	case 1:GetApp()->GetJoystick()->SelectDevice(jsJOYSTICK1);break;
	case 2:GetApp()->GetJoystick()->SelectDevice(jsJOYSTICK2);break;
	default: break;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////
//	MIDI入力関連
TUserFunc(TScript::IsPushMIDIKey){
	GetApp()->GetMIDIIn()->GetKeyState();
	return GetApp()->GetMIDIIn()->IsKeyPush(*p)?1:0;
}
TUserFunc(TScript::IsPressMIDIKey){
	GetApp()->GetMIDIIn()->GetKeyState();
	return GetApp()->GetMIDIIn()->IsKeyPress(*p)?1:0;
}
TUserFunc(TScript::GetVelocityMIDIKey){
	GetApp()->GetMIDIIn()->GetKeyState();
	return GetApp()->GetMIDIIn()->GetVelocity(*p);
}



/////////////////////////////////////////////////////////////////////
//	ユーザー変数関連

LONG*	TScript::m_GameFlag;					//	総GameFlag数
LONG*	TScript::m_GameFlag2;					//	総GameFlag2数

TUserFunc(TScript::LoadGameFlag)		{
	CFile file;
	if (file.Read((LPSTR)*p)==0) {
		::CopyMemory(TScript::m_GameFlag,file.GetMemory(),(sizeof(LONG))* m_nGameFlagMax);
		return 0;
	}
	return 1;
}

TUserFunc(TScript::SaveGameFlag)		{
	CFile file;
	return file.Write((LPSTR)*p,TScript::m_GameFlag,(sizeof(LONG)) * m_nGameFlagMax);
}

TUserFunc(TScript::LoadGameFlag2)		{
	CFile file;
	if (file.Read((LPSTR)*p)==0) {
		::CopyMemory(TScript::m_GameFlag2,file.GetMemory(),(sizeof(LONG))* m_nGameFlagMax2);
		return 0;
	}
	return 1;
}

TUserFunc(TScript::SaveGameFlag2)		{
	CFile file;
	return file.Write((LPSTR)*p,TScript::m_GameFlag2,(sizeof(LONG)) * m_nGameFlagMax2);
}

TUserFunc(TScript::ResetGameFlag){
	for (int i=0;i<m_nGameFlagMax;i++){
		m_GameFlag[i] = 0;
	}
	return 0;
}

TUserFunc(TScript::ResetGameFlag2){
	for (int i=0;i<m_nGameFlagMax2;i++){
		m_GameFlag2[i] = 0;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////

TUserFunc(TScript::GetMouseInfo)		{
	GetApp()->GetMouse()->GetInfo((int&)*(int*)*p,(int&)*(int*)*(p+1),(int&)*(int*)*(p+2));
	return 0;
}
TUserFunc(TScript::SetMouseInfo)		{
	GetApp()->GetMouse()->SetXY((int)*p,(int)*(p+1));
	return 0;
}
TUserFunc(TScript::EnableMouseCursor)	{
	GetApp()->GetMouseLayer()->Enable(p[0]!=0);
	return 0;
}
TUserFunc(TScript::MouseInput)		{
	GetApp()->GetMouse()->Flush();
	return 0;
}

/////////////////////////////////////////////////////////////////////
//	ディスプレイモードの変更

int TScript::m_nDisplayMode1 = 0;
int TScript::m_nDisplayMode2 = 1;

TUserFunc(TScript::SetDisplayMode)		{ m_nDisplayMode1 = *p; m_nDisplayMode2 = *(p+1); return 0; }
TUserFunc(TScript::ChangeDisplayMode)	{
	int nDisplayMode = 0;
	switch (*p) {
	case 1: nDisplayMode = m_nDisplayMode1; break;
	case 2: nDisplayMode = m_nDisplayMode2; break;
	default: return 1;	// Error!!
	}
	// DisplayMode 0:WindowMode,1:FullScreen256,2:FullScreen64K,3:FullScreen16M,4:TrueColor
	if (nDisplayMode==0) {
		return GetApp()->GetDraw()->SetDisplay(false);
	}

	GetApp()->GetDraw()->BeginChangeDisplay();

	switch(nDisplayMode){
	case 1: break;
	case 2: goto Disp16;
	case 3: goto Disp24;
	case 4: goto Disp32;
	}

	//	FullScreenModeのときは、希望するモードにならなければ、その次の解像度にする。
		GetApp()->GetDraw()->TestDisplayMode(640,480,true,8);
Disp16:
		GetApp()->GetDraw()->TestDisplayMode(640,480,true,16);
Disp24:
		GetApp()->GetDraw()->TestDisplayMode(640,480,true,24);
Disp32:
		GetApp()->GetDraw()->TestDisplayMode(640,480,true,32);
		GetApp()->GetDraw()->TestDisplayMode(640,480,false); // Windowモードならなんでも良い
	return GetApp()->GetDraw()->EndChangeDisplay();
}

/////////////////////////////////////////////////////////////////////
//	画面効果関連

TUserFunc(TScript::SetBrightness)		{	GetApp()->GetDraw()->SetBrightness(*p); return 0; }

/////////////////////////////////////////////////////////////////////
//	シナリオの描画関連
/////////////////////////////////////////////////////////////////////
CScenarioDraw* TScript::m_lpScenario;

TUserFunc(TScript::ScenarioLayerOn)		{
	DELETE_SAFE(m_lpScenario);
	m_lpScenario = new CScenarioDraw;

	//	このあと、初期化
	CScenarioDraw& scn = *m_lpScenario;

	//	バックログ
	scn.SetBackLog(smart_ptr<CScenarioDrawBackLogManager>(new CScenarioDrawBackLogManager,true));

	//	各種 plug-in
	scn.SetTextDraw(smart_ptr<CTextDrawBase>(new CTextDrawFastPlaneA,true));
	scn.SetBGLoader(smart_ptr<CPlaneLoaderBasePre>(new CFastPlaneLoader,true));
	scn.SetBGMLoader(smart_ptr<CBGMLoader>(new CBGMLoader,true));
	scn.SetSELoader(smart_ptr<CSELoader>(new CSELoader,true));
	scn.SetSCLoader(smart_ptr<CPlaneLoaderBasePre>(new CFastPlaneLoader,true));
	scn.SetFaceLoader(smart_ptr<CPlaneLoaderBasePre>(new CFastPlaneLoader,true));
	scn.SetNameLoader(smart_ptr<CPlaneLoaderBasePre>(new CFastPlaneLoader,true));
	scn.SetButtonLoader(smart_ptr<CPlaneLoaderBasePre>(new CFastPlaneLoader,true));

/*
	//	effect factory
	scn.SetScenarioEffectFactory(smart_ptr<CScenarioEffectFactory>(new CMyScenarioEffectFactory,true));
*/

	//	外字の使用
	scn.SetGaiji(smart_ptr<CSpriteChara>(new CSpriteChara,true));
	scn.GetGaiji()->Load("grp/scenario/gaiji_define.txt");

/*
	//	置換文字列を設定する
	smart_ptr<vector<string> > aszRepString;
	aszRepString.Add();
	aszRepString->push_back("おっすおら");
	aszRepString->push_back("悟空");
	scn.SetRepString( aszRepString );
*/

	//	以下の２つは所有権は移さない
	scn.SetKey(GetApp()->GetKey());
	scn.SetMouse(GetApp()->GetMouse());

	//	設定ファイル
	scn.SetConfigFile("grp/scenario/layout_define.txt");

	//	BGMはストリーム再生だい！
	scn.GetBGMLoader()->UseStreamSound(true);

	scn.SetSkipFast(false);

/*
	//	debug用テスト
	//	インデントうまいこといくんか？
	scn.GetTextDraw()->GetIndent()[0] = 5;
*/

/*
	//	if 用のListener
	class CIfListener : public CScenarioIfListener {
		virtual bool Abstract_If(int n){
			return n==2;
		}
	};
	scn.SetScenarioIfListener(smart_ptr<CScenarioIfListener>(new CIfListener,true));
*/
	return 0;
}

TUserFunc(TScript::ScenarioLayerOff)		{
	DELETE_SAFE(m_lpScenario);
	return 0;
}

TUserFunc(TScript::ScenarioLoad){
	LPSTR szFileName = (LPSTR)p[0];
	return m_lpScenario->Open(szFileName);
}

TUserFunc(TScript::ScenarioOnDraw){
	if (m_lpScenario!=NULL){
		LRESULT lr = m_lpScenario->OnDraw(GetApp()->GetDraw()->GetSecondary());
		if (lr!=0) return lr;
		m_lpScenario->GetSELoader()->OnPlay();
		return m_lpScenario->Input();
	}
	return -1;
}

TUserFunc(TScript::ScenarioEnableSkip){
	if (m_lpScenario!=NULL){
		m_lpScenario->SetSkipFast(p[0]!=0);
		return 0;
	}
	return -1;
}


/////////////////////////////////////////////////////////////////////
//	マップの描画関連
/////////////////////////////////////////////////////////////////////
CFineMapLayer* TScript::m_lpMapLayer;

TUserFunc(TScript::MapLayerOn)		{
	DELETE_SAFE(m_lpMapLayer);
	m_lpMapLayer = new CFineMapLayer;
	RECT rc;
	::SetRect(&rc,0,0,640,480);
	m_lpMapLayer->SetView(&rc);
	return 0;
}

TUserFunc(TScript::MapLayerOff)		{
	DELETE_SAFE(m_lpMapLayer);
	return 0;
}

TUserFunc(TScript::MapOnDraw)		{
	if (m_lpMapLayer==NULL) return 1;
	m_lpMapLayer->SetPos(p[0],p[1]);		//	表示エリア左上の来るべき仮想スクリーン座標
	int nDrawType = p[2];
	switch (nDrawType){
	case 0 : // 全部描画
		m_lpMapLayer->OnDraw(GetApp()->GetDraw()->GetSecondary());
		break ;
	case 1 : // クリッピング計算
		m_lpMapLayer->OnPaint0(GetApp()->GetDraw()->GetSecondary());
		break ;
	case 2 : // 下レイヤ
		m_lpMapLayer->OnPaint1(GetApp()->GetDraw()->GetSecondary());
		break ;
	case 3 : // 中レイヤ
		m_lpMapLayer->OnPaint2(GetApp()->GetDraw()->GetSecondary());
		break ;
	case 4 : // 上レイヤ
		m_lpMapLayer->OnPaint3(GetApp()->GetDraw()->GetSecondary());
		break ;
	case 5 : // 側面
		m_lpMapLayer->OnPaintSide(GetApp()->GetDraw()->GetSecondary());
		break ;
	}
	return 0;
}

TUserFunc(TScript::MapLoad)		{
	if (m_lpMapLayer==NULL) return 1;
	return m_lpMapLayer->Load((LPSTR)p[0]);
}

TUserFunc(TScript::MapSetView)		{
	if (m_lpMapLayer==NULL) return 1;
	RECT rc;
	::SetRect(&rc,p[0],p[1],p[2],p[3]);
	m_lpMapLayer->SetView(&rc);
	return 0;
}

TUserFunc(TScript::MapGetSize)		{
	if (m_lpMapLayer==NULL) return 1;
	m_lpMapLayer->GetMapArea((int&)*(int*)*(p),(int&)*(int*)*(p+1));
	return 0;
}
