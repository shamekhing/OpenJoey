
#include "stdafx.h"
#include "CFineMapIO.h"
#include "CFineMapLayer.h"

//	Layer
//		0,1,2	:	下レイヤの下に描画
//		3		:	下レイヤとブレンドされるレイヤ
//		4,5,6	:	下レイヤより上、中レイヤより下に描画
//		7,8,9	:	中レイヤとブレンドされるレイヤ
//		10		:	中レイヤより上、上レイヤより下
//		11		:	上チップより上

//	立ち位置でのソート用
static int map_compare( const void *lpCMapChara1, const void *lpCMapChara2 )
{
   return (((SSprite*)lpCMapChara1)->nOy ) -  (((SSprite*)lpCMapChara2)->nOy);
}

///////////////////////////////////////////////////////////////////////////////

CFineMapLayer::CFineMapLayer(void){
	m_nMapX	  = m_nMapY	  = 0;
	m_nMapCX  = m_nMapCY  = 1;	// ０割エラー出たらシャレならんので（笑）
	m_nMapCX2 = m_nMapCY2 = 0;
	ResetMapChara();
	::SetRect(&m_rcDraw,0,0,640,480);	//	一応、ディフォルトでは全域
	SetPos(0,0);
	m_bInvalid	= false;
	::SetRect(&m_rcClear,0,0,0,0);

	m_bShowHeight = true;
	m_nAngle = 0;
}

CFineMapLayer::~CFineMapLayer(){
	m_avMapData.clear();
	m_avMapBank.clear();
}

///////////////////////////////////////////////////////////////////////////////

void	CFineMapLayer::SetMapChipSize(int x,int y){
	// マップチップのサイズ設定
	m_nMapCX = x;
	m_nMapCY = y;
}

void	CFineMapLayer::SetMapChip2Size(int x,int y) {
	// 拡張チップのサイズ設定
	m_nMapCX2 = x;
	m_nMapCY2 = y;
}

void	CFineMapLayer::SetMapSize(int x,int y){
	// マップ全体のサイズ設定
	m_avMapData.resize(x*y);	//	CMapChipのコンストラクタが働いて初期化してくれているはず
	m_nMapX = x;
	m_nMapY = y;
}

void	CFineMapLayer::GetMapSize(int &x,int &y){
	x = m_nMapX;
	y = m_nMapY;
}

CFineMapChip*	CFineMapLayer::GetMapChip(int x,int y){
	if (x<0 || y<0 || x>=m_nMapX || y>=m_nMapY) return NULL;
	return &m_avMapData[x + m_nMapX * y];
}

void	CFineMapLayer::GetMapArea(int &x,int &y){
	//	マップ全体のドット数を得る
	x = m_nMapX * m_nMapCX;
	y = m_nMapY * m_nMapCY;
}

void	CFineMapLayer::SetAngle(int nAngle) {
	m_nAngle = nAngle;
}
int		CFineMapLayer::GetAngle(void) {
	return m_nAngle;
}
void	CFineMapLayer::SetShowHeight(bool bShow) {
	m_bShowHeight = bShow;
}
bool	CFineMapLayer::IsShowHeight(void) {
	return m_bShowHeight;
}

void	CFineMapLayer::ResizeMap(int x,int y){
	// 出来る限り、いまのマップを温存してサイズ変更。
	vector<CFineMapChip> avData(x*y);

	for(int j=0;j<y;j++){
		for(int i=0;i<x;i++){
			if (i<m_nMapX && j<m_nMapY) {
				avData[i + j*x] = m_avMapData[i + j*m_nMapX];	//	fixed '00/01/03
			} else {
				/*	//	マップエディタの仕様によるが…
				lpdwData[i + j*x].chip[0] = -1;
				lpdwData[i + j*x].chip[1] = -1;
				lpdwData[i + j*x].chip[2] = -1;
				lpdwData[i + j*x].chip[3] = -1;
				lpdwData[i + j*x].hit = -1;
				*/
			}
		}
	}

	m_avMapData = avData;	// ベクターなのでコピーで可
	m_nMapX = x;			// リサイズされたサイズを反映
	m_nMapY = y;
}

void	CFineMapLayer::InvalidateWall(bool b){
	m_bInvalid = b;
}

//////////////////////////////////////////////////////////////////////////////

void	CFineMapLayer::ResetMapChara(void) {
	ZERO(m_nMapChara);
}

//////////////////////////////////////////////////////////////////////////////

void	CFineMapLayer::OnDraw(CPlaneBase*lpDraw){
	OnPaint0(lpDraw);		//	クリッピング
	OnPaint1(lpDraw);		//	下レイヤ
	OnPaintSide(lpDraw);	//	側面チップ
	OnPaint2(lpDraw);		//	中レイヤ
	OnPaint3(lpDraw);		//	上レイヤ
}

//	クリッピングを行なわない（マップのキャプチャー用）
void	CFineMapLayer::OnDraw2(CPlaneBase*lpDraw){
	//	キャラクターレイヤブレンドカウンタをリセット
	ZERO(m_nMapChara2);

	OnPaint1(lpDraw);		//	下レイヤ
	OnPaintSide(lpDraw);	//	側面チップ
	OnPaint2(lpDraw);		//	中レイヤ
	OnPaint3(lpDraw);		//	上レイヤ
}

//////////////////////////////////////////////////////////////////////////////

void	CFineMapLayer::OnPaint0(CPlaneBase*lpDraw){
	//	クリッピング
	int	msx,msy;	//	マップサイズ取得
	GetMapArea(msx,msy);
	int scx,scy;
	scx = m_rcDraw.right-m_rcDraw.left;
	scy = m_rcDraw.bottom-m_rcDraw.top;
	if (msx<scx || msy<scy) {	//	マップサイズのほうが小さい？
		lpDraw->ClearRect(&m_rcDraw);	//	画面クリア
	}
	//	はみ出ている分を押し戻す
	if (m_nX<0) {
		m_nX = 0;
	} else if (m_nX+scx > msx) {
		m_nX = msx - scx;
	}
	if (m_nY<0) {
		m_nY = 0;
	} else if (m_nY+scy > msy) {
		m_nY = msy - scy;
	}

	//	キャラクターレイヤブレンドカウンタをリセット
	ZERO(m_nMapChara2);
}

//	下レイヤ
void	CFineMapLayer::OnPaint1(CPlaneBase*lpDraw){
	OnPaintLayer(lpDraw,0,2);
//		0,1,2	:	下レイヤの下に描画
	OnSortLayer(3);

	// マップチップ位置算出
	int MX,MY,SX;	// マップチップ番号の算出
	int x=0,y=0;	//	忘れてたにょ:p
	WORD chip;		//	temporary
	int cx,cy;		//	cx,cy
	RECT r;			//	rect

	//	マイナスの場合、まずいが、そのチェックはしてあるので構わない
	SX = m_nX / m_nMapCX;
	MY = m_nY / m_nMapCY;

	int nSX,nSY;// マップのサイズ
	int nCX,nCY;// マップチップのサイズ
	int nCX2,nCY2;// 抜き色となる左上の三角形のサイズ
	nSX = m_nMapX;
	nSY = m_nMapY;
	nCX = m_nMapCX;
	nCY = m_nMapCY;
	nCX2= m_nMapCX2;
	nCY2= m_nMapCY2;

	//	マイナスの場合、まずいが、そのチェックはしてあるので構わない
	int sx,sy;// 表示領域のサイズ
	int ox,oy;// 表示差分
	int nAngle;// 視点
	sx = m_rcDraw.right;
	sy = m_rcDraw.bottom;
	ox = oy = 0;
	nAngle = GetAngle();
	// 角度に応じて方向をかえる
	if((nAngle/2)==1) {
		oy = sy-1-nCY;// (0,0) のチップの表示位置
	}
	if(nAngle==1 || nAngle==2) {
		ox = sx-1-nCX;// (0,0) のチップの表示位置
	}
	if((nAngle%2)==1) {// 90°,270°なら縦横逆
		swap(sx,sy);
	}

	for(y = m_rcDraw.top-(m_nY % nCY) ; y<sy ; y+=nCY-nCY2){
		MX = SX;
		for(x = m_rcDraw.left -(m_nX % nCX) ; x<sx ; x+=nCX-nCX2){
			if (MX>=0 && MX<nSX && MY>=0 && MY<nSY){
				CFineMapChip* lpChip = GetMapChip(MX,MY);
				if ((chip = lpChip->chip[0])!=0xffff){ // -1ならば配置されていない
					cx = (chip & 0xff)*nCX;	// 下位8ビットがマップチップのＸ座標
					cy = (chip >> 8	 )*nCY;

					WARNING (cx>=640 || cy>=480 , "CFineMapLayer::OnPaint1でマップのソースが範囲外");

					::SetRect(&r,cx,cy,cx+nCX,cy+nCY);

					// 高さを表示する高さ分ずらす
					int height = 0;
					if(IsShowHeight())height = lpChip->height;

					int nAngle = GetAngle()*128;// 回転角度(上の nAngle と使い方が違う)
					int px,py;
					// (0,0) を中心に回転させて (ox,oy) だけずらす
					px = ((x*sin.Cos(nAngle)-y*sin.Sin(nAngle))>>16)+ox;
					py = ((x*sin.Sin(nAngle)+y*sin.Cos(nAngle))>>16)+oy-height;
					lpDraw->BltFast(lpChip->bank[0]->m_lpPlane.get(),px,py,&r);
//					lpDraw->RotateBltFast(lpChip->bank[0]->m_lpPlane.get(),&r,px,py,512-nAngle,1<<16,4);
				}
			}
			MX++;
		}
//		3		:	下レイヤとブレンドされるレイヤ
		for(int i=3;i<=3;i++){
			int& p = m_nMapChara2[i];
			for(;p!=m_nMapChara[i];p++){
				SSprite* lpSS = &m_MapChara[i][p];
				int cry = lpSS->nOy;
				//	整順されているから、このチェックはこれで良い
				if (/* (y < cry) && */ (cry <= y+m_nMapCY)) {
					if (lpSS->bFast) {
						lpDraw->BltFast(lpSS->lpPlane,lpSS->nOx,lpSS->nOy + lpSS->nHeight,&lpSS->rcRect,NULL,&m_rcDraw);
					} else {
						lpDraw->Blt(lpSS->lpPlane,lpSS->nOx,lpSS->nOy + lpSS->nHeight,&lpSS->rcRect,NULL,&m_rcDraw);
					}
				} else {
					break;	//	もう嫌（笑）
				}
			}
		}

		MY++;
	}
	OnPaintLayer(lpDraw,4,6);
}

//	側面チップ
void	CFineMapLayer::OnPaintSide(CPlaneBase*lpDraw){
	OnPaintLayer(lpDraw,0,2);
//		0,1,2	:	下レイヤの下に描画
	OnSortLayer(3);

	// マップチップ位置算出
	int MX,MY,SX;	// マップチップ番号の算出
	int x=0,y=0;	//	忘れてたにょ:p
	WORD chip;		//	temporary
	int cx,cy;		//	cx,cy
	RECT r;			//	rect

	//	マイナスの場合、まずいが、そのチェックはしてあるので構わない
	SX = m_nX / m_nMapCX;
	MY = m_nY / m_nMapCY;

	int nSX,nSY;// マップのサイズ
	int nCX,nCY;// マップチップのサイズ
	int nCX2,nCY2;// 抜き色となる左上の三角形のサイズ
	nSX = m_nMapX;
	nSY = m_nMapY;
	nCX = m_nMapCX;
	nCY = m_nMapCY;
	nCX2= m_nMapCX2;
	nCY2= m_nMapCY2;

	//	マイナスの場合、まずいが、そのチェックはしてあるので構わない
	int sx,sy;// 表示領域のサイズ
	int ox,oy;// 表示差分
	int nAngle;// 視点
	sx = m_rcDraw.right;
	sy = m_rcDraw.bottom;
	ox = oy = 0;
	nAngle = 0;
	// 角度に応じて方向をかえる
	if((nAngle/2)==1) {
		oy = sy-1-nCY;// (0,0) のチップの表示位置
	}
	if(nAngle==1 || nAngle==2) {
		ox = sx-1-nCX;// (0,0) のチップの表示位置
	}
	if((nAngle%2)==1) {// 90°,270°なら縦横逆
		swap(sx,sy);
	}

	for(y = m_rcDraw.top-(m_nY % nCY) ; y<sy ; y+=nCY-nCY2){
		MX = SX;
		for(x = m_rcDraw.left -(m_nX % nCX) ; x<sx ; x+=nCX-nCX2){
			if (MX>=0 && MX<nSX && MY>=0 && MY<nSY){
				int nDir = GetAngle();// 方向番号
				CFineMapChip* lpChip = GetMapChip(MX,MY);
				CFineMapBank* lpBank = lpChip->sbank[nDir];// バンクナンバー
				if ((chip = lpChip->schip[nDir])!=0xffff){ // -1ならば配置されていない
					cx = (chip & 0xff);	// 下位8ビットがマップチップのＸ座標
					cy = (chip >> 8	 );

					WARNING (cx*nCX>=640 || cy*nCY>=480 , "CMapEditor::OnPaint2でマップのソースが範囲外");

					int w,h;
					lpBank->m_lpPlane->GetSize(w,h);
					int mx = w/nCX;// 横幅のレート計算
					::SetRect(&r,cx*nCX,cy*nCY+lpBank->m_awHeight[cx+cy*mx],(cx+1)*nCX,(cy+1)*nCY);

					// 高さを表示する高さ分ずらす
					int height = 0;
					if(IsShowHeight())height = lpChip->height;

					int nAngle = GetAngle()*128;// 回転角度
					int px,py;
					// (0,0) を中心に回転させて (ox,oy) だけずらす
					px = ((x*sin.Cos(nAngle)-y*sin.Sin(nAngle))>>16)+ox;
					py = ((x*sin.Sin(nAngle)+y*sin.Cos(nAngle))>>16)+oy-height;
					lpDraw->Blt(lpBank->m_lpPlane.get(),px,py,&r);
				}
			}
			MX++;
		}
		MY++;
	}
}

//	中レイヤ
void	CFineMapLayer::OnPaint2(CPlaneBase*lpDraw){
//		4,5,6	:	下レイヤより上、中レイヤより下に描画

	OnSortLayer(7,9);

	// マップチップ位置算出
	int MX,MY,SX;	// マップチップ番号の算出
	WORD chip;		//	temporary
	int cx,cy;		//	cx,cy
	RECT r;			//	rect

	//	マイナスの場合、まずいが、そのチェックはしてあるので構わない
	SX = m_nX / m_nMapCX;
	MY = m_nY / m_nMapCY;

	int nSX,nSY;// マップのサイズ
	int nCX,nCY;// マップチップのサイズ
	int nCX2,nCY2;// 抜き色となる左上の三角形のサイズ
	nSX = m_nMapX;
	nSY = m_nMapY;
	nCX = m_nMapCX;
	nCY = m_nMapCY;
	nCX2= m_nMapCX2;
	nCY2= m_nMapCY2;

	//	マイナスの場合、まずいが、そのチェックはしてあるので構わない
	int sx,sy;// 表示領域のサイズ
	int ox,oy;// 表示差分
	int nAngle;// 視点
	sx = m_rcDraw.right;
	sy = m_rcDraw.bottom;
	ox = oy = 0;
	nAngle = GetAngle();
	// 角度に応じて方向をかえる
	if((nAngle/2)==1) {
		oy = sy-1-nCY;// (0,0) のチップの表示位置
	}
	if(nAngle==1 || nAngle==2) {
		ox = sx-1-nCX;// (0,0) のチップの表示位置
	}
	if((nAngle%2)==1) {// 90°,270°なら縦横逆
		swap(sx,sy);
	}

	for(int y=0; y<sy; y+=nCY-nCY2){
		MX = SX;
		for(int x=0; x<sx; x+=nCX-nCX2){
			if (MX>=0 && MX<nSX && MY>=0 && MY<nSY){
				CFineMapChip* lpChip = GetMapChip(MX,MY);
				if ((chip = lpChip->chip[1])!=0xffff){ // -1ならば配置されていない
					cx = (chip & 0xff)*nCX;	// 下位8ビットがマップチップのＸ座標
					cy = (chip >> 8	 )*nCY;

					WARNING (cx>=640 || cy>=480 , "CMapEditor::OnPaint3でマップのソースが範囲外");

					::SetRect(&r,cx,cy,cx+nCX,cy+nCY);

					// 高さを表示する高さ分ずらす
					int height = 0;
					if(IsShowHeight())height = lpChip->height;

					int nAngle = GetAngle()*128;// 回転角度(上の nAngle と使い方が違う)
					int px,py;
					// (0,0) を中心に回転させて (ox,oy) だけずらす
					px = ((x*sin.Cos(nAngle)-y*sin.Sin(nAngle))>>16)+ox;
					py = ((x*sin.Sin(nAngle)+y*sin.Cos(nAngle))>>16)+oy-height;
					lpDraw->Blt(lpChip->bank[1]->m_lpPlane.get(),px,py,&r);
//					lpDraw->RotateBlt(lpChip->bank[1]->m_lpPlane.get(),&r,px,py,512-nAngle,1<<16,4);
				}
			}
			MX++;
		}
//		7,8,9	:	中レイヤとブレンドされるレイヤ
		for(int i=7;i<=9;i++){
			int& p = m_nMapChara2[i];
			for(;p!=m_nMapChara[i];p++){
				SSprite* lpSS = &m_MapChara[i][p];
				int cry = lpSS->nOy;
				//	整順されているから、このチェックはこれで良い
				if (/* (y < cry) && */ (cry <= y+m_nMapCY)) {
					if (lpSS->bFast) {
						lpDraw->BltFast(lpSS->lpPlane,lpSS->nOx,lpSS->nOy + lpSS->nHeight,&lpSS->rcRect,NULL,&m_rcDraw);
					} else {
						lpDraw->Blt(lpSS->lpPlane,lpSS->nOx,lpSS->nOy + lpSS->nHeight,&lpSS->rcRect,NULL,&m_rcDraw);
					}
				} else {
					break;	//	もう嫌（笑）
				}
			}
		}

		MY++;
	}

	for(int i=7;i<=9;i++){
		int& p = m_nMapChara2[i];
		for(;p!=m_nMapChara[i];p++){
			SSprite* lpSS = &m_MapChara[i][p];
			int cry = lpSS->nOy;
			//	整順されているから、このチェックはこれで良い
			if (/* (y < cry) && */ (cry <= y+m_nMapCY + 64 /* 64ドット下まで潜って調べる */)) {
				if (lpSS->bFast) {
					lpDraw->BltFast(lpSS->lpPlane,lpSS->nOx,lpSS->nOy + lpSS->nHeight,&lpSS->rcRect,NULL,&m_rcDraw);
				} else {
					lpDraw->Blt(lpSS->lpPlane,lpSS->nOx,lpSS->nOy + lpSS->nHeight,&lpSS->rcRect,NULL,&m_rcDraw);
				}
			} else {
				break;	//	もう嫌（笑）
			}
		}
	}

//		10		:	中レイヤより上、上レイヤより下
	OnPaintLayer(lpDraw,10);
}

//	上レイヤ
void	CFineMapLayer::OnPaint3(CPlaneBase*lpDraw){
	// マップチップ位置算出
	int MX,MY,SX;	// マップチップ番号の算出
	WORD chip;		//	temporary
	int cx,cy;		//	cx,cy
	RECT r;			//	rect

	//	マイナスの場合、まずいが、そのチェックはしてあるので構わない
	SX = m_nX / m_nMapCX;
	MY = m_nY / m_nMapCY;

	int nSX,nSY;// マップのサイズ
	int nCX,nCY;// マップチップのサイズ
	int nCX2,nCY2;// 抜き色となる左上の三角形のサイズ
	nSX = m_nMapX;
	nSY = m_nMapY;
	nCX = m_nMapCX;
	nCY = m_nMapCY;
	nCX2= m_nMapCX2;
	nCY2= m_nMapCY2;

	//	マイナスの場合、まずいが、そのチェックはしてあるので構わない
	int sx,sy;// 表示領域のサイズ
	int ox,oy;// 表示差分
	int nAngle;// 視点
	sx = m_rcDraw.right;
	sy = m_rcDraw.bottom;
	ox = oy = 0;
	nAngle = GetAngle();
	// 角度に応じて方向をかえる
	if((nAngle/2)==1) {
		oy = sy-1-nCY;// (0,0) のチップの表示位置
	}
	if(nAngle==1 || nAngle==2) {
		ox = sx-1-nCX;// (0,0) のチップの表示位置
	}
	if((nAngle%2)==1) {// 90°,270°なら縦横逆
		swap(sx,sy);
	}

	for(int y=0; y<sy; y+=nCY-nCY2){
		MX = SX;
		for(int x=0; x<sx; x+=nCX-nCX2){
			if (MX>=0 && MX<nSX && MY>=0 && MY<nSY){
				CFineMapChip* lpChip = GetMapChip(MX,MY);
				if ((chip = lpChip->chip[1])!=0xffff){ // -1ならば配置されていない
					cx = (chip & 0xff)*nCX;	// 下位8ビットがマップチップのＸ座標
					cy = (chip >> 8	 )*nCY;

					WARNING (cx>=640 || cy>=480 , "CMapEditor::OnPaint3でマップのソースが範囲外");

					::SetRect(&r,cx,cy,cx+nCX,cy+nCY);

					// 高さを表示する高さ分ずらす
					int height = 0;
					if(IsShowHeight())height = lpChip->height;

					int nAngle = GetAngle()*128;// 回転角度
					int px,py;
					// (0,0) を中心に回転させて (ox,oy) だけずらす
					px = ((x*sin.Cos(nAngle)-y*sin.Sin(nAngle))>>16)+ox;
					py = ((x*sin.Sin(nAngle)+y*sin.Cos(nAngle))>>16)+oy-height;
					lpDraw->Blt(lpChip->bank[2]->m_lpPlane.get(),px,py,&r);
//					lpDraw->RotateBlt(lpChip->bank[2]->m_lpPlane.get(),&r,px,py,512-nAngle,1<<16,4);
				}
			}
			MX++;
		}
	}

	OnPaintLayer(lpDraw,11);
//		11		:	上チップより上

	//	デバッグ用クリア矩形
	if (m_rcClear.left || m_rcClear.top ||
		m_rcClear.right || m_rcClear.bottom){
		RECT cl;
		::SetRect(&cl,m_rcClear.left-m_nX,m_rcClear.top-m_nY,
			m_rcClear.right-m_nX,m_rcClear.bottom-m_nY);
		lpDraw->ClearRect(&cl);
	}
}

//////////////////////////////////////////////////////////////////////////////

//	StartLayerからEndLayerまでをPaintする。
//	EndLayerは含む。ディフォルトではnEndLayer==nStartLayer
void	CFineMapLayer::OnPaintLayer(CPlaneBase*lpDraw,int nStartLayer,int nEndLayer){
	if (nEndLayer==-1) nEndLayer=nStartLayer;

	int ox = m_rcDraw.left	- m_nX;
	int oy = m_rcDraw.top	- m_nY;
	
	for (int nLayer=nStartLayer;nLayer<=nEndLayer;nLayer++){
		for(int i=0;i<m_nMapChara[nLayer];i++){
			SSprite* lpSS = &m_MapChara[nLayer][i];
			if (lpSS->bFast) {
				lpDraw->BltFast(lpSS->lpPlane,lpSS->nOx + ox,lpSS->nOy + lpSS->nHeight + oy,&lpSS->rcRect,NULL,&m_rcDraw);
			} else {
				lpDraw->Blt(lpSS->lpPlane,lpSS->nOx + ox,lpSS->nOy + lpSS->nHeight + oy,&lpSS->rcRect,NULL,&m_rcDraw);
			}
		}
	}
}

void	CFineMapLayer::OnSortLayer(int nStartLayer,int nEndLayer){
	if (nEndLayer==-1) nEndLayer=nStartLayer;

	//	ブレンド表示するとき前後関係がおかしくなるから立ち位置でソートする
	for(int nLayer=nStartLayer;nLayer<=nEndLayer;nLayer++){
		for(int i=0;i<m_nMapChara[nLayer];i++){
			m_MapChara[nLayer][i].nOx += (m_rcDraw.left - m_nX);

			//	あとでnHeight足す(C)yaneurao
			m_MapChara[nLayer][i].nOy += (m_rcDraw.top	- m_nY) /* - m_nMapCY */;
																//	位置補正項
			m_MapChara[nLayer][i].nHeight += 0 /* m_nMapCY*/ ;
		}
		::qsort((LPVOID)&m_MapChara[nLayer][0],m_nMapChara[nLayer],sizeof(SSprite),map_compare);
	}
}

//////////////////////////////////////////////////////////////////////////////

void	CFineMapLayer::SetPos(int x,int y){			//	表示エリア左上の来るべき仮想スクリーン座標
	m_nX	=	x;
	m_nY	=	y;
}

void	CFineMapLayer::GetPos(int &x,int &y){			//	表示エリア左上の来るべき仮想スクリーン座標
	x		=	m_nX;
	y		=	m_nY;
}

void	CFineMapLayer::SetView(LPRECT lpRect){		//	描画エリア
	if (lpRect==NULL) return ;
	m_rcDraw = *lpRect;
}

LPRECT	CFineMapLayer::GetView(void){				//	↑の取得
	return &m_rcDraw;
}

void	CFineMapLayer::Release(void){
	//	マップと従属プレーンの解放
	m_avMapData.clear();
	m_avMapBank.clear();
}

LRESULT	CFineMapLayer::Load(string szFileName){		//	マップの読み込み
	Release();
	SetPos(0,0);	//	一応リセットしておく

	CFineMapIO map;
	if (map.Read(szFileName)!=0) return 1;

	CMap* lpMap = map.GetMap();
	m_nMapX	 =lpMap->m_vMapHeader.wdMapX;
	m_nMapY	 =lpMap->m_vMapHeader.wdMapY;
	m_nMapCX = lpMap->m_vMapHeader.wdMapCX;
	m_nMapCY = lpMap->m_vMapHeader.wdMapCY;
	m_nMapCX2 = lpMap->m_vMapHeader.wdMapCX2;
	m_nMapCY2 = lpMap->m_vMapHeader.wdMapCY2;

	SetMapSize(m_nMapX,m_nMapY);

	// プレーンを読み出す＆チップ高さ計算
	vector<WORD> &awdBankNo = lpMap->m_vMapHeader.awdBankNo;
	m_avMapBank.clear();// まずクリア
	for(int i=0;i<awdBankNo.size();i++){
		CHAR szFile[256];
		int nBank = awdBankNo[i];
		CFineMapBank bank;

		::wsprintf(szFile,"bank/bank%.3d.bmp",nBank);
		smart_ptr<CPlaneBase> lp(CPlaneBase::CreatePlane());
		bank.m_lpPlane.Add(lp);
		bank.m_lpPlane->Load(szFile);

		/************************************************/
		/*			サイドチップ高さを調べる			*/
		/************************************************/
		int mx,my,sx,sy;
		bank.m_lpPlane->GetSize(sx,sy);
		mx = sx/m_nMapCX;
		my = sy/m_nMapCY;

		for(int dy=0; dy<my; ++dy) {// バンクプレーン全体のチップ数分
			for(int dx=0; dx<mx; ++dx) {
				for(int y=dy*m_nMapCY; y<(dy+1)*m_nMapCY; ++y) {// １チップ分調べる
					bool bOmit = true;// 抜き色か
					for(int x=dx*m_nMapCX; x<(dx+1)*m_nMapCX; ++x) {
						if(bank.m_lpPlane->GetPixelAlpha(x,y)!=0) {// 0 以外なら抜き色じゃない
							bOmit = false;
							break;
						}
					}
					if(!bOmit)break;// 抜き色でないならそこまで
				}
				bank.m_awHeight.push_back(y-dy*m_nMapCY);// その y 座標のズレが高さ
			}
		}
		m_avMapBank.push_back(bank);// 出来たバンクを追加
	}

	// マップデータコピー
	for(int y=0;y<m_nMapY;y++){
		for(int x=0;x<m_nMapX;x++){
			int nPos = x + y*m_nMapX;
			m_avMapData[nPos].chip[0] = lpMap->m_avMapBody[nPos].wdDownchip;
			m_avMapData[nPos].chip[1] = lpMap->m_avMapBody[nPos].wdMiddlechip;
			m_avMapData[nPos].chip[2] = lpMap->m_avMapBody[nPos].wdUpperchip;
			//	プレーンへのポインタも事前に代入
			m_avMapData[nPos].bank[0] = &m_avMapBank[lpMap->m_avMapBody[nPos].abyBankNo[0]];
			m_avMapData[nPos].bank[1] = &m_avMapBank[lpMap->m_avMapBody[nPos].abyBankNo[1]];
			m_avMapData[nPos].bank[2] = &m_avMapBank[lpMap->m_avMapBody[nPos].abyBankNo[2]];

			m_avMapData[nPos].schip[0] = lpMap->m_avMapBody[nPos].awdSidechip[0];
			m_avMapData[nPos].schip[1] = lpMap->m_avMapBody[nPos].awdSidechip[1];
			m_avMapData[nPos].schip[2] = lpMap->m_avMapBody[nPos].awdSidechip[2];
			m_avMapData[nPos].schip[3] = lpMap->m_avMapBody[nPos].awdSidechip[3];

			m_avMapData[nPos].sbank[0] = &m_avMapBank[lpMap->m_avMapBody[nPos].abySideBankNo[0]];
			m_avMapData[nPos].sbank[1] = &m_avMapBank[lpMap->m_avMapBody[nPos].abySideBankNo[1]];
			m_avMapData[nPos].sbank[2] = &m_avMapBank[lpMap->m_avMapBody[nPos].abySideBankNo[2]];
			m_avMapData[nPos].sbank[3] = &m_avMapBank[lpMap->m_avMapBody[nPos].abySideBankNo[3]];

			//	床判定もこいつが担う
			m_avMapData[nPos].hit		 = lpMap->m_avMapBody[nPos].byHit;
		}
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////

bool	CFineMapLayer::IsWall(int x,int y){			//	(x,y)は床か？
	if (m_bInvalid) return false;
	
	WARNING(m_avMapData.size()==0,"CFineMapLayer::IsWallでm_avMapData.size()==0");
	if (x<0 || y<0 || x>=m_nMapX || y>=m_nMapY) return true;
	return (m_avMapData[x + y*m_nMapX].hit /* & MAP_ALL_WALL */ ) != 0;	
}

DWORD*	CFineMapLayer::GetWall(int x,int y){
	WARNING(m_avMapData.size()==0,"CFineMapLayer::IsWallでm_avMapData.size()==0");

	//	(x,y)の床情報へのポインタを直接得る
	static DWORD dummy;
	if (m_bInvalid) {
		dummy = 0x00000000;	//	ウソのポインタを用意
		return &dummy;
	}
	
	if (x<0 || y<0 || x>=m_nMapX || y>=m_nMapY) {
		dummy = 0xffffffff;	//	ウソのポインタを用意
		return &dummy;
	}
	return &m_avMapData[x + y*m_nMapX].hit;
}

//////////////////////////////////////////////////////////////////////////////

void	CFineMapLayer::BltFix(CSprite*lpSprite,int x,int y){
	//	(x,y)に描画
	
	//	有効か？
	if (!lpSprite->IsEnable()) return ;

	//	レイヤ取得
	int	nLayer = lpSprite->GetLayer();

	SSprite* lpSS = lpSprite->GetSSprite();

	WARNING(nLayer<0||nLayer>=MAPLAYER_MAX,"CFineMapLayer::BltFixでLayer違反");
	int& nCount = m_nMapChara[nLayer];
	WARNING(nCount==MAPCHARA_MAX,"CFineMapLayer::BltFixでキャラオーバー");
	SSprite& mc = m_MapChara[nLayer][nCount];

	mc = *lpSS;
	int ox,oy;
	lpSprite->GetOffset(ox,oy);
	mc.nOx	+=	x+ox;
	//	あとでnHeight足す(C)yaneurao
	int t = mc.nOy - lpSprite->GetHeight();
	mc.nOy	=	y + oy + lpSprite->GetHeight();
	mc.nHeight = t;
	
	nCount++;
}

void	CFineMapLayer::BltOnce(CSprite*lpSprite,int x,int y){
	BltFix(lpSprite,x,y);
	
	//	ケツになっていたら、それ以上は加算しない
	int n=lpSprite->GetMotion();
	lpSprite->IncMotion();
	if (lpSprite->GetMotion()==0) {
		lpSprite->SetMotion(n);
	}
}

//////////////////////////////////////////////////////////////////////////////

