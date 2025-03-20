#ifndef _CFineMapLayer_H
#define _CFineMapLayer_H

#include "CFineMapIO.h"

////////////////////////////////////////////////////////////////////////////////////

//	バンク構造体
class CFineMapBank {
public:
	smart_ptr<CPlaneBase>	m_lpPlane;	// プレーン
	vector<WORD>			m_awHeight;	// バンク高さ
};

//	マップチップクラス
class CFineMapChip {
public:
	WORD			chip[4];		//	下・中・上のチップ／一つは予備
	CFineMapBank*	bank[3];		//	各チップの転送元プレーン
	DWORD			hit;			//	床判定
	WORD			height;			//	高さ

	// 拡張情報
	WORD			schip[4];		// 側面チップ(それぞれ 0°,90°,180°,270°)
	CFineMapBank*	sbank[4];		// 側面のバンクナンバー

	CFineMapChip()	{
		ZERO(chip); ZERO(bank);		//	これ変更したければ変更するとええのだ
		ZERO(schip);ZERO(sbank);
		hit = 0;
		height = 0;
	}
	virtual ~CFineMapChip(){}
};

////////////////////////////////////////////////////////////////////////////////////

//const int MAPCHARA_MAX=256;		//	各レイヤの最大キャラ数
//const int MAPLAYER_MAX=12;		//	レイヤの最大数

////////////////////////////////////////////////////////////////////////////////////

class CFineMapLayer {
public:

	LRESULT	Load(string szFileName);	//	マップの読み込み
	void	Release(void);			//	マップの解放

	//	マップキャラのリセット（CMapCharaへの登録とOnDrawに先行して呼び出してね）
	void	ResetMapChara(void);
	//	マップキャラの登録
	void	RegistMapChara(SSprite&chara,int layer){
		m_MapChara[layer][m_nMapChara[layer]++] = chara;
	}
	//	先頭を取得（マップキャプチャー用）
	SSprite*	GetMapChara(void) { return &m_MapChara[0][0]; }
	//	デバッグ用クリア矩形
	void	SetClearRect(RECT&rc) { m_rcClear = rc; }

	virtual void	OnDraw(CPlaneBase*);	//	実描画
	virtual void	OnDraw2(CPlaneBase*);	//	こちらは画面クリッピング無し
	void	SetPos(int x,int y);		//	表示エリア左上の来るべき仮想スクリーン座標
	void	GetPos(int&x,int&y);		//	↑の取得
	void	SetView(LPRECT lpRect);		//	描画エリア
	LPRECT	GetView(void);				//	↑の取得

	//////////////////////////////////////////////////////////////////////

	void	SetMapSize(int x,int y);		//	マップ全体のサイズ設定
	void	GetMapSize(int &x,int &y);		//	マップ全体のサイズ取得
	void	SetMapChipSize(int x,int y);	//	マップチップのサイズ設定
	void	SetMapChip2Size(int x,int y);	//	拡張マップチップのサイズ設定
	void	GetMapArea(int &x,int &y);		//	各軸方向のマップチップ×配置チップ数で仮想画面サイズを得る
	CFineMapChip*GetMapChip(int x,int y);	//	マップチップへのポインタを得る
	void	ResizeMap(int x,int y);			//	出来る限り、いまのマップを温存してサイズ変更。

	void	SetAngle(int nAngle);			//	視点設定(0:0°1:90°2:180°3:270°)
	int		GetAngle(void);					//	ビューモード取得
	void	SetShowHeight(bool bShow);		//	高さを表示するかを設定
	bool	IsShowHeight(void);				//	高さを表示するか取得

	//////////////////////////////////////////////////////////////////////
	//	床判定

	bool	IsWall(int x,int y);			//	(x,y)は床か？
	DWORD*	GetWall(int x,int y);			//	(x,y)の床情報へのポインタを直接得る
	void	InvalidateWall(bool b);			//	すべての壁を無効にする（デバッグ用）

	//////////////////////////////////////////////////////////////////////
	//	スプライト描画にも対応

	//	通常描画
	void	Blt(CSprite*lpSprite){int x,y;lpSprite->GetPos(x,y);Blt(lpSprite,x,y);}
	//	(x,y)に描画
	void	Blt(CSprite*lpSprite,int x,int y){ BltFix(lpSprite,x,y); lpSprite->IncMotion(); }
	//	通常描画(モーション進めず)
	void	BltFix(CSprite*lpSprite){int x,y;lpSprite->GetPos(x,y);BltFix(lpSprite,x,y);}
	//	(x,y)に描画(モーション進めず)
	void	BltFix(CSprite*lpSprite,int x,int y);
	//	ケツになっていたら、それ以上は加算しない
	void	BltOnce(CSprite*lpSprite,int x,int y);

	CFineMapLayer();
	virtual ~CFineMapLayer();

protected:
	//	マップに表示するキャラクター
	SSprite		m_MapChara[MAPLAYER_MAX][MAPCHARA_MAX];
	int			m_nMapChara[MAPLAYER_MAX];
	int			m_nMapChara2[MAPLAYER_MAX];	//	カウンタ

	int		m_nMapX,m_nMapY;		// マップ全体のサイズ
	int		m_nMapCX,m_nMapCY;		// マップチップの大きさ
	int		m_nMapCX2,m_nMapCY2;	// マップチップの抜き色となる
	vector<CFineMapChip>	m_avMapData;	// マップデータ(サイズ分だけ。すなわちm_MapX×m_MapYだけ)
	vector<CFineMapBank>	m_avMapBank;	// マップを表すバンク

	CSinTable sin;// 回転計算用

public:	//	こいつ、ygs2001で使用するんで、publicに
	// ４段階に分けて、下・中・上レイヤを表示する
	void	OnPaint0(CPlaneBase*);		//	クリッピングルーチン
	void	OnPaint1(CPlaneBase*);		//	下
	void	OnPaint2(CPlaneBase*);		//	中
	void	OnPaint3(CPlaneBase*);		//	上
	void	OnPaintSide(CPlaneBase*);	//	側面
protected:
	//	数字指定でのレイヤ描画
	void	OnPaintLayer(CPlaneBase*,int nStartLayer,int nEndLayer=-1);
	//	立ち位置で整順する
	void	OnSortLayer(int nStartLayer,int nEndLayer=-1);

	//	表示ポジション
	int		m_nX,m_nY;

	//	画面の表示矩形
	RECT	m_rcDraw;

	//	壁の無効化フラグ
	bool	m_bInvalid;

	//	クリア矩形（デバッグ用）
	RECT	m_rcClear;

	//	高さを表示するか
	bool	m_bShowHeight;

	//	現在の表示モード(0:トップビュー 1:クォータービュー)
	int		m_nAngle;

};

#endif
