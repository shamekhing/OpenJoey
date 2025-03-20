//
//	yaneSurface.h
//		サーフェース基底クラス
//
//	yaneSurface1.cpp … 基本実装
//	yaneSurface2.cpp … 通常Blt実装
//	yaneSurface3.cpp … ブレンド系Blt実装
//	yaneSurface4.cpp … 加色減色系Blt実装
//	yaneSurface5.cpp … アフィン変換系Blt実装

#ifndef __yaneSurface_h__
#define __yaneSurface_h__

class ISurfaceLocker {
/**
	SurfaceをLockする機構をポリモーフィックに提供する

	class CSurfaceLockerGuard も参照のこと。
*/
public:
	virtual LRESULT Lock() = 0;
	virtual LRESULT Unlock() = 0;
	virtual ~ISurfaceLocker(){}
};

class INullSurfaceLocker : public ISurfaceLocker {
///	class ISurfaceLocker の Null Device
public:
	virtual LRESULT Lock() { return 0; }
	virtual LRESULT Unlock() { return 0; }
};

class CSurfaceLockerGuard {
/**
	デストラクタでUnlockを呼び出すだけのオブジェクト
	CSurfaceInfo::GetPixel で実際に使っているので、そちらも
	参照のこと。
*/
public:
	CSurfaceLockerGuard(ISurfaceLocker*pLocker)
		: m_pSurfaceLocker(pLocker) {}
	CSurfaceLockerGuard(const smart_ptr<ISurfaceLocker>& vLocker)
		: m_pSurfaceLocker(vLocker.get()) {}
	~CSurfaceLockerGuard(){ m_pSurfaceLocker->Unlock(); }
private:
	ISurfaceLocker* m_pSurfaceLocker;
};

///	ISurfaceRGBはDWORDっちゅーことで
typedef DWORD ISurfaceRGB;

class ISurface;
class CSurfaceInfo : public ISurfaceLocker {
/**
	Lockしたときのサーフェース情報を保持するための構造体
	一応、class CSurfaceLockerGuard が使えるように、
	このクラスは、ISurfaceLockerから派生させておく。
*/
public:

	////////////////////////////////////////////////////
	//	初期化関数
	////////////////////////////////////////////////////
	void	Init(void* lpSurfacePtr,LONG lPitch,const SIZE &size
		,int nSurfaceType = 0) {
		m_lpSurface = lpSurfacePtr; m_lPitch = lPitch; m_size = size,
		m_bInit = true, m_nSurfaceType = nSurfaceType;
	}
	/**
		ここで与えるものは、以下のもの。（あとでも個別に設定できる）
		さらに、lock～unlockに際して、さらに処理が必要ならば
		SetLockerで、それを設定すること。
	
		lpSurfacePtr	:	サーフェースのイメージへのポインタ
			サーフェースのイメージの左上の座標のメモリアドレス
		lPitch			:	１ラインのバイト数
		size			:	Surfaceのサイズ
		nSurfaceType	:	サーフェースの種類

		0:	未調査（OnChangeSurfaceがまだ呼び出されていない)
		1:	不明（以下の以外）

		//	通常のサーフェース
		2:	8bpp
		3:	16(RGB565)
		4:	16(RGB555)
		5:	24(RGB888)
		6:	24(BGR888)
		7:	32(XRGB8888)
		8:	32(XBGR8888)

		//	αサーフェース
		10:	16+a4(ARGB4565)
		11:	16+a4(ARGB4555)
		12:	32(ARGB8888)
		13:	32(ABGR8888)
	*/

	bool	IsInit() const { return m_bInit; }
	void	SetInit(bool b) { m_bInit = b; }

	////////////////////////////////////////////////////
	//	property...
	////////////////////////////////////////////////////

	///	サーフェースの左上のポインタの設定／取得
	void*	GetPtr() const { return m_lpSurface; }
	void	SetPtr(void*lpSurface){ m_lpSurface=lpSurface; }

	///	サーフェースのラスタピッチ
	LONG	GetPitch() const { return m_lPitch; }
	void	SetPitch(LONG lPitch) { m_lPitch = lPitch; }

	///	サーフェースの広さを示す矩形設定／取得
	SIZE	GetSize() const { return m_size; }
	void	SetSize(const SIZE& size) { m_size = size; }

	///	ビデオメモリ上のDirectDrawSurfaceをLockしとるんか？
	///	（システムメモリ上のDirectDrawSurfaceならば、
	///	これを無視して、直接読み書きしてまう！）
	bool	IsLocked() const { return m_bLock; }
		///	lockされているか？
	void	SetLock(bool bLock) { m_bLock = bLock; }
		///	lockされているかの情報を設定する
	///	Lock～Unlockする
	///	(２重ロックすると、CRuntimeExceptionが発生)
	virtual LRESULT Lock()
		#ifdef USE_EXCEPTION
			throw(CRuntimeException)
		#endif
			;
	virtual LRESULT Unlock()
		#ifdef USE_EXCEPTION
			throw(CRuntimeException)
		#endif
			;

	/**
		与えられた矩形を、このサーフェースでクリップして、残る矩形を返す
	*/
	RECT	GetClipRect(const LPRECT lpRect) const;

	///	SurfaceTypeを取得
	int		GetSurfaceType() const { return m_nSurfaceType; }
	void	SetSurfaceType(int n) { m_nSurfaceType = n; }

	///	SurfaceのLockerのSet/Reset
	smart_ptr<ISurfaceLocker> GetLocker() const { return m_vLocker; }
	void SetLocker(const smart_ptr<ISurfaceLocker>& locker) { m_vLocker = locker; }

	///	カラーキーの設定／取得(このサーフェース用にピクセルフォーマットを変換したもの)
	void	SetColorKey(DWORD dw){ m_dwColorKey = dw; }
	DWORD	GetColorKey() const { return m_dwColorKey; }

	///	FillColorの設定／取得(このサーフェース用にピクセルフォーマットを変換したもの)
	void	SetFillColor(DWORD dw) { m_dwFillColor = dw; }
	DWORD	GetFillColor() const { return m_dwFillColor; }
	
	CSurfaceInfo(){
		m_bInit			= false; // 初期化されてまへんInit()してくらはい
		m_bLock			= false;
		m_lpSurface		= NULL;
		m_nSurfaceType	=	0;
		SetLocker(smart_ptr<ISurfaceLocker>(new INullSurfaceLocker));
		//	null lockerをdefaultで
		m_dwColorKey	=	0;
		m_dwFillColor	=	0;
	}

	///	一般的なBltter & Effector

	///	--- 汎用ピクセル操作(遅い)
	LRESULT GetPixel(int x,int y,ISurfaceRGB&rgba) const;
	/**
		rgbaで返る。非αサーフェースの場合、rgbaのaは0が返る。
		サーフェース範囲外のときは、非0が返る
	*/

	LRESULT	GetMatchColor(ISurfaceRGB rgb,DWORD& dw) const;
	/**
		ある色が、そのサーフェースでどう表現されるかを返す
		非αサーフェースの場合、rgbのα値は無視される

		//	抜き色等は、こいつでISurfaceRGBから変換すると良い
	*/

	struct CBltInfo {
	POINT*		pDstPoint;		///	転送先座標(pDstPoint==NULLならば左上)
	SIZE*		pDstSize;		///	転送先サイズ(pSize==NULLならば等倍)
	RECT*		pSrcRect;		///	転送元座標(pSrcPoint==NULLならばサーフェース全域)
	RECT*		pDstClip;		///	転送先のクリップ
									///	(この範囲からはみ出る範囲には描画されない)
	int			nBasePoint;	// todo
	/**
		（pSrcPointで指定しているのは、転送元の、どの点なのか？）
		0:左上 1:真上 2:右上
		3:真左 4:中央 5:真右
		6:左下 7:真下 8:右下
	*/
	CBltInfo(
		POINT*		_pDstPoint=NULL,
		SIZE*		_pDstSize=NULL,
		RECT*		_pSrcRect=NULL,
		RECT*		_pDstClip=NULL,
		int			_nBasePoint=0
		) : pDstPoint(_pDstPoint),pDstSize(_pDstSize),pSrcRect(_pSrcRect),
			pDstClip(_pDstClip),nBasePoint(_nBasePoint){}
	};

	enum EBltType {
	///	転送の種類
		eSurfaceBltFast	,		///	通常Blt。抜き色無し
		eSurfaceBlt		,		///	通常Blt。抜き色あり
	};

	enum EEffectType {
	///	転送の種類
		eSurfaceEffectFill	,		///	塗りつぶし
		eSurfaceEffectFade	,		///	通常Fade
		eSurfaceEffectAddColor,		///	加色合成
		eSurfaceEffectSubColor,		///	減色合成
	};

	/**
		ISurfaceの同名の関数の下請け関数

		これで、BltとEffectを定義してあるので、派生クラスで
		ハードウェアアクセラレータによって転送するのでなければ、
		こいつに委譲するだけで転送が完了する。
	*/
	LRESULT GeneralBlt(EBltType type,CSurfaceInfo*pSrc,CBltInfo* pInfo,DWORD*pAdditionalParameter=NULL);
	LRESULT GeneralEffect(EEffectType type,LPRECT prc=NULL,DWORD*pAdditionalParameter=NULL);

private:
	bool	m_bInit;			//	初期化済みか？
	void*	m_lpSurface;		//	このサーフェースの左上のピクセルのポインタ
	LONG	m_lPitch;			//	このサーフェースのピッチ
								//	（次のラスターまでのバイト数）
	SIZE	m_size;				//	このサーフェースのサイズ
	bool	m_bLock;			//	ロック中か？
	int		m_nSurfaceType;		//	サーフェースの種類もここに保持しておこう

	///	(以下は、このサーフェース用にピクセルフォーマットを変換したもの)
	DWORD	m_dwColorKey;		//	このサーフェースのカラーキー
	DWORD	m_dwFillColor;		//	このサーフェースの塗りつぶし時に使う色

	smart_ptr<ISurfaceLocker> m_vLocker;	//	surfaceをlockするためのオブジェクト
};

class ISurface {
public:
	///	typedef DWORD ISurfaceRGB;
	///	このサーフェースでは、共通して、このRGB値で指定する

	static inline ISurfaceRGB makeRGB(DWORD r,DWORD g,DWORD b,DWORD a=0)
		#ifdef USE_EXCEPTION
			throw(CRuntimeException)
		#endif
	/**
		IPlaneで使うためのRGBピクセル値の生成
		：	↑これは、XRGB8888(DIB32と仕様は同じ)の生成
	*/
	{
		if (r>=256 || g>=256 || b>=256 || a>=256){
		#ifdef USE_EXCEPTION
			throw CRuntimeException();
		#else
			r = g = b = a = 0;
		#endif
		}
		return (a<<24) | (r<<16) | (g<<8) | b ;
	}

	static inline void	getRGB(ISurfaceRGB rgb,BYTE& a,BYTE& r,BYTE& g,BYTE& b)
	/**
		IPlaneで使うためのRGBピクセル値の分解
		：	ARGB8888(DIB32と仕様は同じ)と同じ
	*/
	{
		a = (rgb >> 24) & 0xff;
		r = (rgb >> 16) & 0xff;
		g = (rgb >>	 8) & 0xff;
		b = (rgb	  ) & 0xff;
	}

	virtual int GetType() const = 0;
	/**
		サーフェースのタイプを返す(RTTIもどき)
		0:	Null Device
		1:	CFastPlane
		2:	CDIB32
	*/

	//////////////////////////////////////////////////////////////////////
	///	汎用Bltter
	//////////////////////////////////////////////////////////////////////

	virtual LRESULT GeneralBlt(CSurfaceInfo::EBltType type,CSurfaceInfo*pSrc,CSurfaceInfo::CBltInfo* pInfo,DWORD*pAdditionalParameter=NULL)=0;
	///	汎化されたBltter
	/**
		type : 転送モード(抜き色転送であるだとか、加色合成であるだとか)
		pSrc : 転送元のCSurfaceInfo
		pInfo: 転送パラメータ
		pAdditionalParameter : 追加で必要となるパラメータへのポインタ
	*/

	/// ---- 基本的なBltterは事前に用意しておく
	LRESULT Blt(ISurface*pSrc,int x=0,int y=0,SIZE*pDstSize=NULL,
		RECT* pSrcRect=NULL,RECT*	pDstClip=NULL,int nBasePoint=0){
	/**
		抜き色有り転送
		x,y		 : 転送先座標
		pDstSize : 転送先サイズ(NULLならば等倍)
		pSrcRect : 転送元矩形
		pDstClip : 転送先クリッピング矩形(この範囲には描画されない)
		nBasePoint:転送先基準位置

		//  class CSurfaceInfo::CBltInfo も参照のこと。
  */
		POINT DstPoint = { x , y };
		CSurfaceInfo::CBltInfo info(&DstPoint,pDstSize,pSrcRect,pDstClip,nBasePoint);
		return GeneralBlt(CSurfaceInfo::eSurfaceBlt,pSrc->GetSurfaceInfo(),&info);
	}

	///	抜き色無し転送
	LRESULT BltFast(ISurface*pSrc,int x=0,int y=0,SIZE*pDstSize=NULL,
		RECT* pSrcRect=NULL,RECT*	pDstClip=NULL,int nBasePoint=0){
	///	パラメータの意味は、Bltと同じ
		POINT DstPoint = { x , y };
		CSurfaceInfo::CBltInfo info(&DstPoint,pDstSize,pSrcRect,pDstClip,nBasePoint);
		return GeneralBlt(CSurfaceInfo::eSurfaceBltFast,pSrc->GetSurfaceInfo(),&info);
	}

	//////////////////////////////////////////////////////////////////////////
	///	--- Surfaceのエフェクタ

	virtual LRESULT GeneralEffect(CSurfaceInfo::EEffectType type,LPRECT prc=NULL,DWORD*pAdditionalParameter=NULL)=0;
	///	汎化されたEffector
	/**
		type : 転送モード(通常Fade転送であるだとか、加色合成であるだとか)
		prc	 : エフェクトをかける矩形範囲(NULLならば全域)
		pAdditionalParameter : 追加で必要となるパラメータへのポインタ
			type : eEffectFade ならば、[0]は Fade率(BYTE:0-255)
			type : eAddColor,eSubColor ならば[0]はその定数(ISurfaceRGB)
	*/

	//////////////////////////////////////////////////////////////////////////
	///	--- Surfaceとして、常識的に必要なもの

	///	--- 抜き色の設定／取得
	virtual	LRESULT SetColorKey(ISurfaceRGB rgb)=0;
	virtual ISurfaceRGB GetColorKey() const=0;

	virtual LRESULT SetColorKeyPos(int x,int y)=0;
	///　(x,y)の点を透過キーに設定する

	virtual LRESULT SetColorKeyRGB(int r,int g,int b){ return SetColorKey(makeRGB(r,g,b)); }
	///	RGBで抜き色を指定する。rgbはそれぞれ0～255

	///	--- サーフェースのサイズ設定／取得
	virtual LRESULT GetSize(int& x,int& y) const =0;
	///		未生成のサーフェースならばGetSizeは、-1を返す（そのときx==y==0）
	virtual LRESULT SetSize(int x,int y)=0;
	/**
		SetSizeは、もし現在サイズと違うのであれば、あらたにCreateされる
	*/

	///		サーフェースの種類を返す
	virtual int GetSurfaceType() const
	{ return const_cast<ISurface*>(this)->GetSurfaceInfo()->GetSurfaceType(); }

	///////////////////////////////////////////////////////

	virtual LRESULT		Clear(LPRECT lpRect=NULL)=0;		///	矩形クリア
	virtual LRESULT		SetFillColor(ISurfaceRGB c)=0;		///	Clearする色を指定する(Default==RGB(0,0,0))
	virtual ISurfaceRGB	GetFillColor() const=0;				///	Clearする色の取得
		///	⇒ただし、α付きサーフェースに対するClearは、
		///		α == 0になることが保証されるものとする

	///////////////////////////////////////////////////////

	///　--- ビットマップ関連
	virtual LRESULT Load(const string& szBitmapFileName){
		///	画像の読み込み
		return 0;
	}
	virtual LRESULT Save(const string& szBitmapFileName,LPRECT lpRect=NULL){
		///	画像のファイルへの書き出し(lpRectは書き出し領域。NULLならば全域)
		return 0;
	}

	virtual LRESULT	Release(){ return 0; }
	///	読み込んだ画像の解放

	//////////////////////////////////////////////////////////////////////////
	///	サーフェース情報
	///	(これさえあれば、情報を取得して転送することが出来る)

	virtual CSurfaceInfo* GetSurfaceInfo() { return m_vSurfaceInfo.get(); }

	//////////////////////////////////////////////////////////////////////////

	virtual smart_ptr<ISurface> clone()=0;
	/**
		同じ型のオブジェクトを生成して返す。
		ただし、サーフェースの内容はコピーされるわけではない。
	*/

	ISurface();
	virtual ~ISurface() {}	//	merely place holder

protected:
	///	サーフェース情報を持った構造体
	smart_ptr<CSurfaceInfo>	m_vSurfaceInfo;
	smart_ptr<ISurfaceLocker> m_vSurfacelocker;

};	//	とりあえず、それだけあれば十分っしょ＾＾；

class ISurfaceDefault : public ISurface {
/**
	GenaralBlt,GeneralEffectだけを実装したサーフェース
	普通のサーフェースは、こいつから派生させて実装すると良い。
	使う側はISurfaceを用いる。
	こうしておけば、ISurfaceまわりをDLL側から呼び出すときに、
	CSurfaceInfoの実装が実行コードのなかに混入してこない。
*/
public:
	virtual LRESULT GeneralBlt(CSurfaceInfo::EBltType type,CSurfaceInfo*pSrc,CSurfaceInfo::CBltInfo* pInfo,DWORD*pAdditionalParameter=NULL){
		///	委譲するだけで良いと思われ
		return GetSurfaceInfo()->GeneralBlt(type,pSrc,pInfo,pAdditionalParameter);
	}
	virtual LRESULT GeneralEffect(CSurfaceInfo::EEffectType type,LPRECT prc=NULL,DWORD*pAdditionalParameter=NULL){
		return GetSurfaceInfo()->GeneralEffect(type,prc,pAdditionalParameter);
	}
};

class ISurfaceFactory {
/*
	ISurfaceのfactory
*/
public:
	virtual smart_ptr<ISurface> CreateInstance() = 0;
	virtual ~ISurfaceFactory(){}
};

/////////////////////////////////////////////////////////////////////////////

class ISurfaceNullDevice : public ISurface {
/**
	class IPlane のNullDevice
	要するに、何もしない。virtual proxy(smart_ptr<ISurface>)から使うときのため。
*/
public:
	virtual LRESULT GeneralBlt(CSurfaceInfo::EBltType type,CSurfaceInfo* pSrc,CSurfaceInfo::CBltInfo* lpDst,
		DWORD*pAdditionalParameter=NULL) { return 0; }
	virtual LRESULT GeneralEffect(CSurfaceInfo::EEffectType type,LPRECT prc=NULL,
		DWORD*pAdditionalParameter=NULL) { return 0; }
	virtual int GetType() const { return 0; }
	virtual LRESULT GetSize(int& x,int& y) const { return 0; }
	virtual LRESULT SetSize(int x,int y) { return 0; }
	virtual	LRESULT SetColorKey(ISurfaceRGB rgb) { return 0; }
	virtual ISurfaceRGB GetColorKey() const { return 0; }
	virtual LRESULT SetColorKeyPos(int x,int y){ return 0; }
	virtual smart_ptr<ISurface> clone() { return smart_ptr<ISurface>(new ISurfaceNullDevice);}
	virtual LRESULT		Clear(LPRECT lpRect=NULL){ return 0;}
	virtual LRESULT		SetFillColor(ISurfaceRGB c) { return 0; }
	virtual ISurfaceRGB	GetFillColor() const { return 0;}
};

/////////////////////////////////////////////////////////////////////////////

#endif
