#include "stdafx.h"
#include "CFineMapIO.h"

// CMapIO からほとんどコピペ(^^;

LRESULT	CFineMapIO::Write(string filename){
	CSerialize s;
	s.SetStoring(true);
	s << m_vMap;
	if(s.Save(filename))return 1;
	return 0;
}

LRESULT	CFineMapIO::Read(string filename){
	CSerialize s;
	if(s.Load(filename)!=0)return 0;
	s.SetStoring(false);
	s << m_vMap;

	if (m_vMap.m_vMapHeader.dwTitle != 0x544D4134) {	// これちゃうやつやんか！
		Release();	// こうしておかないと、間違ってdeleteする
		return 2;
	}

	return 0;
}

void	CFineMapIO::Create(int x,int y){
	Release();
	m_vMap.m_vMapHeader.dwTitle = 0x544D4134; // TMA4
	m_vMap.m_vMapHeader.wdMapX = x;
	m_vMap.m_vMapHeader.wdMapY = y;
	m_vMap.m_vMapHeader.wdMapCX = 16; // とりあえずだよ
	m_vMap.m_vMapHeader.wdMapCY = 16;
	m_vMap.m_vMapHeader.wdMapCX2 = 0;// デフォルトは 0
	m_vMap.m_vMapHeader.wdMapCY2 = 0;
	m_vMap.m_vMapHeader.awdBankNo.clear();

	m_vMap.m_avMapBody.resize(x*y);// サイズ分確保

	int n = m_vMap.m_avMapBody.size();
	//	高さはリセットしとこかー
	for(int j=0;j<y;j++){
		for(int i=0;i<x;i++){
			m_vMap.m_avMapBody[i+j*x].wdHeight	= 0;
			m_vMap.m_avMapBody[i+j*x].abyBankNo[0]= 0;
			m_vMap.m_avMapBody[i+j*x].abyBankNo[1]= 0;
			m_vMap.m_avMapBody[i+j*x].abyBankNo[2]= 0;

			m_vMap.m_avMapBody[i+j*x].awdSidechip[0]= -1;
			m_vMap.m_avMapBody[i+j*x].awdSidechip[1]= -1;
			m_vMap.m_avMapBody[i+j*x].awdSidechip[2]= -1;
			m_vMap.m_avMapBody[i+j*x].awdSidechip[3]= -1;
			m_vMap.m_avMapBody[i+j*x].abySideBankNo[0]= 0;
			m_vMap.m_avMapBody[i+j*x].abySideBankNo[1]= 0;
			m_vMap.m_avMapBody[i+j*x].abySideBankNo[2]= 0;
			m_vMap.m_avMapBody[i+j*x].abySideBankNo[3]= 0;
			m_vMap.m_avMapBody[i+j*x].awdReserved[0]= 0;
			m_vMap.m_avMapBody[i+j*x].awdReserved[1]= 0;

			m_vMap.m_avMapBody[i+j*x].wdDownchip	= -1;
			m_vMap.m_avMapBody[i+j*x].wdMiddlechip	= -1;
			m_vMap.m_avMapBody[i+j*x].wdUpperchip	= -1;
			m_vMap.m_avMapBody[i+j*x].byHit = 0;
		}
	}
}

void CFineMapIO::Release(void){
	// vector は解放しとこう
	m_vMap.m_vMapHeader.awdBankNo.clear();
	m_vMap.m_avMapBody.clear();
}

CFineMapIO::CFineMapIO(void){
}

CFineMapIO::~CFineMapIO(){
	Release();
}

//////////////////////////////////////////////////////////////////////////////

LRESULT CFineMapIO::Resize(int x,int y){
// 右と下を削るようなResize

	vector<CFineMapChipBase>	avMapBody;// マップチップ
	avMapBody.resize(x*y);// リサイズ

	// 出来る限り、コピー
	for(int j=0;j<y;j++){
		for(int i=0;i<x;i++){
			CFineMapChipBase &mc = avMapBody[i+j*x];
//			if (i<m_vMap.m_vMapHeader.wdMapCX && j<m_vMap.m_vMapHeader.wdMapCY) {
			if (i<GetMap()->m_vMapHeader.wdMapX && j<GetMap()->m_vMapHeader.wdMapY) {	//	fixed '00/01/03
//				lpTMap->m_avMapBody[i + j*x] = m_vMap.m_avMapBody[i + j*x];
				mc = GetMap()->m_avMapBody[i+j*GetMap()->m_vMapHeader.wdMapX];// コピー
			} else {
				mc.wdDownchip	= -1; // unsignedに-1で良いのか？
				mc.wdMiddlechip	= -1;
				mc.wdUpperchip	= -1;
				mc.wdHeight		= 0;

				mc.awdSidechip[0]= -1;
				mc.awdSidechip[1]= -1;
				mc.awdSidechip[2]= -1;
				mc.awdSidechip[3]= -1;
				mc.abySideBankNo[0]= 0;
				mc.abySideBankNo[1]= 0;
				mc.abySideBankNo[2]= 0;
				mc.abySideBankNo[3]= 0;
				mc.awdReserved[0]= 0;
				mc.awdReserved[1]= 0;

				mc.byHit		= 0;
				mc.abyBankNo[0]	= 0;
				mc.abyBankNo[1]	= 0;
				mc.abyBankNo[2]	= 0;
			}
		}
	}

	// マップサイズは変わった
	GetMap()->m_vMapHeader.wdMapX = x;
	GetMap()->m_vMapHeader.wdMapY = y;

	GetMap()->m_avMapBody = avMapBody;// vector なのでコピーでいいっしょ
	return 0;
}

LRESULT	CFineMapIO::Resize(int cx,int cy,int vx,int vy){ // 複雑なResize
//	if (m_lpTMap==NULL) return 1; // よぶなっちゅーに！

	// 現在のマップをベクトル(vx,vy)だけリサイズして、(cx,cy)方向にコピー！
	int mapx,mapy;
	mapx = m_vMap.m_vMapHeader.wdMapX;
	mapy = m_vMap.m_vMapHeader.wdMapY;
	mapx += vx;
	mapy += vy;
	if (mapx <= 0 || mapy <= 0) return 1; // そんなResizeでけへんがな

	vector<CFineMapChipBase>	avMapBody;// マップチップ
	avMapBody.resize(mapx*mapy);// リサイズ

	// 出来る限り、コピー
	int i,j;
	for(j=0;j<mapy;j++){
		for(i=0;i<mapx;i++){
			CFineMapChipBase &mc = avMapBody[i+j*mapx];
			mc.wdDownchip	= -1; // unsignedに-1で良いのか？
			mc.wdMiddlechip	= -1;
			mc.wdUpperchip	= -1;
			mc.wdHeight		= 0;

			mc.awdSidechip[0]= -1;
			mc.awdSidechip[1]= -1;
			mc.awdSidechip[2]= -1;
			mc.awdSidechip[3]= -1;
			mc.abySideBankNo[0]= 0;
			mc.abySideBankNo[1]= 0;
			mc.abySideBankNo[2]= 0;
			mc.abySideBankNo[3]= 0;
			mc.awdReserved[0]= 0;
			mc.awdReserved[1]= 0;

			mc.byHit		= 0;
			mc.abyBankNo[0]	= 0;
			mc.abyBankNo[1]	= 0;
			mc.abyBankNo[2]	= 0;
		}
	}
	int mapxx,mapyy;
	for (j=0;j<GetMap()->m_vMapHeader.wdMapY;j++){
		for(i=0;i<GetMap()->m_vMapHeader.wdMapX;i++){
			mapxx = i + cx;	// 移動変換先が範囲内ならばコピー！
			mapyy = j + cy;
			if (mapxx>=0 && mapyy >=0 &&
				mapxx<mapx && mapyy < mapy) {
				avMapBody[mapxx + mapyy*mapx] =
					GetMap()->m_avMapBody[i + j*GetMap()->m_vMapHeader.wdMapX];
			}
		}
	}
	GetMap()->m_avMapBody = avMapBody;

	// マップサイズは変わった
	GetMap()->m_vMapHeader.wdMapX = mapx;
	GetMap()->m_vMapHeader.wdMapY = mapy;

	return 0;
}

