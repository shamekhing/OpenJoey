// yaneSoundStream.h
//		programmed by ENRA		'02/03/25

#ifndef __yaneSoundStream_h__
#define __yaneSoundStream_h__

class ISoundStream {
/**
	サウンドデータをPCM形式で読み出すためのインターフェース
*/
public:
	ISoundStream(){};
	virtual ~ISoundStream(){};

	/// オープンする
	virtual LRESULT	Open(const char* filename) = 0;

	/// クローズする
	virtual LRESULT	Close() = 0;

	/// サウンドデータの長さをmsec単位で返す
	virtual LONG	GetLength() const = 0;

	/// 再生位置をmsec単位で設定する
	virtual LRESULT	SetCurrentPos(LONG lPosition) = 0;

	/// 再生位置をmsec単位で返す
	virtual LONG	GetCurrentPos() const = 0;

	/// lpDestに、dwSizeだけPCM形式で展開する
	/// [in] lpDest - 展開バッファへのポインタ
	/// [in] dwSize - 展開サイズ
	virtual DWORD 	Read(BYTE* lpDest, DWORD dwSize) = 0;

	/// ↑のdwSizeの最小値
	virtual DWORD	GetPacketSize() const = 0;

	// WAVEFORMATEX構造体を返す
	virtual WAVEFORMATEX* GetFormat() const = 0;
};

class CNullSoundStream : public ISoundStream {
/**
	class ISoundStream のNullDeviceクラス
*/
public:
	CNullSoundStream(){};
	virtual ~CNullSoundStream(){};

	/// override from class ISoundStream
	LRESULT	Open(const char* filename) { return -1; }
	LRESULT	Close() { return -1; }
	LONG	GetLength() const { return -1; }
	LRESULT	SetCurrentPos(LONG dwPosition) { return -1; }
	LONG	GetCurrentPos() const { return -1; }
	DWORD 	Read(BYTE* lpDest, DWORD dwSize) { return 0; }
	DWORD	GetPacketSize() const { return 0; };
	WAVEFORMATEX* GetFormat() const { return NULL; }
};


class IStringMap;
class ISoundStreamFactory {
/**
	class ISoundStream 派生クラスを生成するfactory のインターフェース
*/
public:
	ISoundStreamFactory(){};
	virtual ~ISoundStreamFactory(){};

	/// ISoundStreamを生成する
	virtual smart_ptr<ISoundStream> Open(const string& filename) = 0;

	IStringMap* GetPlugInMap() { return m_vPlugInMap.get(); }
	/**
		拡張子⇒生成するサウンドPlugInのクラス名
		へのmapです。
		GetPluginMap()->Write("ogg", "CVorbisStream");
		のようにしてプラグインを追加していきます。
	*/

protected:
	/// 拡張子からPlugInのクラス名へのmap
	smart_ptr<IStringMap>	m_vPlugInMap;
};

class IObjectCreater;
class CSoundStreamFactory : public ISoundStreamFactory {
/**
	class ISoundStream 派生クラスを生成するfactory
*/
public:
	CSoundStreamFactory();
	virtual ~CSoundStreamFactory();

	/// override from class ISoundStreamFactory
	smart_ptr<ISoundStream> Open(const string& filename);
};

#endif // __yaneSoundStream_h__
