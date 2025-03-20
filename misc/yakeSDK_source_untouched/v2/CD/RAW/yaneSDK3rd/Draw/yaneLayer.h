// yaneLayer.h

#ifndef __yaneLayer_h__
#define __yaneLayer_h__

class ISurface;

class ILayerBase {
public:
	virtual	void	OnDraw(ISurface*lpDraw)=0;
	virtual	void	OnDraw(HDC)=0;

	void	SetPos(int x,int y)  { m_nX = x; m_nY = y; }	//	ポジション指定
	void	GetPos(int& x,int& y) const { x = m_nX; y = m_nY; }	//	ポジション取得
	void	Enable(bool bEnable) { m_bEnable = bEnable; }	//	描画を有効にする（ディフォルトで有効）
	bool	IsEnable() const { return m_bEnable; }

	ILayerBase() : m_nX(0),m_nY(0),m_bEnable(true) {}
	virtual ~ILayerBase(){}

protected:
	int		m_nX,m_nY;	//	オフセット座標
	bool	m_bEnable;
};

class ILayer : public ILayerBase {
public:
	ILayer();
	virtual ~ILayer();
};

class IAfterLayer : public ILayerBase {
public:
	IAfterLayer();
	virtual ~IAfterLayer();
};

class IHDCLayer : public ILayerBase {
public:
	IHDCLayer();
	virtual ~IHDCLayer();
};

class ILayerList {
public:
	virtual void	Add(ILayerBase*)=0;
	virtual void	Del(ILayerBase*)=0;
	virtual void	Clear()=0;
	virtual bool	IsEmpty() const=0;
	
	virtual void	OnDraw(ISurface*)=0;
	virtual void	OnDraw(HDC)=0;

	virtual ~ILayerList(){}
};

class CLayerList : public ILayerList {
public:
	virtual void	Add(ILayerBase*);			///	自分自身をフックに加える
	virtual void	Del(ILayerBase*);			///	自分自身をフックから外す
	virtual void	Clear();					///	すべてをクリアする
	virtual bool	IsEmpty() const;			///	空であるか？

	///	自分の保持しているフックリストに対してOnDrawを呼び出す
	virtual void	OnDraw(ISurface*);
	virtual void	OnDraw(HDC);

protected:
	list_chain<ILayerBase>	m_listLayer;	//	フックリスト
	list_chain<ILayerBase>* GetList() { return& m_listLayer; }
};

#endif
