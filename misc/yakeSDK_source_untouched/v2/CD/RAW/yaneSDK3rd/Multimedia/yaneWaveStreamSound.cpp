#include "stdafx.h"
#include "../Auxiliary/yaneFile.h"
#include "../Thread/yaneCriticalSection.h"

#include "yaneWaveOutput.h"
#include "yaneSoundParameter.h"
#include "yaneSoundBuffer.h"
#include "yaneSoundStream.h"

#include "yaneWaveStreamSound.h"
//#include "../Window/yaneDebugWindow.h"

#define sign(val) (val>0?1:(val<0?-1:0))	//	符号を返す

//////////////////////////////////////////////////////////////////////////

void CWaveStreamSound::ThreadProc()
{
	bool bWait = false;
	while(true){
		if (!IsThreadValid()) return ;
		// バッファ上の再生位置ではなく、ファイル全体を通しての再生位置を検出する
		// GetNextSoundData関数でのThreadSleep関数の関係上、20msec程度の精度しかない
		{	CCriticalLock lock(&m_vCS);
			DWORD dwPlayCursor, dwWriteCursor;
			GetBuffer()->GetCurrentPosition(dwPlayCursor, dwWriteCursor);
			if (dwPlayCursor >= m_dwPrevPlayCursor){
				m_dwPlayPosition += dwPlayCursor - m_dwPrevPlayCursor;
			} else {
				m_dwPlayPosition += (dwPlayCursor + GetBufferSize()) - m_dwPrevPlayCursor;
			}
			m_dwPrevPlayCursor = dwPlayCursor;
			m_dwPlayPosition %= m_dwLength;
		}

		// データを流し込む
		DWORD ret = GetNextSoundData();
		if (ret==1) {	// EOF
			if (IsLoopPlay()){
				// ループ再生なら、ストリームを巻き戻す
				GetStream()->SetCurrentPos(0);
				continue;
			} else {
				// 非ループ再生なら、終端までの再生の面倒を見る
				bWait = true;
				break;
			}
		} else if (ret>=2) {	// エラー
			// Stop()を呼ぶとデッドロックになっちまう
			InnerStop();
			return ;
		}
	}

	// 再生カーソルが停止位置より前方にあると、
	// dwPlayCursor >= m_dwNextWriteCursor という条件では
	// 正しい位置で停止出来ない
	// （バッファの周期分だけ早く停止してしまう）
	bool bForward = false;
	DWORD dwLimit = 0;
	{	CCriticalLock lock(&m_vCS);
		dwLimit = m_dwNextWriteCursor - (m_nAvgBytesPerSec / 50);	// 20msec手前で止める
		DWORD dwPlayCursor, dwWriteCursor;
		GetBuffer()->GetCurrentPosition(dwPlayCursor, dwWriteCursor);
		if (dwPlayCursor >= dwLimit){
			bForward = true;	// 再生カーソルは停止位置より前方
		}
	}
	while(bWait){
		// バッファ上の再生位置ではなく、ファイル全体を通しての再生位置を検出する
		// ThreadSleep関数の関係上、20msec程度の精度しかない
		{	smart_ptr<CCriticalLock> lock(new CCriticalLock(&m_vCS));
			DWORD dwPlayCursor, dwWriteCursor;
			GetBuffer()->GetCurrentPosition(dwPlayCursor, dwWriteCursor);
			if (!bForward && dwPlayCursor >= dwLimit){
				// キターー(´▽｀)ーー
				// Stop()を呼ぶとデッドロックになっちまう
				InnerStop();
				return ;
			}
			if (dwPlayCursor == m_dwPrevPlayCursor){
				lock.Delete();		// ロック解除
				ThreadSleep(20);	// しばし休息
				continue;
			} else if (dwPlayCursor > m_dwPrevPlayCursor){
				m_dwPlayPosition += dwPlayCursor - m_dwPrevPlayCursor;
			} else {
				m_dwPlayPosition += (dwPlayCursor + GetBufferSize()) - m_dwPrevPlayCursor;
				// これで再生カーソルは停止位置より後方に位置する事になる
				if (bForward) bForward = false;
			}
			m_dwPrevPlayCursor = dwPlayCursor;
			m_dwPlayPosition %= m_dwLength;
		}
	}
}

LRESULT CWaveStreamSound::GetNextSoundData()
{
	// 今から弄るんだから邪魔すんなよー
	smart_ptr<CCriticalLock> lock(new CCriticalLock(&m_vCS));

	DWORD dwPlayCursor, dwWriteCursor;
	GetBuffer()->GetCurrentPosition(dwPlayCursor, dwWriteCursor);

	// 消費したサイズを得る
	DWORD dwUsedSize = 0;
	if (dwWriteCursor == m_dwPrevWriteCursor){	// 消費してないの？
		lock.Delete();		// ロック解除
		ThreadSleep(20);	// しばし休息
		return 0;
	} else {
		dwUsedSize = (GetBufferSize() + dwWriteCursor - m_dwPrevWriteCursor) % GetBufferSize();
	}
	dwUsedSize += m_dwWriteDiffer;	// 前回補充出来なかった分を上乗せ
	if (dwUsedSize < GetStream()->GetPacketSize()){
		// パケットサイズより小さいなら何もしない
		// ACMは出力パケットサイズ以下のデコードを認めてくれない
		// バッファリングしてもええけど、めんどくさいねん
		return 0;
	}
	m_dwWriteDiffer = 0;

	// 再生ポジションより先は弄っちゃだめ
	if(m_dwNextWriteCursor < dwPlayCursor&&dwPlayCursor < m_dwNextWriteCursor + dwUsedSize){
		dwUsedSize = dwPlayCursor - m_dwNextWriteCursor;
	}
	// 現在の書込カーソルを覚えておく
	m_dwPrevWriteCursor = dwWriteCursor;

	if (m_dwNextWriteCursor+dwUsedSize > GetBufferSize()) {
		// 再生カーソルを超えないようにする
		DWORD dwNewUsedSize = GetBufferSize() - m_dwNextWriteCursor;
		DWORD dwDiffer = dwUsedSize - dwNewUsedSize;
		if (dwDiffer > dwPlayCursor) {
			dwUsedSize = dwNewUsedSize + dwPlayCursor;
		}
	}

	BYTE* lpLockedBuffer;
	DWORD dwLockedSize;
	BYTE* lpLockedBuffer2;
	DWORD dwLockedSize2;
	smart_ptr<BYTE> pBuffer;	// ラップアラウンドした時のためにリニアで用意する
	pBuffer.AddArray(dwUsedSize);
	if (GetBuffer()->Lock(m_dwNextWriteCursor, dwUsedSize, &lpLockedBuffer, &dwLockedSize, &lpLockedBuffer2, &dwLockedSize2)){
		Err.Out("CWaveStaticSound::GetNextSoundData GetBuffer()->Lock()に失敗！");
		return 2;
	}
	DWORD readed = GetStream()->Read(pBuffer.get(), dwUsedSize);
	if (readed > dwUsedSize){	// エラーと考えられる
		Err.Out("CWaveStaticSound::GetNextSoundData GetStream()->Read()に失敗！");
		return 3;
	}
	// リニアバッファから分割コピーする
	if (readed < dwLockedSize){
		// 少し読めなかったらしい
		memcpy(lpLockedBuffer, pBuffer.get(), readed);
	} else {
		// ラップアラウンドしたらしい
		memcpy(lpLockedBuffer, pBuffer.get(), dwLockedSize);
	}
	if (lpLockedBuffer2!=NULL && readed > dwLockedSize){
		// ラップアラウンドした分をコピー
		if (readed - dwLockedSize < dwLockedSize2){
			// 少し読めなかったらしい
			memcpy(lpLockedBuffer2, pBuffer.get()+dwLockedSize, readed - dwLockedSize);
		} else {
			// 残りをきっちりコピー
			memcpy(lpLockedBuffer2, pBuffer.get()+dwLockedSize, dwLockedSize2);
		}
	}
	if (GetBuffer()->Unlock(lpLockedBuffer, dwLockedSize, lpLockedBuffer2, dwLockedSize2)){
		Err.Out("CWaveStaticSound::GetNextSoundData GetBuffer()->Unlock()に失敗！");
		return 5;
	}
	// カーソル更新してー
	m_dwNextWriteCursor += readed;
	m_dwNextWriteCursor %= GetBufferSize();
	// 読み込みできなかったサイズは次回にまわす
	if(readed < dwUsedSize) {
		m_dwWriteDiffer += dwUsedSize - readed;
	}

	if (GetStream()->GetCurrentPos()>=GetLength()){
		return 1;	// EOFでっせ
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////

CWaveStreamSound::CWaveStreamSound(IWaveOutput* p) : m_pOutput(p)
{
	// デフォルトでループしない
	SetLoopPlay(false);

	// NullSoundBufferをつっこんどく
	SetBuffer(smart_ptr<ISoundBuffer>(new CNullSoundBuffer));
	// なんも読み込んでないから０
	m_dwLength = 0;
	m_nAvgBytesPerSec = 0;
	// 参照カウント方式
	m_bPaused = 0;
	// Volumeを初期化
	m_lVolume = 0;

	// 面倒見てください
	GetOutput()->AddSound(this);
}

CWaveStreamSound::~CWaveStreamSound(){
	Release();
	// さよならー
	GetOutput()->DelSound(this);
}

LRESULT CWaveStreamSound::InitBuffer()
{
	// 今から弄るんだから邪魔すんなよー
	CCriticalLock lock(&m_vCS);

	// ファイル読みこんでるの？
	if (m_strFileName==""||m_nAvgBytesPerSec==0){
		Err.Out("CWaveStreamSound::InitBuffer ファイルが読み込めてない");
		return 1;
	}

	// 読み込めてるのかチェック
	if (GetStream()->GetFormat()==NULL){
		Err.Out("CWaveStreamSound::InitBuffer GetStream()->GetFormat()==NULL");
		return 2;
	}

	// DirectSoundバッファの作成
	SetBuffer(GetOutput()->CreateBuffer());
	if (GetBuffer()->Create(GetStream()->GetFormat(), GetBufferSize(), true, GetOutput()->GetSoundParameter().get()) != 0){
		Err.Out("CWaveStreamSound::InitBuffer サウンドバッファの作成に失敗！");
		return 2;
	}

	// カーソルを全て初期化
	m_dwPrevWriteCursor = 0;
	m_dwNextWriteCursor = 0;
	m_dwWriteDiffer = 0;
	m_dwPrevPlayCursor = 0;
	GetBuffer()->SetCurrentPosition(0);
	// バッファにVolumeを反映させておく
	GetBuffer()->SetVolume(m_lVolume);

	// バッファサイズの7/8くらいを読み込む
	DWORD dwUsedSize = (GetBufferSize()*7)/8;
	// DirectSoundバッファをLock
	BYTE* lpLockedBuffer;
	DWORD dwLockedSize;
	if (GetBuffer()->Lock(m_dwNextWriteCursor, dwUsedSize, &lpLockedBuffer, &dwLockedSize)){
		// これでダメなら、メモリ足りんのちゃう？
		Err.Out("CWaveStaticSound::InitBuffer GetBuffer()->Lock()に失敗！");
		return 5;
	}
	DWORD readed = GetStream()->Read(lpLockedBuffer, dwLockedSize);
	if (readed > dwLockedSize){	// エラーと考えられる
		Err.Out("CWaveStaticSound::InitBuffer GetStream()->Read()に失敗！");
		return 6;
	}
	if (GetBuffer()->Unlock(lpLockedBuffer, dwLockedSize)){
		// こんなんふつー、失敗するかぁ...どないせーちゅーんじゃ
		Err.Out("CWaveStaticSound::InitBuffer GetBuffer()->Unlock()に失敗！");
		return 7;
	}
	// カーソル更新してー
	m_dwNextWriteCursor += readed;
	// 読み込みできなかったサイズは次回にまわす
	if(readed < dwUsedSize) { m_dwWriteDiffer = dwUsedSize - readed; }

	return 0;
}

LRESULT CWaveStreamSound::Load(const string& filename)
{
	DWORD dwDataLen = 0;
	LPVOID pWaveData = NULL;
	WAVEFORMATEX* pWFormat=NULL;

	Release(); // まずは、解放しようね！
	m_strFileName.erase();
	if (filename.empty()){ return 0; }

	// ファイルを開く
	SetStream(GetOutput()->GetSoundParameter()->GetStreamFactory()->Open(filename));

	//	秒間の再生秒数
	LPWAVEFORMATEX lpwfe = GetStream()->GetFormat();
	if(lpwfe && (lpwfe->nAvgBytesPerSec)){
		m_nAvgBytesPerSec = lpwfe->nAvgBytesPerSec;
	} else {
		//	おかしいなぁ。これエラーになるんかい？
		m_nAvgBytesPerSec = 0;
	}

	m_strFileName = filename;
	m_dwLength = (LONGLONG)GetStream()->GetLength() * m_nAvgBytesPerSec / 1000;

	// 頭出し
	SetCurrentPos(0);

	return 0;
}

LRESULT CWaveStreamSound::Release(){
	if (IsPlay()) Stop();
	// 再生中ならとめんといかんよ！ （一応ね）

	// 今から弄るんだから邪魔すんなよー
	CCriticalLock lock(&m_vCS);

	// NullSoundBufferをつっこんどく
	SetBuffer(smart_ptr<ISoundBuffer>(new CNullSoundBuffer));
	m_strFileName.erase();

	return 0;
}

LRESULT CWaveStreamSound::Restore(){
	// 今から弄るんだから邪魔すんなよー
	DWORD status;
	{	CCriticalLock lock(&m_vCS);
		status = GetBuffer()->GetStatus();
	}

	if (m_strFileName!="" && status==1) {
		// うわーロストしてるよ(;´Д`)
		bool bPlay = IsPlay();
		Stop();
		string temp = m_strFileName; // コピーして渡さんとReleaseされちゃう:p
		Load(temp);
		return (bPlay) ? Play() : 0;
	}

	return 0;
}

LRESULT CWaveStreamSound::Play()
{
	// 再生中ならとめんといかんよ！
	if (IsPlay()) Stop();

	m_bPaused = 0;

	// バッファを初期化する
	if (InitBuffer()!=0){
		Stop();
		return 1;
	}

	// バッファを再生状態にする
	if (GetBuffer()->Play(true)!=0){
		Err.Out("CWaveStaticSound::InitBuffer GetBuffer()->Play()に失敗！");
		Stop();
		return 2;
	}

	// あとはワーカスレッドに任せる
	if (CreateThread()!=0){
		Err.Out("CWaveStaticSound::InitBuffer CreateThread()に失敗！");
		Stop();
		return 3;
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////

LRESULT CWaveStreamSound::Stop()
{
	// 今から弄るんだから邪魔すんなよー
	{
		CCriticalLock lock(&m_vCS);
		if (m_bPaused!=0) return 0; // ポーズ中だなぁ
	}

	// スレッド止める->安全のため止まるまで待つStopThread関数を使う
	StopThread();

	return InnerStop();
}

LRESULT CWaveStreamSound::InnerStop()
{
	// スレッド内から呼ばれる事もあるのでLockが必要
	CCriticalLock lock(&m_vCS);

	// バッファ止めて初期化する
	LRESULT ret = GetBuffer()->Stop();
	GetBuffer()->SetCurrentPosition(0);
	GetStream()->SetCurrentPos(0);
	m_dwPlayPosition = 0;
	m_bPaused = 0;

	return ret;
}

LONG CWaveStreamSound::GetCurrentPos() const
{
	// 今から弄るんだから邪魔すんなよー
	CCriticalLock lock(&const_cast<CWaveStreamSound*>(this)->m_vCS);

	if (m_nAvgBytesPerSec==0) return -1;	//	Loadの情報取得で失敗している

	LONG pos = LONG(((DWORDLONG) m_dwPlayPosition * 1000 / m_nAvgBytesPerSec) );
	//	DWORDLONGにキャストしないと溢れちゃう:p
	return pos;
}

LRESULT CWaveStreamSound::SetCurrentPos(LONG lPos){
	if (m_nAvgBytesPerSec==0) return 1;	//	Loadの情報取得で失敗している

	// きっちり位置を変えるには止めるしかない
	bool bPlay = IsPlay();
	Stop();

	// Stop関数により、スレッドは止まっている
	// よってCriticalSectionは必要ない
	GetStream()->SetCurrentPos(lPos);
	m_dwPlayPosition = LONG(((DWORDLONG) lPos * m_nAvgBytesPerSec / 1000 ) );

	return (bPlay) ? Play() : 0;
}

bool CWaveStreamSound::IsPlay() const
// そのチャンネルが再生中(=3)ならtrue
{
	// 今から弄るんだから邪魔すんなよー
	CCriticalLock lock(&const_cast<CWaveStreamSound*>(this)->m_vCS);

	DWORD ret = GetBuffer()->GetStatus();
	return ret==3;
}

LRESULT CWaveStreamSound::Pause()
 // 特定チャンネルのpause
{
	// 今から弄るんだから邪魔すんなよー
	CCriticalLock lock(&m_vCS);

	m_bPaused += sign(m_bPaused);	//	必殺技:p
	if (!IsPlay()) {
		return 0; // もう終わってる
	}
	//	再生中のをpauseしたんならば
	m_bPaused = 1;

	LRESULT ret = GetBuffer()->Stop();
	return ret;
}

LRESULT CWaveStreamSound::Replay()
// pauseで止めていた、そのチャンネルの再生を再開。
{
	// 今から弄るんだから邪魔すんなよー
	CCriticalLock lock(&m_vCS);

	if (!m_bPaused) return 0; // pauseしてへんて！

	if (--m_bPaused==0) {	//	参照カウント方式
		LRESULT ret = GetBuffer()->Play(true);
		return ret;
	}
	return 0;
}

LRESULT CWaveStreamSound::SetVolume(LONG volume)
{
	// 今から弄るんだから邪魔すんなよー
	CCriticalLock lock(&m_vCS);

	// 範囲外のエラーチェックせーへんから、勝手にやってやー
	m_lVolume = volume;
	LRESULT ret = GetBuffer()->SetVolume(m_lVolume);
	return ret;

	// DSBVOLUME_MAX（減衰なし） :		0
	// DSBVOLUME_MIN（無音）	 : -10000
	// の間で指定するのよん。
}

LONG CWaveStreamSound::GetVolume() const
// 取得は、特定チャンネルに対してしかできない...
{
	// 今から弄るんだから邪魔すんなよー
	CCriticalLock lock(&const_cast<CWaveStreamSound*>(this)->m_vCS);

	return m_lVolume;
}

//	bmdさんの提案
DWORD CWaveStreamSound::GetFrequency() const
{
	// 今から弄るんだから邪魔すんなよー
	CCriticalLock lock(&const_cast<CWaveStreamSound*>(this)->m_vCS);

	DWORD dwFreq;
	LRESULT ret = GetBuffer()->GetFrequency(dwFreq);

	if( ret != 0 ) return -1;
	return( (LONG)dwFreq );
}

LRESULT CWaveStreamSound::SetFrequency(DWORD freq)
{
	// 今から弄るんだから邪魔すんなよー
	CCriticalLock lock(&m_vCS);

	if((DSBFREQUENCY_MIN<=freq && freq<=DSBFREQUENCY_MAX) || freq==DSBFREQUENCY_ORIGINAL){
		LRESULT ret = GetBuffer()->SetFrequency(freq);
		if(ret==0) return 0;
	}
	return 1;
}

LRESULT	CWaveStreamSound::SetLoopPlay(bool bLoop)
{
	// 今から弄るんだから邪魔すんなよー
	CCriticalLock lock(&m_vCS);

	m_bLoop = bLoop;
	return 0;
}
bool CWaveStreamSound::IsLoopPlay() const
{
	// 今から弄るんだから邪魔すんなよー
	CCriticalLock lock(&const_cast<CWaveStreamSound*>(this)->m_vCS);

	bool ret = m_bLoop;
	return ret;
}

LONG CWaveStreamSound::GetLength() const
{
	// 今から弄るんだから邪魔すんなよー
	CCriticalLock lock(&const_cast<CWaveStreamSound*>(this)->m_vCS);

	LONG len =  GetStream()->GetLength();
	return len;
}
