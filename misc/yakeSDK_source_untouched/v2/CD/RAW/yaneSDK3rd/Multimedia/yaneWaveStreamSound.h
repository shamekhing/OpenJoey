// yaneWaveStreamSound.h
//		programmed by ENRA		 '02/03/25

#ifndef __yaneWaveStreamSound_h__
#define __yaneWaveStreamSound_h__

#include "yaneSound.h"
#include "../Thread/yaneThread.h"
#include "../Thread/yaneCriticalSection.h"
#include "../Thread/yaneEvent.h"

class IWaveOutput;
class ISoundBuffer;
class ISoundStream;
class CWaveStreamSound : public IWaveSound, public CThread {
/**
	Wave再生のための class ISound 派生クラス
	実際はclass CWaveSoundを通じて利用する
	class CWaveStaticSound と違い、ファイルを展開しながら再生する
*/
public:
	CWaveStreamSound(IWaveOutput* p);
	virtual ~CWaveStreamSound();

	/// override from class ISound
	LRESULT Open(const string& strFileName) { return Load(strFileName); }
	LRESULT Close()	{ return Release(); }
	LRESULT Play();
	LRESULT Replay();
	LRESULT Stop();
	LRESULT Pause();
	bool	IsPlay() const;
	LRESULT	SetLoopPlay(bool bLoop);
	bool	IsLoopPlay() const;
	LONG	GetLength() const;
	LRESULT SetCurrentPos(LONG lPos);
	LONG	GetCurrentPos() const;
	LRESULT SetVolume(LONG volume);
	LONG	GetVolume() const;
	string	GetFileName() const { return m_strFileName; }
	int	GetType() const { return 4; }

	/// override from class IWaveSound
	LRESULT Restore();

	/// ファイルを読み込む
	LRESULT Load(const string& strFileName);

	/// ファイルを閉じる
	LRESULT Release();

	/// 再生周波数の設定・取得
	LRESULT SetFrequency(DWORD freq);
	DWORD	GetFrequency() const;

	//////////////////////////////////////////////////////////////////////////

protected:
	// override from class CThread
	void	ThreadProc();

	// セカンダリバッファ
	smart_ptr<ISoundBuffer>	m_vBuffer;	
	smart_ptr<ISoundBuffer>	GetBuffer() const { return m_vBuffer; }
	void	SetBuffer(smart_ptr<ISoundBuffer> p) { m_vBuffer = p; }
	// バッファのByteサイズを得る
	enum	{ eBufferLength = 3, };	// セカンダリバッファの秒数
	DWORD	GetBufferSize() const { return eBufferLength * m_nAvgBytesPerSec; }

	// ファイルを読み込むため、ISoundStreamのインスタンスを保持する
	smart_ptr<ISoundStream>	m_vStream;
	smart_ptr<ISoundStream>	GetStream() const { return m_vStream; }
	void	SetStream(smart_ptr<ISoundStream> p) { m_vStream = p; }

	// Threadを作るのでCriticalSectionが必須
	CCriticalSection	m_vCS;
	CCriticalSection*	GetCS() { return &m_vCS; }

	// ファイル中の再生位置をByteで示す
	DWORD	m_dwPlayPosition;

	// 内部停止処理
	LRESULT InnerStop();

	// リングバッファ関連
	// バッファを初期化
	LRESULT InitBuffer();
	// バッファにサウンドデータを流し込む
	LRESULT GetNextSoundData();
	// 前回の書込カーソル
	DWORD	m_dwPrevWriteCursor;
	// 次回の書込場所
	DWORD	m_dwNextWriteCursor;
	// 前回の書き込めなかったサイズ
	DWORD	m_dwWriteDiffer;
	// 前回の再生カーソル
	DWORD	m_dwPrevPlayCursor;

	string	m_strFileName;		//	ファイル名
	int		m_bPaused;			//	サウンドpause中か？
	bool	m_bLoop;			//	ループモードで再生するのか？
	DWORD	m_nAvgBytesPerSec;	//	秒間のデータバイト数（再生位置算出用）
	DWORD	m_dwLength;			//  曲の長さ[単位: Byte]
	LONG	m_lVolume;			//　Volume

	IWaveOutput*	GetOutput() const { return m_pOutput; }
	IWaveOutput*	m_pOutput;
};

#endif // __yaneWaveStreamSound_h__
