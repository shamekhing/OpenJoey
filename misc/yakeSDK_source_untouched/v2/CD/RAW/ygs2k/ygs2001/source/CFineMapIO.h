
#ifndef _CFineMapIO_H
#define _CFineMapIO_H

#include "../../yaneSDK/yaneSDK.h"

class CFineMapChipBase : public CArchive {
public:
	WORD wdDownchip;	// 下にあるチップ
	WORD wdMiddlechip;	// 真ん中にあるチップ
	WORD wdUpperchip;	// 上にあるチップ
	WORD wdHeight;		// 高さを持っている
	BYTE byHit;			// 接触判定(on or off)
	BYTE abyBankNo[3];	// 各チップのバンクナンバー

	// 拡張情報
	WORD awdSidechip[4];	// 側面チップ(それぞれ 0°,90°,180°,270°)
	BYTE abySideBankNo[4];	// 側面のバンクナンバー
	WORD awdReserved[2];	// 予約領域(常に 0)

	void Serialize(CSerialize &s) {
		s << wdDownchip;
		s << wdMiddlechip;
		s << wdUpperchip;
		s << wdHeight;
		s.Store(awdSidechip,4);
		s.Store(abySideBankNo,4);
		s.Store(awdReserved,2);
		s << byHit;
		s.Store(abyBankNo,3);
	}

	CFineMapChipBase(){}
	virtual ~CFineMapChipBase(){}
};

class CFineMapHeader : public CArchive {
public:
	DWORD dwTitle; // "TMA4" ←これにしてください。
	vector<WORD> awdBankNo; // バンクナンバー
	// バンクナンバーが1番ならば bankファイル名は"bank001.bmp"
	WORD wdMapX; // X方向のチップ数
	WORD wdMapY; // Y方向のチップ数
	WORD wdMapCX; // X方向のマップチップサイズ
	WORD wdMapCY; // Y方向のマップチップサイズ
	WORD wdFeature; // 拡張情報の数(reserved)

	// 拡張情報
	WORD wdMapCX2;// 抜き色となる左上の三角形の横幅
	WORD wdMapCY2;// 抜き色となる左上の三角形の縦幅

	void Serialize(CSerialize &s) {
		s << dwTitle;
		s << awdBankNo;
		s << wdMapX;
		s << wdMapY;
		s << wdMapCX;
		s << wdMapCY;
		s << wdMapCX2;
		s << wdMapCY2;
		s << wdFeature;
	}

	CFineMapHeader(){}
	virtual ~CFineMapHeader(){}
};

class CMap :public CArchive {
public:
	CFineMapHeader				m_vMapHeader;
	vector<CFineMapChipBase>	m_avMapBody;

	void Serialize(CSerialize &s) {
		s << m_vMapHeader;

		// マップチップの保存
		int n;
		if (s.IsStoring()) {
			n = m_avMapBody.size();
			s << n;	//	サイズもついでに保存しなくては！
		} else {
			s << n;	//	長さを復元
			m_avMapBody.resize(n);
		}
		for(int i=0;i<n;i++){
			s << m_avMapBody[i];
		}
	}

	CMap(){}
	virtual ~CMap(){}
};

class CFineMapIO {
public:
	LRESULT	Write(string filename);	// そのまんま書き込み
	LRESULT	Read(string filename);	// そのまんま読み込み
	void	Create(int x,int y);	// サイズ(x,y)のマップを用意する

	CMap*	GetMap(void) { return &m_vMap; }	// こいつを介してアクセスして！
	LRESULT	Resize(int x,int y);				// マップのResize
	LRESULT	Resize(int cx,int cy,int vx,int vy); // 複雑なResize

	CFineMapIO();
	virtual ~CFineMapIO();

protected:
	void	Release(void);
	CMap	m_vMap;			// マップ
};

#endif

