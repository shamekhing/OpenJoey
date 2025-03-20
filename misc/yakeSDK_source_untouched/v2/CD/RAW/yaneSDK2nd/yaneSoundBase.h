//
// yaneSoundBase.h
//
//		MIDI,WAVE出力基底クラス
//

#ifndef __yaneSoundBase_h__
#define __yaneSoundBase_h__

/////////////////////////////////////////////////////////////////////////////
class CSoundBase {
public:
	CSoundBase(void) {}
	virtual ~CSoundBase() {}

	virtual LRESULT Open(LPCTSTR pFileName) = 0;
	virtual LRESULT Close(void) = 0;
	virtual LRESULT Play(void)	= 0;
	virtual LRESULT Replay(void) = 0;
	virtual LRESULT Stop(void)	= 0;
	virtual LRESULT Pause(void) = 0;
	virtual bool IsPlay(void) = 0;
	virtual bool IsLoopPlay(void) = 0;// 追加
	virtual LRESULT SetLoopMode(bool bLoop) = 0;
	virtual LONG	GetCurrentPos(void) = 0;
	virtual LRESULT	SetCurrentPos(LONG lPos) = 0;	// [ms]
	virtual LONG	GetLength(void) = 0; // 曲長を得る [ms]
	virtual LRESULT SetVolume(LONG volume)=0; // このチャンネルに対するヴォリューム
	virtual LONG	GetVolume(void)=0;	// 取得は、特定チャンネルに対してしかできない...

	virtual LRESULT ConvertToMealTime(LONG nPos,int&nHour,int&nMin,int&nSec,int&nMS);
	virtual LRESULT ConvertFromMealTime(LONG&nPos,int nHour,int nMin,int nSec,int nMS);
};

//	俗に言うNULL DEVICE。
//	MIDIの付いていない環境では、これでをアップキャストして使えば良い
class CSoundNullDevice : public CSoundBase {
public:
	virtual LRESULT Open(LPCTSTR pFileName) { return 0; }
	virtual LRESULT Close(void)	 { return 0; }
	virtual LRESULT Play(void)	 { return 0; }
	virtual LRESULT Replay(void) { return 0; }
	virtual LRESULT Stop(void)	 { return 0; }
	virtual LRESULT Pause(void)	 { return 0; }
	virtual bool IsPlay(void)	 { return false; }
	virtual bool IsLoopPlay(void){ return m_bLoop; }
	virtual LRESULT SetLoopMode(bool bLoop) { m_bLoop=bLoop; return 0; }
	virtual LONG	GetCurrentPos(void) { return -1; }
	virtual LRESULT	SetCurrentPos(LONG lPos) { return -1;}
	virtual LONG	GetLength(void){ return -1;}
	virtual LRESULT SetVolume(LONG volume){ return -1;}
	virtual LONG	GetVolume(void){ return DSBVOLUME_MIN;}

	CSoundNullDevice(void) { m_bLoop = false; }

protected:
	bool m_bLoop;
};

#endif
