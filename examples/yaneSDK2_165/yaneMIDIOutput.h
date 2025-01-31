//
// yaneMIDIOutput.h
//
//		generic MIDI wrapper	programmed by yaneurao(M.Isozaki) '00/09/09
//

#ifndef __yaneMIDIOutput_h__
#define __yaneMIDIOutput_h__

#include "yaneSoundBase.h"	//	MIDI出力基底クラス

class CMIDIOutput : public CSoundBase {
public:
	CMIDIOutput(void);
	virtual ~CMIDIOutput();

	virtual LRESULT Open(LPCTSTR pFileName) { return m_lpSound->Open(pFileName); }
	virtual LRESULT Close(void) { return m_lpSound->Close();  }
	virtual LRESULT Play(void)	{ return m_lpSound->Play();   }
	virtual LRESULT Replay(void){ return m_lpSound->Replay(); }
	virtual LRESULT Stop(void)	{ return m_lpSound->Stop();   }
	virtual LRESULT Pause(void) { return m_lpSound->Pause();  }
	virtual bool IsPlay(void)	{ return m_lpSound->IsPlay(); }
	virtual bool IsLoopPlay(void){ return m_lpSound->IsLoopPlay(); }// 追加
	virtual LRESULT SetLoopMode(bool bLoop) { return m_lpSound->SetLoopMode(bLoop); }
	virtual LONG	GetCurrentPos(void) { return m_lpSound->GetCurrentPos(); }
	virtual LONG	GetLength(void) { return m_lpSound->GetLength();}
	virtual LRESULT SetCurrentPos(LONG lPos){ return m_lpSound->SetCurrentPos(lPos);}
	virtual LRESULT SetVolume(LONG volume){ return m_lpSound->SetVolume(volume);}
	virtual LONG	GetVolume(void){ return m_lpSound->GetVolume();}

	//	0:NULL device , 1:CMIDIOutputMM(廃止('01/02/04)) , 2:CMIDIOutputMCI
	//	3:CMIDIOutputDM
	static	void	SelectDevice(int nDevice);
	static	int		GetDevice(void) { return m_nMIDIType;}

protected:
	auto_ptrEx<CSoundBase>	m_lpSound;

private:
	static bool m_bFirst;		//	最初の一回か？
	static bool m_bMIDINoDevice;	//	MIDIデバイスの有無(m_bFirst==falseになってから有効)
	static int m_nMIDIType;		//	0 : NULL device
								//	1:CMIDIOutputMM(廃止('01/02/04))
								//	2:CMIDIOutputMCI
								//	3:CMIDIOutputDM
	static void	CreateInstance(CMIDIOutput* t);
	static void	MIDITest(void);
	static set<CMIDIOutput*>	m_lpaMIDIList;	//	全インスタンスへのチェイン
};

#endif
